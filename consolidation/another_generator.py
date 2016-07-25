#!/usr/bin/python3

import sys
import random 

try:
    nb_lines = int(sys.argv[1])
    max_value = int(sys.argv[2])
    file = open(sys.argv[3], "w")
except:
        print("Nope!")
        exit(-1)


for i in range(0, nb_lines):
    file.write( str(random.randint(0, max_value)) + '\n');


