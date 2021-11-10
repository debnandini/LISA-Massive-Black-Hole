cdef extern from "search.h":
	int main_gut(int seg)

def py_main_gut(int seg):
	main_gut(seg)
