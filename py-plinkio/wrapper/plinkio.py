import cplinkio

class PlinkFile: 
    ##
    # Opens the plink file at the given path.
    #
    # @param path The prefix for a .bed, .fam and .bim without
    #             the extension. E.g. for the files /plink/myfile.fam,
    #             /plink/myfile.bim, /plink/myfile.bed use the path
    #             /plink/myfile
    #
    def __init__(self, path):
        self.handle = cplinkio.open( path )
        self.loci = cplinkio.get_loci( self.handle )
        self.samples = cplinkio.get_samples( self.handle )

    ##
    # Returns an iterator from the beginning of
    # the file.
    #
    def __iter__(self):
        cplinkio.reset_row( self.handle )

        return self

    ##
    # Clean-up.
    #
    def __del__(self):
        self.close( )

    ##
    # Returns a list of the samples.
    #
    def get_samples(self):
        return self.samples

    ##
    # Returns a list of the loci.
    #
    def get_loci(self):
        return self.loci

    ##
    # Determines how the snps are stored. It will return
    # true if a row contains the genotypes of all individuals
    # from a single locus, false otherwise.
    #
    def one_locus_per_row(self):
        return cplinkio.one_locus_per_row( self.handle )

    ##
    # Goes to next row.
    #
    def next(self):
        row = cplinkio.next_row( self.handle )
        if not row:
            raise StopIteration

        return row

    ##
    # Closes the file.
    #
    def close(self):
        if self.handle:
            cplinkio.close( self.handle )
            self.handle = None

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
        # Affection of individual, 0/1, case/control
        #
        self.affection = affection

        ##
        # Optional continuous phenotype
        #
        self.phenotype = phenotype

    def __str__(self):
        return "{0} {1} {2} {3}".format( self.fid, self.iid, self.sex, self.affection )

class Locus:
    def __init__(self, chromosome, name, position, bp_position, major, minor):
        ##
        # Chromosome number starting from 1
        #
        self.chromosome = chromosome

        ##
        # Name of the loci, usually RS number or
        # chrX:pos.
        #
        self.name = name

        ##
        # Position.
        #
        self.position = position

        ##
        # Base pair position.
        #
        self.bp_position = bp_position

        ##
        # Major allele A, C, G or T.
        #
        self.major = major

        ##
        # Minor allele A, C, G, or T.
        #
        self.minor = minor

    def __str__(self):
        return "{0} {1}".format( chromosome, name )

##
# Opens the plink file at the given path.
#
# @param path The prefix for a .bed, .fam and .bim without
#             the extension. E.g. for the files /plink/myfile.fam,
#             /plink/myfile.bim, /plink/myfile.bed use the path
#             /plink/myfile
#
def open(path):
    return PlinkFile( path )
