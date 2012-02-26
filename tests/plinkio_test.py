import unittest

from plinkio import plinkfile

##
# Tests that opening, reading and closing
# of plink files works from python.
#
class TestPlinkIo(unittest.TestCase):
    ##
    # Test open.
    #
    def test_open(self):
        pf = plinkfile.open( "./data/wgas" )
        self.assertNotEqual( pf, None );

    ##
    # Make sure open throws IOError when
    # an invalid path is specified.
    #
    def test_fail_open(self):
        with self.assertRaises( IOError ):
            plinkfile.open( "/" )

    ##
    # Test iteration.
    #
    def test_iter(self):
        pf = plinkfile.open( "./data/wgas" )

        num_rows = 0
        for row in pf:
            num_rows += 1

        self.assertEqual( num_rows, 94 )

##
# Tests the snp array.
#
class TestSnpArray(unittest.TestCase):
    ##
    # Common setup.
    #
    def setUp(self):
        pf = plinkfile.open( "./data/wgas" )
        self.row = next( pf )
        pf.close( )

    ##
    # Test access operator.
    #
    def test_access(self):
        self.assertEqual( self.row[ 0 ], 0 )
        self.assertEqual( self.row[ 2 ], 1 )

    ##
    # Test contains.
    #
    def test_contains(self):
        self.assertTrue( 0 in self.row )
        self.assertTrue( 1 in self.row )
        self.assertFalse( 2 in self.row )
        self.assertTrue( 3 in self.row )

    ##
    # Test allele count on first row.
    #
    def test_count(self):
        self.assertEqual( self.row.allele_counts( ), [ 67, 22, 0, 1 ] )

if __name__ == '__main__':
    unittest.main( )
