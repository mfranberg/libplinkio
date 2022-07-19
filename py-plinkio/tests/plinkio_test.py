import os
import pytest

from plinkio import plinkfile

HERE = os.path.abspath(os.path.dirname(__file__))
TEST_DATA_PREFIX = os.path.join(HERE, "../../tests/data/wgas")

##
# Tests that opening, reading and closing
# of plink files works from python.
#


##
# Test open.
#
def test_open():
    test_pf = plinkfile.open(TEST_DATA_PREFIX)
    assert test_pf is not None


##
# Make sure that the path is correct.
#
def test_get_path():
    path = TEST_DATA_PREFIX
    test_pf = plinkfile.open(path)
    assert path == test_pf.get_path()


##
# Make sure that the number of parsed samples
# is correct.
#
def test_get_samples():
    test_pf = plinkfile.open(TEST_DATA_PREFIX)
    assert len(test_pf.get_samples()) == 90


##
# Make sure that the number parsed loci is
# correct.
#
def test_get_loci():
    test_pf = plinkfile.open(TEST_DATA_PREFIX)
    assert len(test_pf.get_loci()) == 228694


##
# Make sure open throws IOError when
# an invalid path is specified.
#
def test_fail_open():
    with pytest.raises(IOError):
        plinkfile.open("/")


##
# Test iteration.
#
def test_iter():
    test_pf = plinkfile.open(TEST_DATA_PREFIX)

    num_rows = 0
    for _ in test_pf:
        num_rows += 1

    assert num_rows == 228694


##
# Tests the snp array.
#
class TestSnpArray:
    # pylint: disable=unused-argument,attribute-defined-outside-init

    ##
    # Common setup.
    #
    @pytest.fixture
    def fixture_setup(self):
        test_pf = plinkfile.open(TEST_DATA_PREFIX)
        self.row = next(test_pf)
        test_pf.close()

    ##
    # Test access operator.
    #
    def test_access(self, fixture_setup):
        assert self.row[0] == 2
        assert self.row[2] == 1

    ##
    # Test contains.
    #
    def test_contains(self, fixture_setup):
        assert 0 not in self.row
        assert 1 in self.row
        assert 2 in self.row
        assert 3 in self.row

    ##
    # Test allele count on first row.
    #
    def test_count(self, fixture_setup):
        assert self.row.allele_counts() == [0, 22, 67, 1]
