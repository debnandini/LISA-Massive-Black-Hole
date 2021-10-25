from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

unique_extension = Extension(
        name="pyunique",
	sources=["pyunique.pyx"],
	libraries=["unique"],
	library_dirs=["lib"],
	include_dirs=["lib"]
)
setup(
	name="pyunique",
	ext_modules=cythonize([unique_extension])
)

