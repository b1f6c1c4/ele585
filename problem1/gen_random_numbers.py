#!/usr/bin/env python

import sys
import getopt
import random

def usage(argv):
	print argv[0], "[-hv] --seed SEED --ofile OUTPUT_FILE --number LENGTH --max MAX_VALUE"

def main(argv=None):
	if argv is None:
		argv = sys.argv
	
	try:
		opts, args = getopt.getopt(argv[1:], "hs:o:n:m:v", ["help", "seed=", "ofile=", "number=", "max="])
	except getopt.GetoptError, err:
        	# print help information and exit:
		print str(err) # will print something like "option -a not recognized"
		usage(argv)
		return 2
	ofile = None
	seed = None
	number = None
	max = None
	verbose = False
	for o, a in opts:
        	if o == "-v":
            		verbose = True
		elif o in ("-h", "--help"):
			usage(argv)
            		return 0
		elif o in ("-s", "--seed"):
			seed = a
		elif o in ("-o", "--ofile"):
			ofile = a
		elif o in ("-n", "--number"):
			number = a
		elif o in ("-m", "--max"):
			max = a
		else:
			assert False, "unhandled option"
	if (ofile == None) or (seed == None) or (number == None) or (max == None):
		usage(argv)
		return 2
	print number
	# Initialize the random number generator
	random.seed(seed)
	# Open output file
	file = open(ofile, 'w')
	for n in xrange(int(number)):
		rand_num = random.randint(0, int(max))
		output_string = "%d\n" % rand_num
		file.write(output_string)
	file.close()

if __name__ == "__main__":
	sys.exit(main())
