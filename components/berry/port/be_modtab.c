/********************************************************************
 ** Copyright (c) 2018-2020 Guan Wenliang
 ** This file is part of the Berry default interpreter.
 ** skiars@qq.com, https://github.com/Skiars/berry
 ** See Copyright Notice in the LICENSE file or at
 ** https://github.com/Skiars/berry/blob/master/LICENSE
 ********************************************************************/
#include "berry.h"

/* this file contains the declaration of the module table. */

/* default modules declare */
be_extern_native_module(string);
be_extern_native_module(json);
be_extern_native_module(math);
be_extern_native_module(time);
be_extern_native_module(os);
be_extern_native_module(global);
be_extern_native_module(sys);
be_extern_native_module(debug);
be_extern_native_module(gc);
be_extern_native_module(solidify);
be_extern_native_module(introspect);
be_extern_native_module(strict);
be_extern_native_module(undefined);

/* user-defined modules declare start */
/* Tasmota Berry extensions */
#include "be_mapping.h"
be_extern_native_module(cb);
//be_extern_native_class(int64);

/* user-defined modules declare end */

/* module list declaration */
BERRY_LOCAL const bntvmodule *const be_module_table[] = {
/* default modules register */
#if BE_USE_STRING_MODULE
        &be_native_module(string),
#endif
#if BE_USE_JSON_MODULE
        &be_native_module(json),
#endif
#if BE_USE_MATH_MODULE
        &be_native_module(math),
#endif
#if BE_USE_TIME_MODULE
        &be_native_module(time),
#endif
#if BE_USE_OS_MODULE
        &be_native_module(os),
#endif
#if BE_USE_GLOBAL_MODULE
        &be_native_module(global),
#endif
#if BE_USE_SYS_MODULE
        &be_native_module(sys),
#endif
#if BE_USE_DEBUG_MODULE
        &be_native_module(debug),
#endif
#if BE_USE_GC_MODULE
        &be_native_module(gc),
#endif
#if BE_USE_SOLIDIFY_MODULE
        &be_native_module(solidify),
#endif
#if BE_USE_INTROSPECT_MODULE
        &be_native_module(introspect),
#endif
#if BE_USE_STRICT_MODULE
        &be_native_module(strict),
#endif
        &be_native_module(undefined),
        /* user-defined modules register start */
        /* Tasmota Berry extensions */
        &be_native_module(cb),

        /* user-defined modules register end */
        NULL /* do not remove */
};

/* user-defined classes declare start */
/* be_extern_native_class(my_class); */
/* user-defined classes declare end */

BERRY_LOCAL bclass_array be_class_table = {
/* first list are direct classes */
/* &be_native_class(my_class), */
        //&be_native_class(int64),
        NULL, /* do not remove */
};

/* this code loads the native class definitions */
BERRY_API void be_load_custom_libs(bvm *vm)
{
    (void)vm;   /* prevent a compiler warning */

    /* add here custom libs */
#if !BE_USE_PRECOMPILED_OBJECT
    /* be_load_xxxlib(vm); */
#endif
}
