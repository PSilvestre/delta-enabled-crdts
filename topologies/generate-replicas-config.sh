#!/usr/bin/env python

import sys
from os import system

if len(sys.argv) < 3:
  print "Usage: " + sys.argv[0] + " REPLICAS_CONFIG_DIR EXECUTIONS_NUMBER"
  sys.exit()

replicas_config_dir = sys.argv[1]
executions_number = int(sys.argv[2])

system("rm -f " + replicas_config_dir + "*.cfg")

port = 3000
replicas_number = 13

for i in range(executions_number):
  with open(replicas_config_dir + str(i) + ".cfg", "w") as file:
    for i in range(replicas_number):
      file.write(str(i) + " localhost " + str(port) + "\n")
      port += 1
