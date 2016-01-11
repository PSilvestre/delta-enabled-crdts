#!/usr/bin/env python

# Install:
# $ sudo pip install networkx
# $ sudo apt-get install python-matplotlib

import networkx as nx
import matplotlib.pyplot as plt

def show(graph):
  print graph.edges()
  nx.draw(graph)
  plt.show()
 
def save(file_name, graph):
  file = open("config/" + file_name, "w")
  for edge in graph.edges():
    file.write(str(edge[0]) + " " + str(edge[1]) + "\n")
  file.close()

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


random_ring()
random_tree()
random_graph()

