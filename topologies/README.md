# Topologies

### Network

```bash
$ chmod u+x networkx.sh
$ ./networkx.sh
```

This generates three files:
- __network/ring.cfg__
- __network/tree.cfg__
- __network/graph.cfg__


Each line of these files contains two ids. 

__Example:__

If "0 1" is on one of the lines, then, in that topology, replica 0 will be connected to replica 1.


### Connects

```bash
$ chmod u+x process_graph_config.sh
$ ./process_graph_config.sh replicas.cfg network/ring.cfg 
```

This generates some files:
- __cmds/0.cmds__
- __cmds/1.cmds__
- __cmds/2.cmds__
- ...

__Usage:__
```bash
$ ./process_graph_config.sh REPLICAS_CONFIG GRAPH_CONFIG
```

__Example:__

If "0 1" is on one of the lines of __network/ring.cfg__, and __replicas.cfg__ has the line "1 localhost 3001", then the file __cmds/0.cmds__ will have the line "connect 1:localhost:3001".

If "1 2" is on one of the lines of __network/ring.cfg__, and __replicas.cfg__ has the line "2 localhost 3002", then the file __cmds/1.cmds__ will have the line "connect 2:localhost:3002".

