from optparse import OptionParser
import subprocess
import sys
import os
import pySpecFit
import pySpecAverage
import pysearch
import pyunique
import pyPTMCMC

def parse_command_line():
	parser = OptionParser(
	)
	parser.add_option("-s", "--segments", type = "int", help = "number of segments to compute psd for (not segment number).(Use 1 for non segmented data)")
	parser.add_option("-i", "--segment-start", type = "int", help = "segment to start analysis from (provide segment numbers 0-11). (Use 0 for non segmented data)")
	parser.add_option("-f", "--segment-end", type = "int", help = "segment to end analysis at. (Use 0 for non segmented data)")
	parser.add_option("-p", "--Pseg", type = "int", help = "segment to run PTMCMC on. Use -1 to run over full data set")
	parser.add_option("-o", "--source", type = "int", help = "source number to run PTMCMC on. Note: Sangria training data has 15 unique sources numbered 0 to 14")
	parser.add_option("-v", "--verbose", action = "store_true", help = "Be verbose.")

	options = parser.parse_args()
	return options

#
# =============================================================================
#
#                                     Main
#
# =============================================================================
#


#
# parse command line
#


options = parse_command_line()
seg = options.segments
segs = options.segment_start
sege = options.segment_end
pseg = options.Pseg
source = options.source

# make segments

subprocess.call(["gcc", "segmentSangria.c", "-osegmentSangria", "-lhdf5", "-lgsl"])
subprocess.call(["./segmentSangria"])

# psd estimate

for j in range(seg):
	for i in range(2):
		pySpecFit.py_main_gut(b"AET_seg%d_t.dat"%(j), i)
		os.rename('specfit.dat', 'specfit_%d_%d.dat'%(i,j))
		subprocess.call(["gnuplot", "Qscan.gnu"])
		os.rename('Qscan.png', 'Qscan_%d_%d.png'%(i,j))

# average and interpolate psd

pySpecAverage.py_main()

# search

for k in range(segs, sege+1):
	pysearch.py_main_gut(k)

# select signals with snr > 12

pyunique.py_main()

# PTMCMC

pyPTMCMC.py_main_gut(pseg, source)

#FIXME remove unnecesary files here
