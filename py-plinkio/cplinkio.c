#include <Python.h>

#include <plinkio/plinkio.h>

/**
 * Wrapper object for a plink file. In python it will
 * act as a handle to the file.
 */
typedef struct
{
    PyObject_HEAD

    /**
     * The plink file.
     */
    struct pio_file_t file;

    /**
     * Buffer for reading a row.
     */
    snp_t *row;

    /**
     * Length of the row.
     */
    size_t row_length;
} cPlinkFile;

#if PY_MAJOR_VERSION >= 3

/**
 * Python type of the above.
 */
static PyTypeObject cPlinkFileType = {
    PyVarObject_HEAD_INIT( NULL, 0 )
    "plinkio.cPlinkFile",       /* tp_name */
    sizeof( cPlinkFile ),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    0,                          /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Contains the pio_file_t struct for interfacing libplinkio.",           /* tp_doc */
};

#else

/**
 * Python type of the above.
 */
static PyTypeObject cPlinkFileType = {
    PyObject_HEAD_INIT( NULL )
    0,
    "plinkio.cPlinkFile",       /* tp_name */
    sizeof( cPlinkFile ),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    0,                          /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Contains the pio_file_t struct for interfacing libplinkio.",           /* tp_doc */
};

#endif

/**
 * Opens a plink file and returns a handle to it.
 *
 * @param self -
 * @param args First argument is a path to the plink file.
 *
 * @return A handle to the plink file, or throws an IOError.
 */
static PyObject *
plinkio_open(PyObject *self, PyObject *args)
{
    const char *path;
    struct pio_file_t plink_file = { 0 };
    cPlinkFile *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "s", &path ) )
    {
        return NULL;
    }
    
    if( pio_open( &plink_file, path ) != PIO_OK )
    {
        PyErr_SetString( PyExc_IOError, "Error while trying to open plink file." );
        return NULL;
    }

    c_plink_file = (cPlinkFile *) cPlinkFileType.tp_alloc( &cPlinkFileType, 0 );
    c_plink_file->file = plink_file;
    c_plink_file->row  = (snp_t *) malloc( pio_row_size( &plink_file ) );
    c_plink_file->row_length = pio_num_samples( &plink_file );
    if( !pio_one_locus_per_row( &plink_file ) )
    {
        c_plink_file->row_length = pio_num_loci( &plink_file );
    }


    return (PyObject *) c_plink_file;
}

/**
 * Reads a row of SNPs from the bed, advances the file pointer,
 * returns the snps as a list, where the SNPs are encoded as in
 * pio_next_row.
 *
 * @param self -
 * @param args - First argument is a handle to an opened file.
 *
 * @return List of snps, or None if we are at the end. Throws IOError
 *         if an error occurred.
 */
static PyObject *
plinkio_next_row(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    cPlinkFile *c_plink_file;
    int i;
    
    if( !PyArg_ParseTuple( args, "O!", &cPlinkFileType, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (cPlinkFile *) plink_file;
    snp_t *row = c_plink_file->row;
    int status = pio_next_row( &c_plink_file->file, row );
    if( status == PIO_END )
    {
        Py_RETURN_NONE;
    }
    else if( status == PIO_ERROR )
    {
        PyErr_SetString( PyExc_IOError, "Error while reading from plink file." );
        return NULL;
    }

    PyObject *row_list = PyList_New( c_plink_file->row_length );
    for(i = 0; i < c_plink_file->row_length; i++)
    {
#if PY_MAJOR_VERSION >= 3
        PyObject *snp = PyLong_FromLong( (long) row[ i ] );
#else
        PyObject *snp = PyInt_FromLong( (long) row[ i ] );
#endif
        PyList_SetItem( row_list, i, snp );
    }

    return row_list;
}

/**
 * Moves the file pointer to the first row, so that the next
 * call to pio_next_row returns the first row.
 *
 * @param self -
 * @param args - First argument is a handle to an opened file.
 */
static PyObject *
plinkio_reset_row(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    cPlinkFile *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &cPlinkFileType, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (cPlinkFile *) plink_file;
    
    pio_reset_row( &c_plink_file->file );
    
    Py_RETURN_NONE;
}

/**
 * Returns a list of loci and their locations whithin the
 * genome that are contained in the file.
 *
 * @param self -
 * @param args - First argument is a handle to an opened file.
 *
 * @return List of loci.
 */
static PyObject *
plinkio_get_loci(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    cPlinkFile *c_plink_file;
    int i;
    
    if( !PyArg_ParseTuple( args, "O!", &cPlinkFileType, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (cPlinkFile *) plink_file;

    PyObject *module = PyImport_ImportModule( "plinkio.plinkfile" );
    if( module == NULL )
    {
        return NULL;
    }

    PyObject *locusClass = PyObject_GetAttrString( module, "Locus" );
    if( locusClass == NULL )
    {
        return NULL;
    }

    PyObject *loci_list = PyList_New( pio_num_loci( &c_plink_file->file ) );
    for(i = 0; i < pio_num_loci( &c_plink_file->file ); i++)
    {
        struct pio_locus_t *locus = pio_get_locus( &c_plink_file->file, i );

        PyObject *args = Py_BuildValue( "Bskkcc",
                                        locus->chromosome,
                                        locus->name,
                                        locus->position,
                                        locus->bp_position,
                                        locus->major,
                                        locus->minor );
        PyObject *pyLocus = PyObject_CallObject( locusClass, args );

        PyList_SetItem( loci_list, i, pyLocus );
    }

    return loci_list;
}

/**
 * Returns a list of samples and associated information
 * that are contained in the file.
 *
 * @param self -
 * @param args - First argument is a handle to an opened file.
 *
 * @return List of samples.
 */
static PyObject *
plinkio_get_samples(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    cPlinkFile *c_plink_file;
    int i;
    
    if( !PyArg_ParseTuple( args, "O!", &cPlinkFileType, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (cPlinkFile *) plink_file;

    PyObject *module = PyImport_ImportModule( "plinkio.plinkfile" );
    if( module == NULL )
    {
        return NULL;
    }

    PyObject *sampleClass = PyObject_GetAttrString( module, "Sample" );
    if( sampleClass == NULL )
    {
        return NULL;
    }

    PyObject *sample_list = PyList_New( pio_num_samples( &c_plink_file->file ) );
    for(i = 0; i < pio_num_samples( &c_plink_file->file ); i++)
    {
        struct pio_sample_t *sample = pio_get_sample( &c_plink_file->file, i );

        PyObject *args = Py_BuildValue( "IsIIiif",
                                        sample->fid,
                                        sample->iid,
                                        sample->father_iid,
                                        sample->mother_iid,
                                        (int) sample->sex,
                                        (int) sample->affection,
                                        sample->phenotype );
        PyObject *pySample = PyObject_CallObject( sampleClass, args );

        PyList_SetItem( sample_list, i, pySample );
    }

    return sample_list;
}

/**
 * Determines whether samples are stored row-wise or column-wise.
 *
 * @param self -
 * @param args - First argument is a handle to an opened file.
 *
 * @return True if one row contains the genotypes for a single locus.
 */
static PyObject *
plinkio_one_locus_per_row(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    cPlinkFile *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &cPlinkFileType, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (cPlinkFile *) plink_file;
 
    return PyBool_FromLong( (long) pio_one_locus_per_row( &c_plink_file->file ) );
}

/**
 * Closes the given plink file.
 *
 * Note: Releases some memory allocated in pio_open.
 *
 * @param self -
 * @param args - First argument is a handle to an opened file.
 */
static PyObject *
plinkio_close(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    cPlinkFile *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &cPlinkFileType, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (cPlinkFile *) plink_file;

    pio_close( &c_plink_file->file );
    free( c_plink_file->row );
    bzero( &c_plink_file->file, sizeof( c_plink_file->file ) );
    
    Py_RETURN_NONE;    
}

static PyMethodDef plinkio_methods[] = {
    { "open", plinkio_open, METH_VARARGS, "Opens a plink file." },
    { "next_row", plinkio_next_row, METH_VARARGS, "Reads the next row of a plink file." },
    { "reset_row", plinkio_reset_row, METH_VARARGS, "Resets reading of the plink file to the first row." },
    { "get_loci", plinkio_get_loci, METH_VARARGS, "Returns the list of loci." },
    { "get_samples", plinkio_get_samples, METH_VARARGS, "Returns the list of samples." },
    { "one_locus_per_row", plinkio_one_locus_per_row, METH_VARARGS, "Returns true if a row contains the snps for a single locus." },
    { "close", plinkio_close, METH_VARARGS, "Close a plink file." },
    { NULL }
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

#if PY_MAJOR_VERSION  >= 3

static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "cplinkio",
    "Wrapper module for the libplinkio c functions.",
    -1,
    plinkio_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit_cplinkio(void)
{
    PyObject *module;

    cPlinkFileType.tp_new = PyType_GenericNew;
    if( PyType_Ready( &cPlinkFileType ) < 0 )
    {
        return NULL;
    }

    module = PyModule_Create( &moduledef );
    if( module == NULL )
    {
        return NULL;
    }

    Py_INCREF( &cPlinkFileType );
    PyModule_AddObject( module, "cPlinkFile", (PyObject *) &cPlinkFileType );

    return module;
}

#else

PyMODINIT_FUNC
initcplinkio(void)
{
    PyObject *m;

    cPlinkFileType.tp_new = PyType_GenericNew;
    if( PyType_Ready( &cPlinkFileType ) < 0 )
    {
        return;
    }

    m = Py_InitModule3( "cplinkio", plinkio_methods, "Wrapper module for the libplinkio c functions." );

    Py_INCREF( &cPlinkFileType );
    PyModule_AddObject( m, "cPlinkFile", (PyObject *) &cPlinkFileType );
}

#endif
