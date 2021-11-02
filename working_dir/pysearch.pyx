cdef extern from "search.h":
	int main(int argc, char *argv[])

def py_main(int argc, list argv):
	cdef vector[char] segvec
	segvec.resize(argc)
	segvec[0]=0
	segvec[1]=argv[1]
	main(argc, segvec)
