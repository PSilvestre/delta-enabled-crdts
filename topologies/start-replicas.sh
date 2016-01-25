#!/usr/bin/env python

import sys
from os import system
from sets import Set
from os import listdir
from os.path import isfile, join

if (len(sys.argv) < 4):
  print "Usage: " + sys.argv[0] + " REPLICAS_CONFIG REPLICAS_COMMANDS_DIR LOGS_DIR"
  sys.exit()

replicas_config = sys.argv[1]
replicas_commands_dir = sys.argv[2]
logs_dir = sys.argv[3]

config = {}
commands_files = [f for f in listdir(replicas_commands_dir) if isfile(join(replicas_commands_dir, f))]

with open(replicas_config, "r") as file:
  for line in file:
    id,host,port = (i.rstrip("\n") for i in line.split(" "))
    config[id] = [host, port]

print "rm .PIDS"
print "rm log/*.log"

for commands in commands_files:
  replica_id = commands.split(".")[0]
  if (replica_id == ''):
    continue
  replica_port = config[replica_id][1]

  start_command = "./../replica " + replica_id + ":" + replica_port + " 2>&1 > " + logs_dir + replica_id + ".log < " + replicas_commands_dir + commands + " &"
  print start_command
  print "echo $! >> .PIDS"


