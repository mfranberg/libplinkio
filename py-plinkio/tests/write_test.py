import os
import tempfile

from plinkio import plinkfile
from plinkio.plinkfile import Sample, Locus


def test_read_write():
    with tempfile.TemporaryDirectory() as temp_dir:
        plink_prefix = os.path.join(temp_dir, "test")

        samples = [Sample("fid1", "iid1", "0", "0", 0, 0), Sample("fid2", "iid2", "0", "0", 0, 1)]
        loci = [Locus(1, "chr1:1", 1.0, 1, "A", "C"), Locus(2, "chr1:2", 2.0, 2, "G", "T")]
        rows = [[0, 1], [1, 2]]

        writer = plinkfile.create(plink_prefix, samples)

        for i, locus in enumerate(loci):
            writer.write_row(locus, rows[i])

        writer.close()

        reader = plinkfile.open(plink_prefix)
        assert samples == reader.get_samples()
        assert loci == reader.get_loci()

        for row, reader_row in zip(rows, reader):
            assert row == list(reader_row)
