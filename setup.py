from setuptools import setup, Extension
import os
import glob
import sys

libplinkio_src_dir = "src"
libplinkio_include_dir = libplinkio_src_dir
libplinkio_src_files = glob.glob(os.path.join(libplinkio_src_dir, "*.c"))

pyplinkio_src_dir = "py-plinkio/src/cplinkio"
pyplinkio_include_dir = pyplinkio_src_dir
pyplinkio_src_files = glob.glob(os.path.join(pyplinkio_src_dir, "*.c"))

libraries = []
if sys.platform == 'win32':
    libraries.append("Bcrypt")

cplinkio = Extension(
    "plinkio.cplinkio",
    libplinkio_src_files + pyplinkio_src_files,
    library_dirs=[],
    include_dirs=[
        libplinkio_include_dir,
        pyplinkio_include_dir,
    ],
    libraries=libraries,
    language="c",
    extra_compile_args=[],
    define_macros=[],
)

setup(
    ext_modules=[cplinkio],
)
