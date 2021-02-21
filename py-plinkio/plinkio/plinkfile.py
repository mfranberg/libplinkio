from . import cplinkio


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
        self.path = path
        self.handle = cplinkio.open(path)
        self.loci = cplinkio.get_loci(self.handle)
        self.samples = cplinkio.get_samples(self.handle)

    ##
    # Returns an iterator from the beginning of
    # the file.
    #
    def __iter__(self):
        cplinkio.reset_row(self.handle)

        return self

    ##
    # Returns the prefix path to the plink file, e.g.
    # without .bim, .bed or .fam.
    #
    def get_path(self):
        return self.path

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
        return cplinkio.one_locus_per_row(self.handle)

    ##
    # Goes to next row.
    #
    def next(self):
        row = cplinkio.next_row(self.handle)
        if not row:
            raise StopIteration

        return row

    ##
    # For python 3.x.
    #
    def __next__(self):
        return self.next()

    ##
    # Closes the file.
    #
    def close(self):
        if self.handle:
            cplinkio.close(self.handle)
            self.handle = None

    ##
    # Transposes the file.
    #
    def transpose(self, new_path):
        return cplinkio.transpose(self.path, new_path)


class WritablePlinkFile:
    ##
    # Creates the plink file at the given path containing the given
    # samples. Their genotypes can then be written one row at a time.
    # This file may not be read simulatenously.
    #
    # @param path The prefix for a .bed, .fam and .bim without
    #             the extension. E.g. for the files /plink/myfile.fam,
    #             /plink/myfile.bim, /plink/myfile.bed use the path
    #             /plink/myfile
    # @param samples A list of Sample objects which are the final subjects
    #                that will be in the file.
    #
    def __init__(self, path, samples):
        self.path = path
        self.samples = samples
        self.loci = []
        self.handle = cplinkio.create(path, samples)

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
    # Takes a locus and the corresponding genotypes and
    # writes them to the plink file.
    #
    # @param locus A Locus object to write.
    # @param row An indexable list of genotypes.
    #
    def write_row(self, locus, row):
        cplinkio.write_row(self.handle, locus, row)
        self.loci.append(locus)

    ##
    # Closes the file.
    #
    def close(self):
        if self.handle:
            cplinkio.close(self.handle)
            self.samples = None
            self.loci = None
            self.handle = None


class Sample:
    def __init__(self, fid, iid, father_iid, mother_iid, sex, affection, phenotype=0.0):
        # pylint: disable = too-many-arguments
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
        # 0 is female
        # 1 is male
        # anything else is unknown.
        #
        self.sex = sex

        ##
        # Affection of individual, 0/1, control/case
        #
        # 0 is control
        # 1 is case
        # -9 is missing
        # anything else means that the phenotype is continuous.
        #
        self.affection = affection

        ##
        # Optional continuous phenotype, will be 0.0/1.0 if control/case
        #
        self.phenotype = phenotype

    def __str__(self):
        return "{0} {1} {2} {3}".format(self.fid, self.iid, self.sex, self.affection)


class Locus:
    __slots__ = ["chromosome", "name", "position", "bp_position", "allele1", "allele2"]

    def __init__(self, chromosome, name, position, bp_position, allele1, allele2):
        # pylint: disable = too-many-arguments
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
        # Genetic position (floating point).
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

    def __str__(self):
        return "{0} {1}".format(self.chromosome, self.name)


##
# Opens the plink file at the given path.
#
# @param path The prefix for a .bed, .fam and .bim without
#             the extension. E.g. for the files /plink/myfile.fam,
#             /plink/myfile.bim, /plink/myfile.bed use the path
#             /plink/myfile
#
def open(path):
    # pylint: disable = redefined-builtin
    return PlinkFile(path)


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
    return WritablePlinkFile(path, samples)
