/*
 * Copyright 2022 Emiliano Gonzalez (egonzalez . hiperion @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/esp32-berry-lang *
 *
 * This is based on other projects:
 *    Berry (https://github.com/berry-lang/berry)
 *    LittleFS port for ESP-IDF (https://github.com/joltwallet/esp_littlefs)
 *    Lightweight TFTP server library (https://github.com/lexus2k/libtftp)
 *    esp32 run berry language (https://github.com/HoGC/esp32_berry)
 *    Tasmota (https://github.com/arendst/Tasmota)
 *    Others (see individual files)
 *
 *    please contact their authors for more information.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>

#include "tftp_server.h"
#include "internals.h"

/**
 * This method is called, when new read request is received.
 * Override this method and add implementation for your system.
 * @param file name of the file requested
 * @return return 0 if file can be read, otherwise return -1
 */
int TFTP_on_read(char *file);

/**
 * This method is called, when new write request is received.
 * Override this method and add implementation for your system.
 * @param file name of the file requested
 * @return return 0 if file can be written, otherwise return -1
 */
int TFTP_on_write(char *file);

/**
 * This method is called, when new data are required to be read from file for sending.
 * Override this method and add implementation for your system.
 * @param buffer buffer to fill with data from file
 * @param len maximum length of buffer
 * @return return number of bytes read to buffer
 * @important if length of read data is less than len argument value, it is considered
 *            as end of file
 */
int TFTP_on_read_data(uint8_t *buffer, int len);

/**
 * This method is called, when new data arrived for writing to file.
 * Override this method and add implementation for your system.
 * @param buffer buffer with received data
 * @param len length of received data
 * @return return number of bytes written
 */
int TFTP_on_write_data(uint8_t *buffer, int len);

/**
 * This method is called, when transfer operation is complete.
 * Override this method and add implementation for your system.
 */
void TFTP_on_close(void);

/////////////////////////////////////

void TFTP_send_ack(uint16_t block_num);
void TFTP_send_error(uint16_t code, const char *message);
 int TFTP_wait_for_ack(uint16_t block_num);
 int TFTP_process_write(void);
 int TFTP_process_read(void);
 int TFTP_parse_wrq(void);
 int TFTP_parse_rrq(void);
 int TFTP_parse_rq(void);

/////////////////////////////////////

FILE *f;
TaskHandle_t thTFTP;
static const char* ip_to_str(struct sockaddr *addr) {
    static char buf[INET_ADDRSTRLEN];
    return inet_ntop(AF_INET, addr, buf, INET_ADDRSTRLEN);
}

static char TAG[] = "TFTP";

enum e_tftp_cmd {
    TFTP_CMD_RRQ   = 1,
    TFTP_CMD_WRQ   = 2,
    TFTP_CMD_DATA  = 3,
    TFTP_CMD_ACK   = 4,
    TFTP_CMD_ERROR = 5,
};

enum e_error_code {
    ERR_NOT_DEFINED         = 0,
    ERR_FILE_NOT_FOUND      = 1,
    ERR_ACCESS_VIOLATION    = 2,
    ERR_NO_SPACE            = 3,
    ERR_ILLEGAL_OPERATION   = 4,
    ERR_UNKNOWN_TRANSFER_ID = 5,
    ERR_FILE_EXISTS         = 6,
    ERR_NO_SUCH_USER        = 7
};

uint16_t m_port;
int m_sock = -1;
struct sockaddr m_client;
uint8_t *m_buffer = NULL;
int m_data_size;

// Max supported TFTP packet size
const int TFTP_DATA_SIZE = 512 + 4;

void TFTP_init(uint16_t port) {
    if (port == 0)
        m_port = TFTP_DEFAULT_PORT;
    else
        m_port = port;
}

int TFTP_process_write(void) {
    int result = -1;
    int total_size = 0;
    int next_block_num = 1;
    while (1) {
        socklen_t addr_len = sizeof(m_client);
        m_data_size = recvfrom(m_sock, m_buffer, TFTP_DATA_SIZE, 0, &m_client, &addr_len);

        if (m_data_size < 0) {
            ESP_LOGE(TAG, "error on receive");
            break;
        }
        uint16_t code = ntohs(*(uint16_t*) (&m_buffer[0]));
        if (code != TFTP_CMD_DATA) {
            ESP_LOGE(TAG, "not data packet received: [%d]", code);
            ESP_LOG_BUFFER_HEX_LEVEL(TAG, m_buffer, m_data_size, ESP_LOG_DEBUG);
            if ((code == TFTP_CMD_WRQ) && (next_block_num == 1)) {
                // some clients repeat request several times
                TFTP_send_ack(0);
                continue;
            }
            result = 1; // repeat
            break;
        }
        uint16_t block_num = ntohs(*(uint16_t*) (&m_buffer[2]));
        int data_size = m_data_size - 4;
        TFTP_send_ack(block_num);
        if (block_num < next_block_num) {
            // Maybe this is dup, ignore it
            ESP_LOGI(TAG, "dup packet received: [%d], expected [%d]", block_num, next_block_num);
        } else {
            next_block_num++;
            total_size += data_size;
            TFTP_on_write_data(&m_buffer[4], data_size);
            ESP_LOGD(TAG, "Block size: %d", data_size);
        }
        if (m_data_size < TFTP_DATA_SIZE) {
            ESP_LOGI(TAG, "file received: (%d bytes)", total_size);
            result = 0;
            break;
        }
    }
    return result;
}

int TFTP_process_read(void) {
    int result = -1;
    int block_num = 1;
    int total_size = 0;

    for (;;) {
        *(uint16_t*) (&m_buffer[0]) = htons(TFTP_CMD_DATA);
        *(uint16_t*) (&m_buffer[2]) = htons(block_num);

        m_data_size = TFTP_on_read_data(&m_buffer[4], TFTP_DATA_SIZE - 4);
        if (m_data_size < 0) {
            ESP_LOGE(TAG, "Failed to read data from file");
            TFTP_send_error(ERR_ILLEGAL_OPERATION, "failed to read file");
            result = -1;
            break;
        }
        ESP_LOGD(TAG, "Sending data to %s, blockNumber=%d, size=%d", ip_to_str(&m_client), block_num, m_data_size);

        for (int r = 0; r < 3; r++) {
            result = sendto(m_sock, m_buffer, m_data_size + 4, 0, &m_client, sizeof(m_client));
            if (result < 0) {
                break;
            }
            if (TFTP_wait_for_ack(block_num) == 0) {
                result = 0;
                break;
            }
            ESP_LOGE(TAG, "No ack/wrong ack, retrying");
            result = -1;
        }
        if (result < 0) {
            break;
        }
        total_size += m_data_size - 4;

        if (m_data_size < TFTP_DATA_SIZE - 4) {
            ESP_LOGI(TAG, "Sent file (%d bytes)", total_size);
            result = 0;
            break;
        }
        block_num++;
    }
    return result;
}

void TFTP_send_ack(uint16_t block_num) {
    uint8_t data[4];

    *(uint16_t*) (&data[0]) = htons(TFTP_CMD_ACK);
    *(uint16_t*) (&data[2]) = htons(block_num);
    ESP_LOGD(TAG, "ack to %s, blockNumber=%d", ip_to_str(&m_client), block_num);
    sendto(m_sock, (uint8_t*) &data[0], sizeof(data), 0, &m_client, sizeof(struct sockaddr));
}

void TFTP_send_error(uint16_t code, const char *message) {
    *(uint16_t*) (&m_buffer[0]) = htons(TFTP_CMD_ERROR);
    *(uint16_t*) (&m_buffer[2]) = htons(code);
    strcpy((char*) (&m_buffer[4]), message);
    sendto(m_sock, m_buffer, 4 + strlen(message) + 1, 0, &m_client, sizeof(m_client));
}

int TFTP_wait_for_ack(uint16_t block_num) {
    uint8_t data[4];

    ESP_LOGD(TAG, "waiting for ack");
    socklen_t len = sizeof(m_client);
    int sizeRead = recvfrom(m_sock, (uint8_t*) &data, sizeof(data), 0, &m_client, &len);

    if ((sizeRead != sizeof(data)) || (ntohs(*(uint16_t*) (&data[0])) != TFTP_CMD_ACK)) {
        ESP_LOGE(TAG, "received wrong ack packet: %d", ntohs(*(uint16_t*) (&data[0])));
        TFTP_send_error(ERR_NOT_DEFINED, "incorrect ack");
        return -1;
    }

    if (ntohs(*(uint16_t*) (&data[2])) != block_num) {
        ESP_LOGE(TAG, "received ack not in order");
        return 1;
    }
    return 0;
}

int TFTP_parse_wrq(void) {
    uint8_t *ptr = m_buffer + 2;
    char *filename = (char*) ptr;
    ptr += strlen(filename) + 1;
    char *mode = (char*) ptr;
    ptr += strlen(mode) + 1;
    if (TFTP_on_write(filename) < 0) {
        ESP_LOGE(TAG, "failed to open file %s for writing", filename);
        TFTP_send_error(ERR_ACCESS_VIOLATION, "cannot open file");
        return -1;
    }
    if ((ptr - m_buffer < m_data_size) && !strcmp((char*) ptr, "blksize")) {
        uint8_t data[] = { 0, 6, 'b', 'l', 'k', 's', 'i', 'z', 'e', 0, '5', '1', '2', 0 };
        sendto(m_sock, data, sizeof(data), 0, &m_client, sizeof(m_client));
        ESP_LOGI(TAG, "Extended block size is requested. rejecting");
    } else {
        TFTP_send_ack(0);
    }
    ESP_LOGI(TAG, "receiving file: %s", filename);
    return 0;
}

int TFTP_parse_rrq(void) {
    uint8_t *ptr = m_buffer + 2;
    char *filename = (char*) ptr;
    ptr += strlen(filename) + 1;
    char *mode = (char*) ptr;
    ptr += strlen(mode) + 1;
    if (TFTP_on_read(filename) < 0) {
        ESP_LOGE(TAG, "failed to open file %s for reading", filename);
        TFTP_send_error(ERR_FILE_NOT_FOUND, "cannot open file");
        return -1;
    }
    ESP_LOGI(TAG, "sending file: %s", filename);
    return 0;
}

int TFTP_parse_rq(void) {
    int result = -1;
    for (;;) {
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, m_buffer, m_data_size, ESP_LOG_DEBUG);
        uint16_t cmd = ntohs(*(uint16_t*) (&m_buffer[0])); /* parse command */
        switch (cmd) {
            case TFTP_CMD_WRQ:
                result = TFTP_parse_wrq();
                if (result == 0) {
                    result = TFTP_process_write();
                    TFTP_on_close();
                } else {
                    result = 0; // it is ok since parsing is not network issue
                }
                break;
            case TFTP_CMD_RRQ:
                result = TFTP_parse_rrq();
                if (result == 0) {
                    result = TFTP_process_read();
                    TFTP_on_close();
                } else {
                    result = 0; // it is ok since parsing is not network issue
                }
                break;
            default:
                ESP_LOGW(TAG, "unknown command %d", cmd);
        }
        if (result <= 0) {
            break;
        }
    }
    return result;
}

int TFTP_start(void) {
    if (m_sock >= 0) {
        return 0;
    }
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0) {
        return -1;
    }
    m_buffer = (uint8_t*) malloc(TFTP_DATA_SIZE);
    if (m_buffer == NULL) {
        close (m_sock);
        m_sock = -1;
        return -1;
    }

    int optval = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(int));

    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short) m_port);
    if (bind(m_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "binding error");
        return 0;
    }

    ESP_LOGI(TAG, "Started on port %d", m_port);
    return 0;
}

// Call with true for blocking mode
int TFTP_run(bool wait_for) {
    socklen_t len = sizeof(struct sockaddr);
    m_data_size = recvfrom(m_sock, m_buffer, TFTP_DATA_SIZE, wait_for ? 0 : MSG_DONTWAIT, &m_client, &len);
    if (m_data_size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            return 0;
        }
        return -1;
    }

    return TFTP_parse_rq();
}

void TFTP_stop(void) {
    if (m_sock >= 0) {
        ESP_LOGI(TAG, "Stopped");
        close (m_sock);
        m_sock = -1;
        free (m_buffer);
    }
    m_buffer = NULL;
}

int TFTP_on_read(char *file) {
    f = F_OPEN(file, "r");
    if (f == NULL)
        return -1;
    else
        return 0;
}

int TFTP_on_write(char *file) {
    f = F_OPEN(file, "w");
    if (f == NULL)
        return -1;
    else
        return 0;
}

int TFTP_on_read_data(uint8_t *buffer, int len) {
    int cnt = 0, ch;
    while ((ch = getc(f)) != EOF && cnt < len) {
        buffer[cnt] = ch;
        ++cnt;
    }

    return cnt;
}

int TFTP_on_write_data(uint8_t *buffer, int len) {
    int cnt = 0;

    while (fputc(buffer[cnt], f) != EOF && cnt < len)
        ++cnt;

    return cnt;
}

void TFTP_on_close(void) {
    if (f != NULL)
        fclose(f);
    return;
}

void TFTP_task(void *pvParameter) {
    TFTP_init(0);
    TFTP_start();
    while (TFTP_run(false) >= 0)
    {
        vTaskDelay(2);
    }
    TFTP_stop();
}

void TFTP_task_start(void) {
    xTaskCreatePinnedToCore(
            TFTP_task,
            "TFTP task",
            5000,
            NULL,
            10,
            &thTFTP,
            0
            );
}

void TFTP_task_stop(void) {
    TFTP_stop();
    vTaskDelete(thTFTP);
}
