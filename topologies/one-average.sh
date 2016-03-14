#!/usr/bin/env python

import sys
import pygal
from os import listdir
from os.path import isfile, join
from sets import Set
import json

MAX_X = 1600
MAX_Y = 1000000
WIDTH = 1600

def get_title(analyse_key):
  labels = {}
  labels['e'] = "Erdos-Renyi"
  labels['r'] = "Ring"
  labels['h'] = "HyParView"
  labels['d'] = "With Deltas"
  labels['s'] = "Without Deltas"
  
  analyse_key_parts = analyse_key.split("_")
  title = labels[analyse_key_parts[0]] + " (" + labels[analyse_key_parts[2]] + ")"

  return title

def get_fanout_title(fanout):
  labels = {}
  labels['f1'] = "Fanout 1"
  labels['f2'] = "Fanout 2"
  labels['f'] = "Flooding"

  return labels[fanout]

def get_fanout(analyse_key):
  return analyse_key.split("_")[1]

def delta_or_not(analyse_key):
  analyse_key_parts = analyse_key.split("_")
  return analyse_key_parts[2] == "d"

# list of lists average
def lol_average(lol):
  result = []

  for t in range(len(lol[0])):
    sum = 0
    count = 0

    for c in range(len(lol)):
      if t < len(lol[c]):
        sum += lol[c][t]
        count += 1
      else:
        print "weird"

    average = sum / count
    result.append(average)

  return result

def closest_times(all_times, list):
  result = []

  for time in list:
    index = min(range(len(all_times)), key=lambda i: abs(all_times[i] - time))
    closest_time = all_times[index]
    result.append(closest_time)

  return result
  
def do_average(average_dir, analyse_key, analysis):
  if analyse_key == "":
    return

  all_times = []
  for i in range(0, MAX_X/5 + 1):
    all_times.append(i * 5)

  all_bytes = []
  all_convergences = []

  # process all analysis
  for analyse in analysis:
    bytes = {}
    convergence_list = []

    for i in range(len(analyse['times'])):
      time =  analyse['times'][i]
      deltas = analyse['deltas'][i]
      convergence = analyse['convergence'][i]

      bytes[time] = {'deltas' : deltas}
      if convergence:
        convergence_list.append(time)

    all_bytes.append(bytes)
    all_convergences.append(convergence_list)

  size = len(all_bytes)

  convergences = lol_average(all_convergences)
  convergences = closest_times(all_times, convergences)

  print convergences

  # do average of bytes
  deltas_list = []
  values_before = []

  for i in range(size):
    values_before.append({'deltas' : 0})

  for time in all_times:
    deltas_sum = 0

    for i in range(size):
      values = all_bytes[i]
      if time in values:
        this_deltas = values[time]['deltas']
      else:
        this_deltas = values_before[i]['deltas']

      deltas_sum += this_deltas
      values_before[i]['deltas'] = this_deltas

    deltas_list.append(deltas_sum / size)

  return (deltas_list, convergences)


if len(sys.argv) < 3:
  print "Usage: " + sys.argv[0] + " ANALYSIS_DIR AVERAGE_DIR"
  sys.exit()

analysis_dir = sys.argv[1]
average_dir = sys.argv[2]
analysis_files = [f for f in listdir(analysis_dir) if isfile(join(analysis_dir, f))]

analysis = []
last_key = ""

analysis_results = {}

for analyse_file in sorted(analysis_files):
  parts = analyse_file.split("_")

  # ignore invalid files
  if len(parts) < 3:
    continue

  analyse_key = parts[0] + "_" + parts[1] + "_" + parts[2]

  if analyse_key != last_key:
    if last_key != "":
      result = do_average(average_dir, last_key, analysis)
      analysis_results[last_key] = result
      if last_key == "r_f_d":
        r_f_d = result
    analysis = []
    last_key = analyse_key

  with open(analysis_dir + analyse_file) as input_file:
    analyse = json.load(input_file)
    analysis.append(analyse)


result = do_average(average_dir, last_key, analysis)
analysis_results[last_key] = result
r_f_s = result

all_times = []
for i in range(0, MAX_X/5 + 1):
  all_times.append(i * 5)

print all_times

three_graphics = {}

for key, (state_list, convergences) in analysis_results.iteritems():
  fanout = get_fanout(key)
  fanout_results = {'key': key, 'values': state_list, 'convergences': convergences}
  if fanout in three_graphics:
    three_graphics[fanout].append(fanout_results)
  else:
    three_graphics[fanout] = [fanout_results]

three_graphics["f2"].append({'key': 'r_f_d', 'values': r_f_d[0], 'convergences':r_f_d[1]})
three_graphics["f2"].append({'key': 'r_f_s', 'values': r_f_s[0], 'convergences':r_f_s[1]})

with open("charts/reads.json", "w") as output_file:
  json.dump(three_graphics, output_file)

for fanout, fanout_results in three_graphics.iteritems():
  print fanout
  print len(fanout_results)
  # draw bytes chart now
  chart = pygal.Line(x_label_rotation=90, range=(0, MAX_Y), width=WIDTH)
  chart.x_labels = all_times
  chart.title = get_fanout_title(fanout)
  for results in fanout_results:
    chart.add(get_title(results['key']), results['values'])
  chart.render_to_png(average_dir + fanout + ".png")

