import os
import tempfile
import hashlib
import re
import contextlib

from plinkio import plinkfile
from plinkio.plinkfile import Sample, Locus

HERE = os.path.abspath(os.path.dirname(__file__))


##
# Verify that the contents of bed files are the same.
#
def compare_bed_files(test_prefix, write_prefix):
    with open(test_prefix + ".bed", "rb") as file:
        data = file.read()
        test_hash_bed = hashlib.sha256(data).hexdigest()
    with open(write_prefix + ".bed", "rb") as file:
        data = file.read()
        write_hash = hashlib.sha256(data).hexdigest()
    assert test_hash_bed == write_hash


##
# Verify that the contents of bim files are the same.
#
def compare_bim_files(test_prefix, write_prefix):
    with contextlib.ExitStack() as stack:
        f_test = stack.enter_context(open(test_prefix + ".bim", "r", encoding="utf-8"))
        f_write = stack.enter_context(open(write_prefix + ".bim", "r", encoding="utf-8"))

        assert len(f_test.readlines()) == len(f_write.readlines())

        for line_test, line_write in zip(f_test.readlines(), f_write.readlines()):
            tokens_test = re.split(r"\s+", line_test)
            tokens_write = re.split(r"\s+", line_write)
            assert int(tokens_test[0]) == int(tokens_write[0])
            assert tokens_test[1] == tokens_write[1]
            assert (
                (float(tokens_test[2]) - float(tokens_write[2])) / float(tokens_test[2])
            ) < 1.0e-6
            assert int(tokens_test[3]) == int(tokens_write[3])
            assert tokens_test[4] == tokens_write[4]
            assert tokens_test[5] == tokens_write[5]


##
# Verify that the contents of fam files are the same.
#
def compare_fam_files(test_prefix, write_prefix):
    with contextlib.ExitStack() as stack:
        f_test = stack.enter_context(open(test_prefix + ".fam", "r", encoding="utf-8"))
        f_write = stack.enter_context(open(write_prefix + ".fam", "r", encoding="utf-8"))

        assert len(f_test.readlines()) == len(f_write.readlines())

        for line_test, line_write in zip(f_test.readlines(), f_write.readlines()):
            tokens_test = re.split(r"\s+", line_test)
            tokens_write = re.split(r"\s+", line_write)
            assert tokens_test[0] == tokens_write[0]
            assert tokens_test[1] == tokens_write[1]
            assert int(tokens_test[2]) == int(tokens_write[2])
            assert int(tokens_test[3]) == int(tokens_write[3])
            assert int(tokens_test[4]) == int(tokens_write[4])
            assert int(tokens_test[5]) == int(tokens_write[5])


##
# Tests reading existing plink files.
# The test bed file contains CR + LF (0x0D 0x0A).
#
def test_read_existing_data():
    test_data_prefix = os.path.join(HERE, "./data/crlf")
    test_pf = plinkfile.open(test_data_prefix)
    samples = test_pf.get_samples()
    loci = test_pf.get_loci()

    correct_samples = [
        Sample("fid1", "iid1", "0", "0", 0, 0),
        Sample("fid2", "iid2", "0", "0", 0, 1),
        Sample("fid3", "iid3", "0", "0", 0, 0),
        Sample("fid4", "iid4", "0", "0", 0, 1),
    ]
    correct_loci = [
        Locus(1, "chr1:1", 1.0, 1, "A", "C"),
        Locus(2, "chr1:2", 2.0, 2, "G", "T"),
    ]
    correct_rows = [
        [3, 2, 0, 0],
        [1, 1, 0, 0],
    ]
    assert samples == correct_samples
    assert loci == correct_loci

    with tempfile.TemporaryDirectory() as temp_dir:
        write_data_prefix = os.path.join(temp_dir, "test")
        writer = plinkfile.create(write_data_prefix, samples)

        for row, correct_row, locus in zip(test_pf, correct_rows, loci):
            assert list(row) == correct_row
            writer.write_row(locus, row)

        writer.close()

        compare_bed_files(test_data_prefix, write_data_prefix)
        compare_bim_files(test_data_prefix, write_data_prefix)
        compare_fam_files(test_data_prefix, write_data_prefix)


##
# Tests reading existing plink text files.
# The test bed file contains CR + LF (0x0D 0x0A).
#
def test_read_existing_txt_data():
    test_data_prefix = os.path.join(HERE, "./data/crlf")
    test_pf = plinkfile.open(test_data_prefix, is_txt=True, experimental_mode=True)
    samples = test_pf.get_samples()
    loci = test_pf.get_loci()

    correct_samples = [
        Sample("fid1", "iid1", "0", "0", 0, 0),
        Sample("fid2", "iid2", "0", "0", 0, 1),
        Sample("fid3", "iid3", "0", "0", 0, 0),
        Sample("fid4", "iid4", "0", "0", 0, 1),
    ]
    correct_loci = [
        Locus(1, "chr1:1", 1.0, 1, "C", "A"),
        Locus(2, "chr1:2", 2.0, 2, "T", "G"),
    ]
    correct_rows = [
        [3, 0, 2, 2],
        [1, 1, 2, 2],
    ]

    assert samples == correct_samples
    assert loci == correct_loci

    for row, correct_row in zip(test_pf, correct_rows):
        assert list(row) == correct_row


##
# Tests reading existing plink text files.
# The test bed file contains CR + LF (0x0D 0x0A).
#
def test_read_existing_compound_txt_data():
    test_data_prefix = os.path.join(HERE, "./data/crlf_compound")
    test_pf = plinkfile.open(test_data_prefix, is_txt=True, experimental_mode=True)
    samples = test_pf.get_samples()
    loci = test_pf.get_loci()

    correct_samples = [
        Sample("fid1", "iid1", "0", "0", 0, 0),
        Sample("fid2", "iid2", "0", "0", 0, 1),
        Sample("fid3", "iid3", "0", "0", 0, 0),
        Sample("fid4", "iid4", "0", "0", 0, 1),
    ]
    correct_loci = [
        Locus(1, "chr1:1", 1.0, 1, "C", "A"),
        Locus(2, "chr1:2", 2.0, 2, "T", "G"),
    ]
    correct_rows = [
        [3, 0, 2, 2],
        [1, 1, 2, 2],
    ]

    assert samples == correct_samples
    assert loci == correct_loci

    for row, correct_row in zip(test_pf, correct_rows):
        assert list(row) == correct_row


##
# Tests writing new plink files.
#
def test_write_new_data():
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

        reader.close()
