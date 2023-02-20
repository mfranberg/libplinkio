#ifndef _COMMON_H_
#define _COMMON_H_

#include <Python.h>

/**
 * Common integer conversion for python 3 and 2.x.
 */
#if PY_MAJOR_VERSION < 3
    #define PyLong_FromLong(x) ( (PyObject *) PyInt_FromLong( (long) ( x ) ) )
    #define PyLong_AsLong(x) ( (long) PyInt_AsLong( ( x ) ) )
#endif

#endif
