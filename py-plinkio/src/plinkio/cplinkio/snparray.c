#include <Python.h>

#include "snparray.h"
#include "common.h"

snp_array_t *
snparray_from_array(PyTypeObject *prototype, snp_t *array, size_t length)
{
    snp_array_t *snp_array = (snp_array_t *) prototype->tp_alloc( prototype, 0 );
    if( snp_array != NULL )
    {
        snp_array->array = (snp_t *) malloc( sizeof( snp_t ) * length );
        memcpy( snp_array->array, array, sizeof( snp_t ) * length );
        snp_array->length = length;
    }

    return snp_array;
}

void
snparray_dealloc(snp_array_t *self)
{
    if( self != NULL )
    {
        free( self->array );
        self->length = 0;
        Py_TYPE( self )->tp_free( ( PyObject * ) self );
    }
}

PyObject *
snparray_str(PyObject *self)
{   
    snp_array_t *snp_array = (snp_array_t *) self;
    size_t i;
    size_t string_length = 3 * snp_array->length + 3;
    char *as_string = (char *) malloc( string_length );
    char *string_p = as_string;
    PyObject *py_string;

    *string_p++ = '[';

    for(i = 0; i < snp_array->length; i++)
    {
        if( snp_array->array[ i ] <= 3 )
        {
            *string_p++ = '0' + snp_array->array[ i ];
        }
        else
        {
            *string_p++ = 'E';
        }

        *string_p++ = ',';
        *string_p++ = ' ';
    }

    // We should remove the last ", ".
    string_p -= 2;
    *string_p++ = ']';
    *string_p++ = '\0';

    py_string = PyUnicode_FromString( as_string );
    free( as_string );

    return py_string;
}

PyObject *
snparray_allele_counts(PyObject *self, PyObject *none)
{
    snp_array_t *snp_array = (snp_array_t *) self;
    long counts[4] = { 0 };
    size_t i;
    PyObject *count_list;

    for(i = 0; i < snp_array->length; i++)
    {
        if( snp_array->array[ i ] <= 3 )
        {
            counts[ snp_array->array[ i ] ]++;
        }
    }

    count_list = PyList_New( 4 );
    if( count_list == NULL )
    {
        PyErr_SetString( PyExc_MemoryError, "Could not allocate count list." );
        return NULL;
    }

    for(i = 0; i < 4; i++)
    {
        PyObject *count = PyLong_FromLong( counts[ i ] );
        PyList_SET_ITEM( count_list, i, count );
    }

    return count_list;
}

Py_ssize_t
snparray_length(PyObject *self)
{
    snp_array_t *snp_array = (snp_array_t *) self;
    return snp_array->length;
}

PyObject *
snparray_getitem(PyObject *self, Py_ssize_t index)
{
    snp_array_t *snp_array = (snp_array_t *) self;
    
    if( index >= (Py_ssize_t) snp_array->length )
    {
        PyErr_SetString( PyExc_IndexError, "snparray index out of range" );
        return NULL;
    }

    return PyLong_FromLong( (long) snp_array->array[ index ] );
}

int
snparray_contains(PyObject *self, PyObject *value)
{
    snp_array_t *snp_array = (snp_array_t *) self;
    size_t i;

    long value_as_long = PyLong_AsLong( value );
    snp_t value_to_look_for;
    if( value_as_long == -1 )
    {
        return 0;
    }

    value_to_look_for = (snp_t) value_as_long;
    for(i = 0; i < snp_array->length; i++)
    {
        if( snp_array->array[ i ] == value_to_look_for )
        {
            return 1;
        }
    }

    return 0;
}
