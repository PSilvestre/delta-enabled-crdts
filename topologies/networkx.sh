#!/usr/bin/env python

# Install:
# $ sudo pip install networkx
# $ sudo apt-get install python-matplotlib

import networkx as nx
import matplotlib.pyplot as plt
import sys
from os import system

def show(graph):
  print graph.edges()
  nx.draw(graph)
  plt.show()
 
def save(file_name, graph):
  with open(topologies_config_dir + file_name, "w") as file:
    for edge in graph.edges():
      file.write(str(edge[0]) + " " + str(edge[1]) + "\n")

def random_ring():
  nodes = 13
  ring = nx.cycle_graph(nodes)
  show(ring)
  save("ring.cfg", ring)

def random_tree():
  forks_on_each_node = 3
  height = 2
  tree = nx.balanced_tree(forks_on_each_node, height)
  show(tree)
  save("tree.cfg", tree)

def random_graph():
  nodes = 13
  neighbours = 2
  probability_of_rewiring_each_edge = 0.5 # ?
  graph = nx.connected_watts_strogatz_graph(nodes, neighbours, probability_of_rewiring_each_edge)
  show(graph)
  save("graph.cfg", graph)

if(len(sys.argv) < 2):
  print "Usage: " + sys.argv[0] + " TOPOLOGIES_CONFIG_DIR"
  sys.exit()

topologies_config_dir = sys.argv[1]
system("rm -f " + topologies_config_dir + "*.cfg")

random_ring()
random_tree()
random_graph()

