#include <Python.h>

#include <plinkio/plinkio.h>

#include "snparray.h"

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
} c_plink_file_t;

/**
 * Deallocates a Python CPlinkFile object.
 * 
 * @param self Pointer to a c_plink_file_t.
 */
void
cplinkfile_dealloc(c_plink_file_t *self)
{
    if( self->row != NULL )
    {
        pio_close( &self->file );
        free( self->row );
        self->row_length = 0;
        Py_TYPE( self )->tp_free( ( PyObject * ) self );
    }
}

#if PY_MAJOR_VERSION >= 3

/**
 * Python type of the above.
 */
static PyTypeObject c_plink_file_prototype =
{
    PyVarObject_HEAD_INIT( NULL, 0 )
    "plinkio.CPlinkFile",       /* tp_name */
    sizeof( c_plink_file_t ),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor) cplinkfile_dealloc, /* tp_dealloc */
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
static PyTypeObject c_plink_file_prototype =
{
    PyObject_HEAD_INIT( NULL )
    0,
    "plinkio.CPlinkFile",       /* tp_name */
    sizeof( c_plink_file_t ),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor) cplinkfile_dealloc, /* tp_dealloc */
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
    struct pio_file_t plink_file;
    c_plink_file_t *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "s", &path ) )
    {
        return NULL;
    }
    int pio_open_status = pio_open( &plink_file, path );
    if( pio_open_status != PIO_OK )
    {
        if( pio_open_status == P_FAM_IO_ERROR )
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to open the FAM plink file." );
        }
        else if( pio_open_status == P_BIM_IO_ERROR )
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to open the BIM plink file." );
        }
        else if( pio_open_status == P_BED_IO_ERROR )
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to open the BED plink file." );
        }
        else
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to open plink file." );
        } 
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) c_plink_file_prototype.tp_alloc( &c_plink_file_prototype, 0 );
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
 * Creates a plink file and returns a handle to it.
 *
 * @param self -
 * @param args First argument is a path to the plink file. Second argument
 *             is a list of Sample objects.
 *
 * @return A handle to the plink file, or throws an IOError.
 */
static PyObject *
plinkio_create(PyObject *self, PyObject *args)
{
    int i, sex, affection;
    const char *path;
    struct pio_sample_t *samples;
    struct pio_file_t plink_file;
    c_plink_file_t *c_plink_file;
    PyObject *sample_list;
    PyObject *sample_object;
    PyObject *i_object;
    PyObject *fid_object;
    PyObject *iid_object;
    PyObject *father_iid_object;
    PyObject *mother_iid_object;
    PyObject *phenotype_object;
    PyObject *sex_object;
    PyObject *affection_object;

    if( !PyArg_ParseTuple( args, "sO", &path, &sample_list ) )
    {
        return NULL;
    }
    
    samples = ( struct pio_sample_t * ) malloc( sizeof( struct pio_sample_t ) * PyObject_Size( sample_list ) );
    for(i = 0; i < PyObject_Size( sample_list ); i++)
    {
        i_object = PyInt_FromLong( i );
        sample_object = PyObject_GetItem( sample_list, i_object );
        fid_object = PyObject_GetAttrString( sample_object, "fid" );
        iid_object = PyObject_GetAttrString( sample_object, "iid" );
        father_iid_object = PyObject_GetAttrString( sample_object, "father_iid" );
        mother_iid_object = PyObject_GetAttrString( sample_object, "mother_iid" );
        phenotype_object = PyObject_GetAttrString( sample_object, "phenotype" );

        samples[ i ].fid = strndup( PyString_AsString( fid_object ), PyString_Size( fid_object ) );
        samples[ i ].iid = strndup( PyString_AsString( iid_object ), PyString_Size( iid_object ) );
        samples[ i ].father_iid = strndup( PyString_AsString( father_iid_object ), PyString_Size( father_iid_object ) );
        samples[ i ].mother_iid = strndup( PyString_AsString( mother_iid_object ), PyString_Size( mother_iid_object ) );
        samples[ i ].phenotype = PyFloat_AsDouble( phenotype_object );
        
        sex_object = PyObject_GetAttrString( sample_object, "sex" );
        sex = PyInt_AsLong( sex_object );
        if( sex == 0 )
        {
            samples[ i ].sex = PIO_FEMALE;
        }
        else if( sex == 1 )
        {
            samples[ i ].sex = PIO_MALE;
        }
        else
        {
            samples[ i ].sex = PIO_UNKNOWN;
        }

        affection_object = PyObject_GetAttrString( sample_object, "affection" );
        affection = PyInt_AsLong( affection_object );
        if( affection == 0 )
        {
            samples[ i ].affection = PIO_CONTROL;
        }
        else if( affection == 1 )
        {
            samples[ i ].affection = PIO_CASE;
        }
        else if( affection == -9 )
        {
            samples[ i ].affection = PIO_MISSING;
        }
        else
        {
            samples[ i ].affection = PIO_CONTINUOUS;
        }
        
        /* Lower refcount for both accessed attributes and objects */
        Py_DECREF( i_object );
        Py_DECREF( iid_object );
        Py_DECREF( fid_object );
        Py_DECREF( father_iid_object );
        Py_DECREF( mother_iid_object );
        Py_DECREF( sex_object );
        Py_DECREF( affection_object );
        Py_DECREF( sample_object );
    }

    int pio_create_status = pio_create( &plink_file, path, samples, PyObject_Size( sample_list ) );
    
    /* Free duplicated strings and sample array */
    for(i = 0; i < PyObject_Size( sample_list ); i++)
    {
        free( samples[ i ].iid );
        free( samples[ i ].fid );
        free( samples[ i ].father_iid );
        free( samples[ i ].mother_iid );
    }
    free( samples );
    
    /* Check for errors */
    if( pio_create_status != PIO_OK )
    {
        free( samples );
        if( pio_create_status == P_FAM_IO_ERROR )
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to creating FAM file." );
        }
        else if( pio_create_status == P_BIM_IO_ERROR )
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to creating BIM file." );
        }
        else if( pio_create_status == P_BED_IO_ERROR )
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to creating BED file." );
        }
        else
        {
            PyErr_SetString( PyExc_IOError, "Error while trying to creating plink file." );
        }
        return NULL;
    }
    
    c_plink_file = (c_plink_file_t *) c_plink_file_prototype.tp_alloc( &c_plink_file_prototype, 0 );
    c_plink_file->file = plink_file;
    c_plink_file->row = (snp_t *) malloc( pio_row_size( &plink_file ) );
    c_plink_file->row_length = pio_num_samples( &plink_file );
    
    return (PyObject *) c_plink_file;
}

/**
 * Writes a row to a created plink file.
 *
 * @param self -
 * @param args First argument is the plink file. Second argument
 *             is a Locus object. Third argument is a list of genotypes.
 *
 * @return A handle to the plink file, or throws an IOError.
 */
static PyObject *
plinkio_write_row(PyObject *self, PyObject *args)
{
    PyObject *plink_file;
    c_plink_file_t *c_plink_file;
    PyObject *locus_object;
    PyObject *genotypes;
    PyObject *i_object;
    PyObject *chromosome_object;
    PyObject *name_object;
    PyObject *position_object;
    PyObject *bp_position_object;
    PyObject *allele1_object;
    PyObject *allele2_object;
    PyObject *genotype_object;
    struct pio_locus_t locus;
    int i;
    int write_status;
    
    if( !PyArg_ParseTuple( args, "O!OO", &c_plink_file_prototype, &plink_file, &locus_object, &genotypes ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;
    if( PyObject_Size( genotypes ) != c_plink_file->row_length )
    {
        PyErr_SetString( PyExc_ValueError, "Error, wrong number of genotypes given." );
        return NULL;
    }

    chromosome_object = PyObject_GetAttrString( locus_object, "chromosome" );
    name_object = PyObject_GetAttrString( locus_object, "name" ); 
    position_object = PyObject_GetAttrString( locus_object, "position" );
    bp_position_object = PyObject_GetAttrString( locus_object, "bp_position" );
    allele1_object = PyObject_GetAttrString( locus_object, "allele1" );
    allele2_object = PyObject_GetAttrString( locus_object, "allele2" );

    locus.chromosome = PyInt_AsLong( chromosome_object );
    locus.name = PyString_AsString( name_object );
    locus.position = PyFloat_AsDouble( position_object );
    locus.bp_position = PyInt_AsLong( bp_position_object );
    locus.allele1 = PyString_AsString( allele1_object );
    locus.allele2 = PyString_AsString( allele2_object );
    
    for(i = 0; i < c_plink_file->row_length; i++)
    {
        i_object = PyInt_FromLong( i );
        genotype_object = PyObject_GetItem( genotypes, i_object );
        c_plink_file->row[ i ] = (snp_t) PyInt_AsLong( genotype_object );

        Py_DECREF( genotype_object );
        Py_DECREF( i_object );
    }

    write_status = pio_write_row( &c_plink_file->file, &locus, c_plink_file->row );
    
    Py_DECREF( chromosome_object );
    Py_DECREF( name_object );
    Py_DECREF( position_object );
    Py_DECREF( bp_position_object );
    Py_DECREF( allele1_object );
    Py_DECREF( allele2_object );

    if( write_status != PIO_OK )
    {
        PyErr_SetString( PyExc_IOError, "Error while writing to plink file." );
        return NULL;
    }
    
    Py_RETURN_NONE;
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
    c_plink_file_t *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &c_plink_file_prototype, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;
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

    return (PyObject *) snparray_from_array( &py_snp_array_prototype, row, c_plink_file->row_length );
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
    c_plink_file_t *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &c_plink_file_prototype, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;
    
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
    c_plink_file_t *c_plink_file;
    int i;
    
    if( !PyArg_ParseTuple( args, "O!", &c_plink_file_prototype, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;

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

        PyObject *args = Py_BuildValue( "BsfLss",
                                        locus->chromosome,
                                        locus->name,
                                        locus->position,
                                        locus->bp_position,
                                        locus->allele1,
                                        locus->allele2 );
        PyObject *pyLocus = PyObject_CallObject( locusClass, args );

        /* Steals the pyLocus reference */
        PyList_SetItem( loci_list, i, pyLocus );
        
        Py_DECREF( args );
    }

    Py_DECREF( module );
    Py_DECREF( locusClass );

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
    c_plink_file_t *c_plink_file;
    int i, sex, affection;
    
    if( !PyArg_ParseTuple( args, "O!", &c_plink_file_prototype, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;

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

        sex = 0;
        if( sample->sex == PIO_MALE )
        {
            sex = 1;
        }
        else if( sample->sex != PIO_FEMALE )
        {
            sex = -9;
        }

        affection = 0;
        if( sample->affection == PIO_CASE )
        {
            affection = 1;
        }
        else if( sample->affection != PIO_CONTROL )
        {
            affection = -9;
        }

        PyObject *args = Py_BuildValue( "ssssiif",
                                        sample->fid,
                                        sample->iid,
                                        sample->father_iid,
                                        sample->mother_iid,
                                        sex,
                                        affection,
                                        sample->phenotype );
        PyObject *pySample = PyObject_CallObject( sampleClass, args );

        /* Steals the pySample reference */
        PyList_SetItem( sample_list, i, pySample );

        Py_DECREF( args );
    }
    
    Py_DECREF( module );
    Py_DECREF( sampleClass );

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
    c_plink_file_t *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &c_plink_file_prototype, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;
 
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
    c_plink_file_t *c_plink_file;
    
    if( !PyArg_ParseTuple( args, "O!", &c_plink_file_prototype, &plink_file ) )
    {
        return NULL;
    }

    c_plink_file = (c_plink_file_t *) plink_file;

    pio_close( &c_plink_file->file );
    
    Py_RETURN_NONE;    
}

/**
 * Transposes the given plink file.
 *
 * @param self -
 * @param args First argument is a path to the plink file to transpose, and
 *             the second argument is the path to the transposed plink file.
 *
 * @return True if the file could be transposed, false otherwise.
 */
static PyObject *
plinkio_transpose(PyObject *self, PyObject *args)
{
    const char *old_path;
    const char *new_path;
    
    if( !PyArg_ParseTuple( args, "ss", &old_path, &new_path ) )
    {
        return NULL;
    }
     
    return PyBool_FromLong( (long) ( pio_transpose( old_path, new_path ) == PIO_OK ) );
}

static PyMethodDef plinkio_methods[] =
{
    { "open", plinkio_open, METH_VARARGS, "Opens a plink file." },
    { "next_row", plinkio_next_row, METH_VARARGS, "Reads the next row of a plink file." },
    { "reset_row", plinkio_reset_row, METH_VARARGS, "Resets reading of the plink file to the first row." },
    { "get_loci", plinkio_get_loci, METH_VARARGS, "Returns the list of loci." },
    { "get_samples", plinkio_get_samples, METH_VARARGS, "Returns the list of samples." },
    { "one_locus_per_row", plinkio_one_locus_per_row, METH_VARARGS, "Returns true if a row contains the snps for a single locus." },
    { "close", plinkio_close, METH_VARARGS, "Close a plink file." },
    { "transpose", plinkio_transpose, METH_VARARGS, "Transposes the plink file." },
    { "create", plinkio_create, METH_VARARGS, "Creates a new plink file." },
    { "write_row", plinkio_write_row, METH_VARARGS, "Writes genotypes to a created plink file." },
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

    c_plink_file_prototype.tp_new = PyType_GenericNew;
    if( PyType_Ready( &c_plink_file_prototype ) < 0 )
    {
        return NULL;
    }

    py_snp_array_prototype.tp_new = PyType_GenericNew;
    if( PyType_Ready( &py_snp_array_prototype ) < 0 )
    {
        return;
    }

    module = PyModule_Create( &moduledef );
    if( module == NULL )
    {
        return NULL;
    }

    Py_INCREF( &c_plink_file_prototype );
    PyModule_AddObject( module, "CPlinkFile", (PyObject *) &c_plink_file_prototype );

    Py_INCREF( &py_snp_array_prototype );
    PyModule_AddObject( module, "SnpArray", (PyObject *) &py_snp_array_prototype );

    return module;
}

#else

PyMODINIT_FUNC
initcplinkio(void)
{
    PyObject *m;

    c_plink_file_prototype.tp_new = PyType_GenericNew;
    if( PyType_Ready( &c_plink_file_prototype ) < 0 )
    {
        return;
    }

    py_snp_array_prototype.tp_new = PyType_GenericNew;
    if( PyType_Ready( &py_snp_array_prototype ) < 0 )
    {
        return;
    }

    m = Py_InitModule3( "cplinkio", plinkio_methods, "Wrapper module for the libplinkio c functions." );

    Py_INCREF( &c_plink_file_prototype );
    PyModule_AddObject( m, "CPlinkFile", (PyObject *) &c_plink_file_prototype );

    Py_INCREF( &py_snp_array_prototype );
    PyModule_AddObject( m, "SnpArray", (PyObject *) &py_snp_array_prototype );
}

#endif
