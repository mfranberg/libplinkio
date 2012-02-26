#ifndef _SNPARRAY_H_
#define _SNPARRAY_H_

#include <Python.h>

#include <plinkio/plinkio.h>

/**
 * Python object that allows access to an underlying
 * C snp row. The main reason for using this instead
 * of a list is that there is a significant overhead
 * in converting a C array to a Python list.
 */
typedef struct
{
    PyObject_HEAD

    /**
     * The underlying C snp array.
     */
    snp_t *array;

    /**
     * Length of the row.
     */
    size_t length;
} snp_array_t;

/**
 * Creates a Python snp array from a C snp_t array.
 *
 * @param type The snparray type object.
 * @param array An array containing snps.
 * @param length The length of the array.
 *
 * @return A python snp array.
 */
snp_array_t *snparray_from_array(PyTypeObject *prototype, snp_t *array, size_t length);

/**
 * Destructor.
 *
 * Makes sure that the memory allocated on the C-side is
 * freed.
 *
 * @param self The snp array to deallocate. 
 */
void snparray_dealloc(snp_array_t *self);

/**
 * Returns a string representation of the snp array. All
 * alleles are converted to their corresponding number, if
 * any value is higher than 3 they are represented as 'E'.
 *
 * @param self Pointer to snp_array_t.
 *
 * @return Python string representation.
 */
PyObject *snparray_str(PyObject *self);

/**
 * Returns a list of counts for each allele. The list
 * is always of size 4. In index 0, 1, 2 are the counts
 * for the different alleles, and 3 is for missing values.
 *
 * @param self Pointer to snp_array_t.
 * @param none Ignored.
 *
 * @return list of counts for each allele.
 */
PyObject *snparray_allele_counts(PyObject *self, PyObject *none);

/**
 * Returns the length of the snp array.
 *
 * @param self Pointer to snp_array_t.
 *
 * @return the length of the snp array.
 */
Py_ssize_t snparray_length(PyObject *self);

/**
 * Returns the allele at the given position of the snp array.
 *
 * @param self Pointer to snp_array_t.
 * @param index Index in snp array to retreive.
 * 
 * @throws IndexError if index is outside of the array.
 */
PyObject *snparray_getitem(PyObject *self, Py_ssize_t index);

/**
 * Returns true if the array contains an allele with the
 * given value.
 *
 * @param self Pointer to snp_array_t.
 * @param value The value of an allele to look for.
 *
 * @return True if the array contains the given allele,
 *         false otherwise.
 */
int snparray_contains(PyObject *self, PyObject *value);

/**
 * Instance methods.
 */
static PyMethodDef snparray_methods[] =
{
    { "allele_counts", snparray_allele_counts, METH_NOARGS, "Returns a list of counts for each allele. The list is always of size 4, so it includes missing values coded as 3." },
    { NULL }  /* Sentinel */
};

/**
 * Methods relating to the sequence api.
 */
static PySequenceMethods sequence_methods =
{
    snparray_length, /* sq_length */
    0, /* sq_concat */
    0, /* sq_repeat */
    snparray_getitem, /* sq_item */
    0, /* sq_slice */
    0, /* sq_ass_item */
    0, /* sq_ass_slice */
    snparray_contains, /* sq_contains */
};

#if PY_MAJOR_VERSION >= 3

/**
 * Python type prototype definition.
 */
static PyTypeObject py_snp_array_prototype =
{
    PyVarObject_HEAD_INIT( NULL, 0 )
    "plinkio.SnpArray",       /* tp_name */
    sizeof( snp_array_t ),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor) snparray_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    &sequence_methods,          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    snparray_str,               /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "View for C Array containing SNPs",  /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    snparray_methods,          /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};

#else

/**
 * Python type prototype definition.
 */
static PyTypeObject py_snp_array_prototype =
{
    PyObject_HEAD_INIT( NULL )
    0,
    "plinkio.SnpArray",       /* tp_name */
    sizeof( snp_array_t ),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor) snparray_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    &sequence_methods,          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    snparray_str,               /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "View for C Array containing SNPs",  /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    snparray_methods,          /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};

#endif

#endif /* End of _SNPARRAY_H_ */
