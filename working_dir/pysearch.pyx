cdef extern from "search.h":
	int main(int argc, char *argv[]):
		segvec=[]
		segvec.resize(argc)
		segvec[0]=0
		segvec[1]=argv[1]
		segvec[2]=argv[2]
		main(argc, segvec)

def py_main(int argc, list segvec):
	main(argc, segvec)
