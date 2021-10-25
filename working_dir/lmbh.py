import subprocess
import sys
import os

subprocess.call(["gcc", "segmentSangria.c", "-osegmentSangria", "-lhdf5", "-lgsl"])
subprocess.call(["./segmentSangria"])
