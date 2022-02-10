#!/usr/bin/env python3
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

SpecFit_extension = Extension(
        name="pySpecFit",
	sources=["pySpecFit.pyx","Utilities.c"],
	libraries=["SpecFit","Utilities","omp","gsl", "gslcblas"],
	library_dirs=["lib","/opt/local/lib/libomp","/opt/local/lib"],
	include_dirs=["lib","/opt/local/include/libomp","/opt/local/include/gsl","/opt/local/include"]
)
setup(
	name="pySpecFit",
	ext_modules=cythonize([SpecFit_extension])
)

