#!/usr/bin/env python

import sys
from os import system

if (len(sys.argv) < 4):
  print "Usage: " + sys.argv[0] + " REPLICAS_CONFIG TOPOLOGY_CONFIG REPLICAS_COMMANDS_DIR"
  sys.exit()

replicas_config = sys.argv[1]
topology_config = sys.argv[2]
replicas_commands_dir = sys.argv[3]
system("rm -f " + replicas_commands_dir + "*.cmds")

replicas = {}
commands = {}

with open(replicas_config, "r") as file:
  for line in file:
    id,host,port = (i.rstrip("\n") for i in line.split(" "))
    replicas[id] = [host, port]
    commands[id] = []

with open(topology_config, "r") as file:
  for line in file:
    fst,snd = (i.rstrip("\n") for i in line.split(" "))
    sndConfig = replicas[snd]
    sndHost = sndConfig[0]
    sndPort = sndConfig[1]
    commands[fst].append("connect " + snd + ":" + sndHost + ":" + sndPort)


for replicaId, connects in commands.iteritems():
  with open(replicas_commands_dir + replicaId + ".cmds", "w") as file:
    for connect in connects:
      file.write(connect + "\n")

