#!/usr/bin/env python

import sys
import os

if (len(sys.argv) < 3):
  print "Usage: " + sys.argv[0] + " REPLICAS_CONFIG GRAPH_CONFIG"
  sys.exit()

os.system("rm -f cmds/*.cmds")

replicas_config = sys.argv[1]
graph_config = sys.argv[2]

replicas = {}
commands = {}

with open(replicas_config, "r") as file:
  for line in file:
    id,host,port = (i.rstrip("\n") for i in line.split(" "))
    replicas[id] = [host, port]
    commands[id] = []

with open(graph_config, "r") as file:
  for line in file:
    fst,snd = (i.rstrip("\n") for i in line.split(" "))
    sndConfig = replicas[snd]
    sndHost = sndConfig[0]
    sndPort = sndConfig[1]
    
    commands[fst].append("connect " + snd + ":" + sndHost + ":" + sndPort)


for replicaId, connects in commands.iteritems():
  print replicaId
  with open("cmds/" + replicaId + ".cmds", "w") as file:
    for connect in connects:
      file.write(connect + "\n")

