#!/usr/bin/env python

import sys
import pygal
from os import listdir
from os.path import isfile, join
from sets import Set

if(len(sys.argv) < 2):
  print "Usage: " + sys.argv[0] + " LOGS_DIR"
  sys.exit()

logs_dir = sys.argv[1]
logs_files = [f for f in listdir(logs_dir) if isfile(join(logs_dir, f))]

replicas = Set()
elements = Set()
replica_to_state = {}
time_to_bytes = {}

for log in logs_files:
  replica_id = log.split(".")[0]
  last_state = ""

  if replica_id == "":
    continue

  replicas.add(replica_id)

  with open(logs_dir + log, "r") as file:
    for line in file:
      line = line.rstrip("\n")
      parts = line.split("|")

      # in case we have "client disconnect" messages
      if len(parts) == 1: 
        continue

      time = parts[0]
      which = parts[1]

      if which == "O":
        # save all elements added
        # TODO support rmv
        op = parts[2]
        op_parts = op.split(" ")
        if op_parts[0] == "add":
          for i in range(1,len(op_parts)):
            elements.add(op_parts[i])

      elif which == "S":
        # save last state and the time of that last state for all replicas
        last_state_time = time
        last_state = parts[2]

      elif which == "B":
        # save bytes transfered 
        ack_or_delta = parts[2]
        bytes = parts[6]

        if time in time_to_bytes:
          time_to_bytes[time].append((ack_or_delta, bytes))
        else:
          time_to_bytes[time] = [(ack_or_delta, bytes)]
      else:
        print "unknown log type"

    last_state_parts = last_state.split(",")
    state = Set()
    for part in last_state_parts:
      if part != "":
        state.add(part)
    replica_to_state[replica_id] = (last_state_time, state)


def convergence_failed(): 
  print "convergence failed"
  sys.exit()

# draw the chart now
convergence_time = 0
time_zero = int(min(time_to_bytes.keys()))
time_to_delta_bytes = {}
time_to_ack_bytes = {}

for replica_id in replicas:
  if not replica_id in replica_to_state:
    convergence_failed()
  
  (time, state) = replica_to_state[replica_id]
  time = int(time)
  
  if state != elements:
    convergence_failed()

  if time > convergence_time:
    convergence_time = time

convergence_time -= time_zero

for key, value in time_to_bytes.iteritems():
  time = int(key) - time_zero
  delta_bytes = 0
  ack_bytes = 0

  for (type, bytes) in value:
    if type == 'D':
      delta_bytes += int(bytes)
    elif type == 'A':
      ack_bytes += int(bytes)
    else:
      print "unknown bytes type"

  time_to_delta_bytes[time] = delta_bytes
  time_to_ack_bytes[time] = ack_bytes

times = sorted(time_to_delta_bytes)

delta_bytes_list = []
ack_bytes_list = []
convergence_list = []

print times

for time in times:
  delta_bytes_list.append(time_to_delta_bytes[time])

  if time == 0:
    # litle ack: these acks in time zero are used to exchange ids between replicas
    ack_bytes_list.append(0)
  else:
    ack_bytes_list.append(time_to_ack_bytes[time])

  if time == convergence_time:
    convergence_list.append({"value": convergence_time, "node": {"r" : 8}})
  else:
    convergence_list.append(None)

title = "Gossip"
file = title + ".png"

chart = pygal.Line()
chart.title = title
chart.add("Acks", ack_bytes_list)
chart.add("Deltas",  delta_bytes_list)
#chart.add("Convergence", [{"value": convergence_list, "node": {"r": 8}}])
chart.add("Convergence", convergence_list)

chart.render_to_png(file)
chart.render_in_browser()



