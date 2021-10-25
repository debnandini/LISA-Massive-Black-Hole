from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

search_extension = Extension(
        name="pysearch",
	sources=["pysearch.pyx"],
	libraries=["search"],
	library_dirs=["lib"],
	include_dirs=["lib"]
)
setup(
	name="pysearch",
	ext_modules=cythonize([search_extension])
)

