#!/bin/sh
CFLAGS="-I/opt/local/include/libomp"  \
LDFLAGS="-L/opt/local/lib/libomp"     \
	python3 setup.py build_ext --inplace    

