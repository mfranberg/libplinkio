# libplinkio

This is a small C and Python library for reading Plink genotype files. 

Currently it can:
* Read and parse BED, BIM and FAM files.
* Transpose BED files.
* Write BED, BIM and FAM files.

Libplinkio will reach 1.0 when it can:
* Read PED files (i.e. non-binary bed-files).

Project rationales:
* Use C to make it as simple as possible to add bindings
  for other languages.
* Focus only on IO-functionality.
* Few external dependencies to make it easy to use.

## Installing

Installing this library is easy, just **configure** and **make**. This will also install Python bindings for the active interpeter.

*NEWS* The python extension can now be installed 
[from pypi](https://pypi.org/project/plinkio/) by:

    pip install plinkio

To compile and install from source code:

    python setup.py build
    python setup.py install

You require [tox](https://pypi.org/project/tox/) to test plinkio properly. 
By calling:

    tox

in your project directory, you will download dependencies, compile the library 
and run tests. If you get stuck with compiled libraries, adding `-r` option
will reset the test environment (this means wiping out the testing environment 
and re-initialize it from scratch by downloading dependencies and compiling again)

### Installing to a standard location

[CMake](https://cmake.org/) is required in order to compile the C libraries (mind
to the final `.` after *CMake*, which stands for your current *project* local 
directory):

    cmake .
    make
    make test
    make install

### Installing to a custom location

    cmake -DCMAKE_INSTALL_PREFIX:PATH=/path/to/plinkio .
    make
    make test
    make install

### Linking to your program

To link your own application to libplinkio you can use the following include and library paths after installing it:

    gcc source.c -lplinkio 

If you installed libplinkio to a custom location you need to specify the location of libplinkio:

    gcc -lplinkio -I/path/to/plinkio/include -L/path/to/plinkio/lib source.c

## Genotype coding

The genotypes are coded 0, 1, 2, and 3. The numbers 0-2 represent the number of A2 alleles as specified in the .bim file. The number 3 represents a missing genotype.

## Using in C

For specific information look at https://mfranberg.github.io/libplinkio/index.html

The following C program prints the genotypes of all individuals. Note, that it is not recommended to run this program on a big plink file since it will fill your screen with data.

```c
#include <stdlib.h>
#include <stdio.h>
#include <plinkio/plinkio.h>

int
main(int argc, char *argv[])
{   
    struct pio_file_t plink_file;
    snp_t *snp_buffer;
    int sample_id;
    int locus_id;

    if( pio_open( &plink_file, argv[ 1 ] ) != PIO_OK )
    {
        printf( "Error: Could not open %s\n", argv[ 1 ] );
        return EXIT_FAILURE;
    }

    if( !pio_one_locus_per_row( &plink_file ) )
    {
        printf( "This script requires that snps are rows and samples columns.\n" );
        return EXIT_FAILURE;
    }

    locus_id = 0;
    snp_buffer = (snp_t *) malloc( pio_row_size( &plink_file ) );
    while( pio_next_row( &plink_file, snp_buffer ) == PIO_OK )
    {
        for( sample_id = 0; sample_id < pio_num_samples( &plink_file ); sample_id++)
        {
            struct pio_sample_t *sample = pio_get_sample( &plink_file, sample_id );
            struct pio_locus_t *locus = pio_get_locus( &plink_file, locus_id );
            printf( "Individual %s has genotype %d for snp %s.\n", sample->iid, snp_buffer[ sample_id ], locus->name );
        }

        locus_id++;
    }

    free( snp_buffer );
    pio_close( &plink_file );
    
    return EXIT_SUCCESS;
}
```

## Accessing sample and locus information in C

Information about samples and loci are obtained by referencing directly into the struct. The fields are summarized below.

### The pio_sample_t declaration

```c
/**
 * Data structure that contains the PLINK information about a sample (individual).
 */
struct pio_sample_t
{
    /**
     * An internal reference id, so that we can read them in order.
     */
    size_t pio_id;

    /**
     * Family identifier.
     */
    char *fid;

    /**
     * Plink individual identifier.
     */
    char *iid;

    /**
     * Plink individual identifier of father, 0 if none.
     */
    char *father_iid;

    /**
     * Plink individual identifier of mother, 0 if none.
     */
    char *mother_iid;

    /**
     * The sex of the individual.
     */
    enum sex_t sex;

    /**
     * Affection of the individuals, case, control or unkown. Control
     * is always 0 and case always 1.
     */
    enum affection_t affection;

    /**
     * A continuous phenotype of the individual.
     */
    float phenotype;
};
```

### The pio_locus_t declaration

```c
/**
 * Data structure that contains the PLINK information about a locus (SNP).
 */
struct pio_locus_t
{
    /**
     * An internal reference id, so that we can read them in order.
     */
    size_t pio_id;

    /**
     * Chromosome as strings.
     */
    char *chromosome;

    /**
     * Name of the SNP.
     */
    char *name;

    /**
     * Genetic position of the SNP.
     */
    float position;

    /**
     * Base pair position of the SNP.
     */
    long long bp_position;

    /**
     * First allele.
     */
    char *allele1;

    /**
     * Second allele.
     */
    char *allele2;
};
```

## Using in Python

The following script does the same as the above C program, utilizing most of the API.

```python
from plinkio import plinkfile

plink_file = plinkfile.open( "/path/to/plink_file" )
if not plink_file.one_locus_per_row( ):
     print( "This script requires that snps are rows and samples columns." )
     exit( 1 )

sample_list = plink_file.get_samples( )
locus_list = plink_file.get_loci( )

for locus, row in zip( locus_list, plink_file ):
    for sample, genotype in zip( sample_list, row ):
        print( "Individual {0} has genotype {1} for snp {2}.".format( sample.iid, genotype, locus.name ) )
```

## Accessing sample and locus information in Python

### The file API

```python
##
# Opens the plink file at the given path.
#
# @param path The prefix for a .bed, .fam and .bim without
#             the extension. E.g. for the files /plink/myfile.fam,
#             /plink/myfile.bim, /plink/myfile.bed use the path
#             /plink/myfile
#
def open(path):
    pass

##
# Creates a new plink file based on the given samples.
#
# @param path The prefix for a .bed, .fam and .bim without
#             the extension. E.g. for the files /plink/myfile.fam,
#             /plink/myfile.bim, /plink/myfile.bed use the path
#             /plink/myfile
# @param samples A list of Sample objects to write to the file.
#
def create(path, samples):
    pass
```

### The Sample object

```python
class Sample:
    def __init__(self, fid, iid, father_iid, mother_iid, sex, affection, phenotype = 0.0):
        ##
        # Family id.
        #
        self.fid = fid

        ##
        # Individual id.
        #
        self.iid = iid

        ##
        # Individual id of father.
        #
        self.father_iid = father_iid

        ##
        # Individual id of mother.
        #
        self.mother_iid = mother_iid

        ##
        # Sex of individual.
        #
        self.sex = sex

        ##
        # Affection of individual, 0/1, control/case
        #
        self.affection = affection

        ##
        # Optional continuous phenotype, will be 0.0/1.0 if control/case
        #
        self.phenotype = phenotype
```

### The Locus object

```python
class Locus:
    def __init__(self, chromosome, name, position, bp_position, allele1, allele2):
        ##
        # Chromosome string
        #
        self.chromosome = chromosome

        ##
        # Name of the loci, usually rs-number or
        # chrX:pos.
        #
        self.name = name

        ##
        # Genetic position (floating point number).
        #
        self.position = position

        ##
        # Base pair position (integer).
        #
        self.bp_position = bp_position

        ##
        # First allele
        #
        self.allele1 = allele1

        ##
        # Second allele
        #
        self.allele2 = allele2
```

### The PlinkFile object

```python
class PlinkFile:
    ##
    # Returns the prefix path to the plink file, e.g.
    # without .bim, .bed or .fam.
    #
    def get_path():
        pass

    ##
    # Returns a list of the Sample objects.
    #
    def get_samples():
        pass

    ##
    # Returns a list of Locus objects.
    #
    def get_loci():
        pass

    ##
    # Determines how the snps are stored. It will return
    # true if a row contains the genotypes of all individuals
    # from a single locus, false otherwise.
    #
    def one_locus_per_row():
        pass

    ##
    # Closes the file.
    #
    def close():
        pass

    ##
    # Transposes the file.
    #
    # @param new_path Prefix of the new plink file.
    #
    def transpose(new_path):
        pass
```

### The WritablePlinkFile object

```python
class WritablePlinkFile: 
    ##
    # Returns a list of Sample objects.
    #
    def get_samples():
        pass

    ##
    # Returns a list of Locus objects written so far.
    #
    def get_loci():
        pass

    ##
    # Takes a locus and the corresponding genotypes and
    # writes them to the plink file.
    # 
    # @param locus A Locus object to write.
    # @param row An indexable list of genotypes.
    #
    def write_row(locus, row):
        pass
    
    ##
    # Closes the file.
    #
    def close():
        pass
```
