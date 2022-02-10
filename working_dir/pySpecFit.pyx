cdef extern from "SpecFit.h":
	int main_gut(char *file, int ch)

def py_main_gut(bytes file, int ch):
	main_gut(file, ch)
