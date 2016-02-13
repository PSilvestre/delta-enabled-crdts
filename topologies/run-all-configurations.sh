#!/usr/bin/env python

import sys
from os import system

executions_per_config = 3
gossip_sleep_time = 5
rounds_of_updates = 3

log_dir = "log/"
cmd_dir = "cmd/"
replicas_config_dir = "replicas_config/"


# erdos-renyi
e_config = {}
e_config['tag'] = "e"
e_config['file'] = "topology_config/erdos_renyi.cfg"
e_config['fanouts'] = [-1, 1, 2, 3]

# ring
r_config = {}
r_config['tag'] = "r"
r_config['file'] = "topology_config/ring.cfg"
r_config['fanouts'] = [-1, 1]

# hyperview
h_config = {}
h_config['tag'] = "h"
h_config['file'] = "topology_config/hyperview3.cfg"
h_config['fanouts'] = [-1, 1, 2]

configs = [e_config, r_config, h_config]
current_execution = 0

for execution in range(executions_per_config):
  print "echo \"Execution " + str(execution) + "\""
  print "rm -f .PIDS"

  for config in configs:
    for fanout in config['fanouts']:
      for delta_or_full_state in ["d", "s"]:
        dirname = config['tag'] + "_f"
        if fanout != -1:
          dirname += str(fanout)

        dirname += "_" + delta_or_full_state + "_" + str(execution)
        replicas_config = replicas_config_dir + str(current_execution) + ".cfg"
        cmd = cmd_dir + dirname + "/"
        log = log_dir + dirname + "/"
        delta_flag = "-" + delta_or_full_state

        generate_command = "./generate-replicas-commands.sh {replicas_config} {topology_config} {cmd} {updates}".format(replicas_config = replicas_config, topology_config = config['file'], cmd = cmd, updates = rounds_of_updates)
        start_command = "./start-replicas.sh {replicas_config} {cmd} {log} {gossip} {fanout} {delta_flag} | bash".format(replicas_config = replicas_config, cmd = cmd, log = log, gossip = gossip_sleep_time, fanout = fanout, delta_flag = delta_flag)

        # create diretories if not present
        print "mkdir -p " + cmd + " " + log
        # clean those diretories 
        print "rm -f " + cmd + "* " + log + "*"

        print generate_command
        print start_command

        current_execution += 1
  
  print "sleep 9m"      
  print "./stop-replicas.sh"


print "echo \"I'm done!\""
