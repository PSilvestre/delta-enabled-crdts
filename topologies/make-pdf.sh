#!/usr/bin/env python

import sys
from os import system
from os import listdir
from os.path import isfile, join
import re

if len(sys.argv) < 3:
  print "Usage: " + sys.argv[0] + " CHARTS_DIR OUTPUT_FILENAME"
  sys.exit()

charts_dir = sys.argv[1]
out_file = sys.argv[2]
charts_files = [f for f in listdir(charts_dir) if isfile(join(charts_dir, f))]

bytes = {}
bytes["delta"] = {}
bytes["state"] = {}
bytes["delta"]["e"] = []
bytes["delta"]["r"] = []
bytes["delta"]["h"] = []
bytes["state"]["e"] = []
bytes["state"]["r"] = []
bytes["state"]["h"] = []

load = {}
load["delta"] = {}
load["state"] = {}
load["delta"]["e"] = []
load["delta"]["r"] = []
load["delta"]["h"] = []
load["state"]["e"] = []
load["state"]["r"] = []
load["state"]["h"] = []

def load_or_not(key_zero, key_one, file):
  if re.search("load", file):
    load[key_zero][key_one].append(file)
  else:
    bytes[key_zero][key_one].append(file)

for file in sorted(charts_files):
  if re.search("_d", file):
    if re.search("^e_", file):
      load_or_not("delta", "e", file)
    elif re.search("^r_", file):
      load_or_not("delta", "r", file)
    elif re.search("^h_", file):
      load_or_not("delta", "h", file)
  elif re.search("_s", file):
    if re.search("^e_", file):
      load_or_not("state", "e", file)
    elif re.search("^r_", file):
      load_or_not("state", "r", file)
    elif re.search("^h_", file):
      load_or_not("state", "h", file)


def start(output):
  output.write("\\documentclass[a4paper,11pt]{report}\n")
  output.write("\\usepackage[top=.6in, bottom=.6in, left=.3in, right=.3in]{geometry}\n")
  output.write("\\usepackage{graphicx}\n")
  output.write("\\begin{document}\n")
  output.write("\\begin{center}\n")
 
def end(output):
  output.write("\\end{center}\n")
  output.write("\\end{document}\n")

def bytes_chart(file):
  return "\\includegraphics[width=\\textwidth]{" + charts_dir + file + "}\n"
 
def load_chart(file):
  return "\\includegraphics[width=.6\\textwidth]{" + charts_dir + file + "}\n"

def new_page():
  return "\\newpage\n"

with open(out_file + ".tex", "w") as output:
  start(output)

  for delta_or_state, values in bytes.iteritems():
    for topology, charts in values.iteritems():
      for chart in charts:
        output.write(bytes_chart(chart))
      output.write(new_page())

  for delta_or_state, values in load.iteritems():
    for topology, charts in values.iteritems():
      for chart in charts:
        output.write(load_chart(chart))
      output.write(new_page())

  end(output)

      
system("pdflatex " + out_file + ".tex")
system("rm -f *.aux *.tex *.log")
system("evince " + out_file + ".pdf")

