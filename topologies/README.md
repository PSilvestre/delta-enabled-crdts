# Topologies

### Network

In order to generate these random graphs, you need to install the following dependencies.

```bash
$ sudo pip install networkx
$ sudo apt-get install python-matplotlib
```

After that, you can:

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
$ chmod u+x generate-replicas-commands.sh
$ ./generate-replicas-commands.sh replicas.cfg config/ring.cfg cmd/ 5
```

The above generates some files:
- __cmd/0.cmds__
- __cmd/1.cmds__
- __cmd/2.cmds__
- ...

__Usage:__
```bash
$ ./generate-replicas-commands.sh REPLICAS_CONFIG TOPOLOGY_CONFIG REPLICAS_COMMANDS_DIR [ROUNDS_OF_UPDATES]
```

The argument __ROUNDS_OF_UPDATES__ is optional, and by default is 1. If bigger than 1, between each round of updates, the replicas will wait 120 seconds.

__Example:__

If "0 1" is on one of the lines of __config/ring.cfg__, and __replicas.cfg__ has the line "1 localhost 3001", then the file __cmd/0.cmds__ will have the line "connect 1:localhost:3001".

If "1 2" is on one of the lines of __config/ring.cfg__, and __replicas.cfg__ has the line "2 localhost 3002", then the file __cmd/1.cmds__ will have the line "connect 2:localhost:3002".


### Start replicas

Make sure you run `make replicas` in the root of the repository before the following commands.

```bash
$ chmod u+x start-replicas.sh
$ ./start-replicas.sh replicas.cfg cmd/ log/ 0 | bash && tail -f log/*
```

The above starts all replicas that have commands to be executed in __REPLICAS_COMMANDS_DIR__.

__Usage:__
```bash
$ ./start-replicas.sh REPLICAS_CONFIG REPLICAS_COMMANDS_DIR LOGS_DIR EXECUTION_NUMBER [GOSSIP_SLEEP_TIME]
```

The argument __EXECUTION_NUMBER__ can be used to run an experiment several times. Then, the script __analyse.sh__, if it founds more than one execution, it will calculate the average of all executions.

The argument __GOSSIP_SLEEP_TIME__ is optional and by default is __10__ seconds.

### Stop replicas

```bash
$ chmod u+x stop-replicas.sh
$ ./stop-replicas.sh
```


# Charts

In order to generate charts after running the tests, you need to install the following dependencies.

```bash
$ sudo apt-get install python-dev libffi-dev
$ sudo pip install pygal cairosvg lxml tinycss cssselect
```

After that, you can:

```bash
$ chmod u+x analyse.sh
$ ./analyse.sh log/
```

__Usage:__
```bash
$ ./analyse.sh LOGS_DIR
```



