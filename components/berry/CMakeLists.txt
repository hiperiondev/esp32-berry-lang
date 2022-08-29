file(
    GLOB 
        berry_srcs
        port/*.c
        berry/src/*.c
        berry_mapping/src/*.c
        berry_ctypes/src/*.c
)

idf_component_register(
    SRCS ${berry_srcs}
    INCLUDE_DIRS
        port
        generate
        berry/src
        berry_mapping/src
        berry_ctypes/src
    REQUIRES
        internals
)
component_compile_options("-Wno-return-type")
