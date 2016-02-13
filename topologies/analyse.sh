#!/usr/bin/env python

# Install:
# $ sudo pip install pygal cairosvg lxml tinycss cssselect

import sys
import pygal
from os import listdir
from os.path import isfile, join
from sets import Set

if len(sys.argv) < 3:
  print "Usage: " + sys.argv[0] + " LOGS_DIR EXECUTION_NUMBER"
  sys.exit()

logs_dir = sys.argv[1]
execution_number = sys.argv[2]
logs_files = [f for f in listdir(logs_dir) if isfile(join(logs_dir, f))]

replicas = Set()
replica_to_load = {}
time_to_replica_and_state = {}
time_to_bytes = {}
time_with_adds = Set()

for log in logs_files:
  replica_id_and_execution_number = log.split(".")[0]
  replica_id = replica_id_and_execution_number.split("_")[0]
  log_execution_number = replica_id_and_execution_number.split("_")[1]
  last_state = ""
  load = 0

  # ignore invalid files and logs from other executions
  if replica_id == "" or log_execution_number != execution_number:
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
        # save times with rounds of updates
        op_parts = parts[2].split(" ")
        if op_parts[0] == "add":
          time_with_adds.add(time)

      elif which == "S":
        # save the replica and state for all times to detect convergence
        state_log = parts[2]
        state_log_parts = state_log.split(",")
        state = Set()

        for part in state_log_parts:
          if part != "":
            state.add(part)

        replica_and_state = (replica_id, state)

        if time in time_to_replica_and_state:
          time_to_replica_and_state[time].append(replica_and_state)
        else:
          time_to_replica_and_state[time] = [replica_and_state]

      elif which == "B":
        # save bytes transfered 
        ack_or_delta = parts[2]
        bytes = parts[6]
        aod_and_bytes = (ack_or_delta, bytes)

        if time in time_to_bytes:
          time_to_bytes[time].append(aod_and_bytes)
        else:
          time_to_bytes[time] = [aod_and_bytes]

      elif which == "L":
        # load distribution
        load += 1

      else:
        print "unknown log type"

    replica_to_load[int(replica_id)] = load


# process the info from the logs:

# function that, given a collection on timestamps and the time zero,
# returns a set of times truncated to time_zero
def subtract_time_zero(times, time_zero):
  new_times = Set()

  for time in times:
    new_time = int(time) - time_zero
    new_times.add(new_time)

  return new_times

# get all times and the time_zero
all_times = Set()
all_times.update(time_to_replica_and_state.keys())
all_times.update(time_to_bytes.keys())
all_times.update(time_with_adds)

time_zero = int(min(all_times))
all_times = subtract_time_zero(all_times, time_zero)
all_times = sorted(all_times)

# process time_to_replica_and_state
time_with_convergence = Set()
#for key, value in time_to_replica_and_state.iteritems():


# process time_to_bytes
time_to_delta_bytes = {}
time_to_ack_bytes = {}
time_to_id_bytes = {}

for key, value in time_to_bytes.iteritems():
  time = int(key) - time_zero
  delta_bytes = 0
  ack_bytes = 0
  id_bytes = 0

  for (type, bytes) in value:
    if type == 'D':
      delta_bytes += int(bytes)
    elif type == 'A':
      ack_bytes += int(bytes)
    elif type == 'I':
      id_bytes += int(bytes)
    else:
      print "unknown bytes type: " + type

  time_to_delta_bytes[time] = delta_bytes
  time_to_ack_bytes[time] = ack_bytes
  time_to_id_bytes[time] = id_bytes


# process time_with_adds
time_with_adds = subtract_time_zero(time_with_adds, time_zero)

# Compile all info in lists
delta_bytes_list = []
ack_bytes_list = []
id_bytes_list = []
convergence_list = []
updates_list = []

delta_bytes_sum = 0
ack_bytes_sum = 0
id_bytes_sum = 0

for time in all_times:
  delta_bytes = 0
  ack_bytes = 0
  id_bytes = 0

  if time in time_to_delta_bytes:
    delta_bytes = time_to_delta_bytes[time]

  if time in time_to_ack_bytes:
    ack_bytes = time_to_ack_bytes[time]

  if time in time_to_id_bytes:
    id_bytes = time_to_id_bytes[time]

  delta_bytes_sum += delta_bytes
  ack_bytes_sum += ack_bytes
  id_bytes_sum += id_bytes

  delta_bytes_list.append(delta_bytes_sum)
  ack_bytes_list.append(ack_bytes_sum)
  id_bytes_list.append(id_bytes_sum)

  if time in time_with_convergence:
    convergence_list.append({"value": 0, "node": {"r" : 8}})
  else:
    convergence_list.append(None)

  if time in time_with_adds:
    updates_list.append({"value": 0, "node": {"r" : 6}})
  else:
    updates_list.append(None)


# draw the chart now
title = "Hyperview"
subtitle = "(with deltas)"
#subtitle = "(without deltas)"
title_and_subtitle = title + " " + subtitle

filename = title.replace(" ", "_") + subtitle.replace("(", "_").replace(")", "_").replace(" ", "_") # replace this with a regex

chart = pygal.Line(x_label_rotation=90)
#chart = pygal.Line(range=(0, 9000))
chart.x_labels = all_times
chart.title = title_and_subtitle
chart.add("State",  delta_bytes_list)
chart.add("Acks", ack_bytes_list)
#chart.add("IDs", id_bytes_list)
chart.add("Convergence", convergence_list)
chart.add("Update", updates_list)
chart.render_to_png(filename + ".png")
chart.render_in_browser()

l_chart = pygal.HorizontalBar()
l_chart.title = "Load distribution - " + title_and_subtitle
for key, value in replica_to_load.iteritems():
  l_chart.add("replica " + str(key), value)
l_chart.render_to_png(filename + "load.png")
l_chart.render_in_browser()
