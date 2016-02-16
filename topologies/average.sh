#!/usr/bin/env python

import sys
import pygal
from os import listdir
from os.path import isfile, join
from sets import Set
import json

MAX_Y = {'delta' : 30000, 'state' : 100000}
WIDTH = 1600

def get_title(analyse_key):
  labels = {}
  labels['e'] = "Erdos-Renyi"
  labels['r'] = "Ring"
  labels['h'] = "HyParView"
  labels['f1'] = "Fanout 1"
  labels['f2'] = "Fanout 2"
  labels['f3'] = "Fanout 3"
  labels['f'] = "Flooding"
  labels['d'] = "With Deltas"
  labels['s'] = "Without Deltas"
  
  analyse_key_parts = analyse_key.split("_")
  title = labels[analyse_key_parts[0]] + " (" + labels[analyse_key_parts[2]] + ") - " + labels[analyse_key_parts[1]]

  return title

def delta_or_not(analyse_key):
  analyse_key_parts = analyse_key.split("_")
  return analyse_key_parts[2] == "d"

# list of lists average
def lol_average(lol):
  result = []

  for t in range(len(lol[0])):
    sum = 0
    for c in range(len(lol)):
      sum += lol[c][t]

    average = sum / len(lol)
    result.append(average)

  return result

def closest_times(all_times, list):
  result = []

  for time in list:
    index = min(range(len(all_times)), key=lambda i: abs(all_times[i] - time))
    closest_time = all_times[index]
    result.append(closest_time)

  return result
  
def which_range(mins, load):
  for min in mins[::-1]:
    if load >= min:
      return min

def nice_min(list):
  return min(list)

def do_average(average_dir, analyse_key, analysis):
  if analyse_key == "":
    return

  all_times = Set()
  all_bytes = []
  all_convergences = []
  all_updates = []
  all_loads = []

  # process all analysis
  for analyse in analysis:
    convergence_list = []
    updates_list = []
    bytes = {}
    for i in range(len(analyse['times'])):
      time =  analyse['times'][i]
      deltas = analyse['deltas'][i]
      acks = analyse['acks'][i]
      convergence = analyse['convergence'][i]
      updates = analyse['updates'][i]

      if convergence:
        convergence_list.append(time)

      if updates:
        updates_list.append(time)

      bytes[time] = {'deltas' : deltas, 'acks' : acks}

    all_times.update(analyse['times'])
    all_bytes.append(bytes)
    all_convergences.append(convergence_list)
    all_updates.append(updates_list)
    all_loads.extend(analyse['load'])

  # sort times
  size = len(all_bytes)
  all_times = sorted(all_times)

  # do average of convergence and updates
  convergences = lol_average(all_convergences)
  updates = lol_average(all_updates)

  # find the closest value in all_times for convergences and updates
  convergences = closest_times(all_times, convergences)
  updates = closest_times(all_times, updates)

  # do average of bytes
  acks_list = []
  deltas_list = []
  convergence_list = []
  updates_list = []

  values_before = []
  for i in range(size):
    values_before.append({'deltas' : 0, 'acks' : 0})

  for time in all_times:
    deltas_sum = 0
    acks_sum = 0

    for i in range(size):
      values = all_bytes[i]
      if time in values:
        this_deltas = values[time]['deltas']
        this_acks = values[time]['acks']
      else:
        this_deltas = values_before[i]['deltas']
        this_acks = values_before[i]['acks']

      deltas_sum += this_deltas
      acks_sum += this_acks
      values_before[i]['deltas'] = this_deltas
      values_before[i]['acks'] = this_acks

    acks_list.append(acks_sum / size)
    deltas_list.append(deltas_sum / size)

    if time in convergences:
      convergence_list.append({"value": 0, "node": {"r" : 8}})
    else:
      convergence_list.append(None)

    if time in updates:
      updates_list.append({"value": 0, "node": {"r" : 6}})
    else:
      updates_list.append(None)


  # draw bytes chart now
  max_y = MAX_Y['delta'] if delta_or_not(analyse_key) else MAX_Y['state']
  chart = pygal.Line(x_label_rotation=90, range=(0, max_y), width=WIDTH)
  chart.x_labels = all_times
  chart.title = get_title(analyse_key)
  chart.add("State", deltas_list)
  chart.add("Acks", acks_list)
  chart.add("Convergence", convergence_list)
  chart.add("Update", updates_list)
  chart.render_to_png(average_dir + analyse_key + ".png")

  # process load
  min_load = nice_min(all_loads)
  max_load = max(all_loads)

  mins = []
  while (min_load < max_load):
    mins.append(min_load)
    min_load += 3

  loads_count = len(all_loads)
  min_range_to_count = {}

  for load in all_loads:
    min_range = which_range(mins, load)

    if min_range in min_range_to_count:
      min_range_to_count[min_range] += 1
    else:
      min_range_to_count[min_range] = 1

  labels_values_list = []
  
  for min in mins:
    label = str(min) + " - " + str(min + 2) + " msgs"
    
    count = 0
    if min in min_range_to_count:
      count = min_range_to_count[min]

    if count > 0:
      labels_values_list.append((label, 100 * count / float(loads_count)))
    else:
      labels_values_list.append((label, count))

  # draw load chart now 
  l_chart = pygal.Bar()
  l_chart.title = "Load distribution: " + get_title(analyse_key)
  for (label, count) in labels_values_list:
    l_chart.add(label, count)
  l_chart.render_to_png(average_dir + analyse_key + "_load.png")
  
if len(sys.argv) < 3:
  print "Usage: " + sys.argv[0] + " ANALYSIS_DIR AVERAGE_DIR"
  sys.exit()

analysis_dir = sys.argv[1]
average_dir = sys.argv[2]
analysis_files = [f for f in listdir(analysis_dir) if isfile(join(analysis_dir, f))]

analysis = []
last_key = ""

for analyse_file in sorted(analysis_files):
  parts = analyse_file.split("_")

  # ignore invalid files
  if len(parts) < 3:
    continue

  analyse_key = parts[0] + "_" + parts[1] + "_" + parts[2]

  if analyse_key != last_key:
    if last_key != "":
      do_average(average_dir, last_key, analysis)
    analysis = []
    last_key = analyse_key

  with open(analysis_dir + analyse_file) as input_file:
    analyse = json.load(input_file)
    analysis.append(analyse)


do_average(average_dir, last_key, analysis)
