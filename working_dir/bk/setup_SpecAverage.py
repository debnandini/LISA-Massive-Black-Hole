from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

SpecAverage_extension = Extension(
        name="pySpecAverage",
	sources=["pySpecAverage.pyx"],
	libraries=["SpecAverage"],
	library_dirs=["lib"],
	include_dirs=["lib"]
)
setup(
	name="pySpecAverage",
	ext_modules=cythonize([SpecAverage_extension])
)

