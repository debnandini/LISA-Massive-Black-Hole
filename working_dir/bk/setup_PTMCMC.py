#!/usr/bin/env python3
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

PTMCMC_extension = Extension(
        name="pyPTMCMC",
	sources=["pyPTMCMC.pyx","Utils.c","Response.c","IMRPhenomD_internals.c","IMRPhenomD.c"],
	libraries=["PTMCMC","Utils","Response","IMRPhenomD_internals","IMRPhenomD","omp","gsl","gslcblas"],
	library_dirs=["lib","/opt/local/lib/libomp","/opt/local/lib"],
	include_dirs=["lib","/opt/local/include/libomp","/opt/local/include/gsl","/opt/local/include"]
)
setup(
	name="pyPTMCMC",
	ext_modules=cythonize([PTMCMC_extension])
)

