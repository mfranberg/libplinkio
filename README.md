# libplinkio

This is a small C and Python library for reading Plink genotype files. 

Currently it can:
* Read and parse BED, BIM and FAM files.
* Transpose BED files.

Libplinkio will reach 1.0 when it can:
* Read PED files (i.e. non-binary bed-files).
* Write BED, BIM and FAM files.

Project rationales:
* Use C to make it as simple as possible to add bindings
  for other languages.
* Focus only on IO-functionality.
* Few external dependencies to make it easy to use.

## Installing

Installing this library is easy, just **configure** and **make**. This will also install Python bindings for the active interpeter.

### Installing to a standard location

    mkdir build
    cd build
    ../configure
    make && make check && sudo make install

You can also pass the --disable-tests flag to **configure** to avoid building the unit tests and the dependency to libcmockery. Note howerver, in this case **make check** will not do anything.

### Installing to a custom location

    mkdir build
    cd build
    ../configure --prefix=/path/to/plinkio
    make && make check && make install

### Linking to your program

To link your own application to libplinkio you can use the following include and library paths after installing it:

    gcc -lplinkio source.c

If you installed libplinkio to a custom location you need to specify the location of libplinkio:

    gcc -lplinkio -I/path/to/plinkio/include -L/path/to/plinkio/lib source.c

## Using in C

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

    if( pio_open( &plink_file, "/path/to/plink_file" ) != PIO_OK )
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
     * Chromosome number starting from 1.
     */
    unsigned char chromosome;

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
        # Chromosome number starting from 1
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
