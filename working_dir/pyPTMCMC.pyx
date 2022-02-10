cdef extern from "PTMCMC.h":
	int main_gut(int seg, int rep)

def py_main_gut(int seg, int rep):
	main_gut(seg, rep)
