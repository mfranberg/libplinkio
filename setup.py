from setuptools import setup, find_packages, Extension
from codecs import open
from os import path
import os
import glob

here = path.abspath(path.dirname(__file__))

# Get the long description from the relevant file
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

libplinkio_src_dir = "src"
libplinkio_include_dir = os.path.join( libplinkio_src_dir )
libplinkio_src_files = glob.glob( os.path.join( libplinkio_src_dir, "*.c" ) )

libcsv_root_dir = os.path.join( "libs", "libcsv" )
libcsv_src_dir = os.path.join( libcsv_root_dir, "src" )
libcsv_include_dir = os.path.join( libcsv_root_dir, "inc" )
libcsv_src_files = glob.glob( os.path.join( libcsv_src_dir, "*.c" ) )

pyplinkio_src_dir = "py-plinkio"
pyplinkio_include_dir = pyplinkio_src_dir
pyplinkio_src_files = glob.glob( os.path.join( pyplinkio_src_dir, "*.c" ) )

cplinkio = Extension(
    "plinkio.cplinkio",
    libplinkio_src_files + libcsv_src_files + pyplinkio_src_files,
    library_dirs=[],
    include_dirs=[libplinkio_include_dir, libcsv_include_dir, pyplinkio_include_dir],
    libraries=[],
    language="c",
    extra_compile_args=[],
    define_macros=[]
)

setup(
    name='plinkio',

    # Versions should comply with PEP440.  For a discussion on single-sourcing
    # the version across setup.py and the project code, see
    # https://packaging.python.org/en/latest/single_source_version.html
    version='0.9.7',

    description='A library for parsing plink genotype files',
    long_description=long_description,

    # The project's main homepage.
    url='https://github.com/mfranberg/libplinkio',

    # Author details
    author='Mattias Franberg',
    author_email='mattias.franberg@gmail.com',

    # Choose your license
    license='BSD',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 4 - Beta',

        # Indicate who your project is intended for
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering :: Bio-Informatics',

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: BSD License',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
    ],

    # What does your project relate to?
    keywords='plinkio bioinformatics genetics',

    # You can just specify the packages manually here if your project is
    # simple. Or you can use find_packages().
    packages=find_packages( "py-plinkio" ),

    # List run-time dependencies here.  These will be installed by pip when your
    # project is installed. For an analysis of "install_requires" vs pip's
    # requirements files see:
    # https://packaging.python.org/en/latest/requirements.html
    install_requires=[],

    # List additional groups of dependencies here (e.g. development dependencies).
    # You can install these using the following syntax, for example:
    # $ pip install -e .[dev,test]
    extras_require = {
    },

    ext_modules = [ cplinkio ],

    # If there are data files included in your packages that need to be
    # installed, specify them here.  If using Python 2.6 or less, then these
    # have to be included in MANIFEST.in as well.
    package_data={
    },

    package_dir={
        '' : "py-plinkio"
    },

    # Although 'package_data' is the preferred approach, in some case you may
    # need to place data files outside of your packages.
    # see http://docs.python.org/3.4/distutils/setupscript.html#installing-additional-files
    # In this case, 'data_file' will be installed into '<sys.prefix>/my_data'
    data_files=[],

    # To provide executable scripts, use entry points in preference to the
    # "scripts" keyword. Entry points provide cross-platform support and allow
    # pip to create the appropriate form of executable for the target platform.
    entry_points={
    },
)
