# Topologies

### Network

```bash
$ chmod u+x networkx.sh
$ ./networkx.sh config/
```

The above generates three files:
- __config/ring.cfg__
- __config/tree.cfg__
- __config/graph.cfg__

Each line of these files contains two ids. 

__Usage:__
```bash
$ ./networkx.sh TOPOLOGIES_CONFIG_DIR
```

__Example:__

If "0 1" is on one of the lines, then, in that topology, replica 0 will be connected to replica 1.


### Generate replicas commands

```bash
$ chmod u+x process_graph_config.sh
$ ./process_graph_config.sh replicas.cfg config/ring.cfg cmd/ 
```

The above generates some files:
- __cmd/0.cmds__
- __cmd/1.cmds__
- __cmd/2.cmds__
- ...

__Usage:__
```bash
$ ./process_graph_config.sh REPLICAS_CONFIG TOPOLOGY_CONFIG REPLICAS_COMMANDS_DIR 
```

__Example:__

If "0 1" is on one of the lines of __config/ring.cfg__, and __replicas.cfg__ has the line "1 localhost 3001", then the file __cmd/0.cmds__ will have the line "connect 1:localhost:3001".

If "1 2" is on one of the lines of __config/ring.cfg__, and __replicas.cfg__ has the line "2 localhost 3002", then the file __cmd/1.cmds__ will have the line "connect 2:localhost:3002".


### Start replicas

Make sure you run `make replicas` in the root of the repository before the following commands.

```bash
$ chmod u+x start-replicas.sh
$ ./start-replicas.sh replicas.cfg cmd/ log/ | bash && tail -f log/*
```

The above starts all replicas that have commands to be executed in __REPLICAS_COMMANDS_DIR__.

__Usage:__
```bash
$ ./start-replicas.sh REPLICAS_CONFIG REPLICAS_COMMANDS_DIR LOGS_DIR
```

