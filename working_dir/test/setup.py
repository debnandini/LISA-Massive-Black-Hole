from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

max_extension = Extension(
    name="pymax",
    sources=["pymax.pyx"],
    libraries=["max"],
    library_dirs=["lib"],
    include_dirs=["lib"]
)
setup(
	name="pymax",
	ext_modules=cythonize([max_extension])
)
