#!/bin/sh

#SpecFit
rm pySpecFit.*.so
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c SpecFit.c Utilities.c -lgsl  -lm
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c Utilities.c -lgsl  -lm
ar rcs libSpecFit.a SpecFit.o
ar rcs libUtilities.a Utilities.o
mv *.a ./lib/
cp ./bk/setup_SpecFit.py ./setup.py
./setup_search.sh
rm ./setup.py
mv ./build ./bk/build_SpecFit

#SpecAverage
rm pySpecAverage.*.so
gcc -c SpecAverage.c -lgsl
ar rcs libSpecAverage.a SpecAverage.o
mv *.a ./lib/
cp ./bk/setup_SpecAverage.py ./setup.py
./setup_search.sh
rm ./setup.py
mv ./build ./bk/build_Average

#search
rm pysearch.*.so 
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c search.c IMRPhenomD_internals.c IMRPhenomD.c -lgsl -lgslcblas  -lm
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c IMRPhenomD_internals.c -lgsl -lgslcblas -lm
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c IMRPhenomD.c -lgsl -lgslcblas  -lm
ar rcs libsearch.a search.o
ar rcs libIMRPhenomD_internals.a IMRPhenomD_internals.o
ar rcs libIMRPhenomD.a IMRPhenomD.o
mv ./*.a ./lib/
cp ./bk/setup_search.py ./setup.py
./setup_search.sh
rm ./setup.py
mv ./build ./bk/build_search

#unique
rm pyunique.*.so
gcc -c unique.c -lgsl
ar rcs libunique.a unique.o
mv *.a ./lib/
cp ./bk/setup_unique.py ./setup.py
./setup_search.sh
rm ./setup.py
mv ./build ./bk/build_unique

#PTMCMC
rm pyPTMCMC.*.so
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c PTMCMC.c Utils.c Response.c IMRPhenomD_internals.c IMRPhenomD.c -lgsl -lgslcblas  -lm
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c Utils.c -lgsl -lgslcblas  -lm
gcc -Xpreprocessor -I/opt/local/include/libomp -L/opt/local/lib/libomp -lomp -w -c Response.c -lgsl -lgslcblas  -lm
ar rcs libPTMCMC.a PTMCMC.o
ar rcs libUtils.a Utils.o
ar rcs libResponse.a Response.o
mv ./*.a ./lib/
cp ./bk/setup_PTMCMC.py ./setup.py
./setup_search.sh
rm ./setup.py
mv ./build ./bk/build_PTMCMC
