#!/usr/bin/env python3
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

search_extension = Extension(
        name="pysearch",
	sources=["pysearch.pyx","IMRPhenomD_internals.c","IMRPhenomD.c"],
	libraries=["search","IMRPhenomD_internals","IMRPhenomD","omp","gsl","gslcblas"],
	library_dirs=["lib","/opt/local/lib/libomp","/opt/local/lib"],
	include_dirs=["lib","/opt/local/include/libomp","/opt/local/include/gsl","/opt/local/include"]
)
setup(
	name="pysearch",
	ext_modules=cythonize([search_extension])
)

