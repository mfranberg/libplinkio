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

## Using in Python

The following script does the same as the above C program, utilizing most of the API.

```python
from plinkio import plinkfile

plink_file = plinkfile.open( "/path/to/plink" )
if not plink_file.one_locus_per_row( ):
     print( "This script requires that snps are rows and samples columns." )
     exit( 1 )

sample_list = plink_file.get_samples( )
locus_list = plink_file.get_loci( )

for locus, row in zip( locus_list, plink_file ):
    for sample, genotype in zip( sample_list, row ):
        print( "Individual {0} has genotype {1} for snp {2}.".format( sample.iid, locus.name, genotype ) )
```
