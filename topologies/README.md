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
$ ./networkx.sh topology_config/
```

The above generates two files:
- __topology_config/ring.cfg__
- __topology_config/erdos_renyi.cfg__

In __topology_config/__ folder there's another configuration: __hyperview3.cfg__. But this one was not generated using __networkx__ (we used [this](http://blog.ayende.com.s3.amazonaws.com/HyParView/hyparview.html)).

__Usage:__
```bash
$ ./networkx.sh TOPOLOGIES_CONFIG_DIR
```

Each line of these files contains two ids. 

__Example:__

If "0 1" is on one of the lines, then, in that topology, replica 0 will be connected to replica 1.


### Generate replicas commands

```bash
$ chmod u+x generate-replicas-commands.sh
$ ./generate-replicas-commands.sh replicas.cfg topology_config/ring.cfg cmd/ 5
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

The argument __ROUNDS_OF_UPDATES__ is optional, and by default is 1. If bigger than 1, between each round of updates, the replicas will wait 180 seconds.

__Example:__

If "0 1" is on one of the lines of __topology_config/ring.cfg__, and __replicas.cfg__ has the line "1 localhost 3001", then the file __cmd/0.cmds__ will have the line "connect 1:localhost:3001".

If "1 2" is on one of the lines of __topology_config/ring.cfg__, and __replicas.cfg__ has the line "2 localhost 3002", then the file __cmd/1.cmds__ will have the line "connect 2:localhost:3002".


### Start replicas

Make sure you run `make replicas` in the root of the repository before the following commands.

```bash
$ chmod u+x start-replicas.sh
$ ./start-replicas.sh replicas.cfg cmd/ log/ 5 1 -d | bash && tail -f log/*
```

The above starts all replicas that have commands to be executed in __REPLICAS_COMMANDS_DIR__.

__Usage:__
```bash
$ ./start-replicas.sh REPLICAS_CONFIG REPLICAS_COMMANDS_DIR LOGS_DIR GOSSIP_SLEEP_TIME FANOUT DELTA_FLAG
```

The argument __GOSSIP_SLEEP_TIME__ indicates the number of seconds to wait before gossiping.

The argument __FANOUT__ indicates the fanout to be used by replicas. If __-1__ is passed, __flooding__ will be used.

__DELTA_FLAG__ can have two values:
- __-d__ and replicas will exchange deltas
- __-s__ and replicas will exchange its full state

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
$ ./analyse.sh log/ log.json
```

__Usage:__
```bash
$ ./analyse.sh LOGS_DIR OUTPUT_FILE_NAME
```

After generating the charts, we'll store in __OUTPUT_FILE_NAME__ as __JSON__ the values needed to draw the charts again without need to analyse the logs again. These values, also allows us to calculate the average of several executions.

# Several executions

### Generate replicas config

You can generate several config files, and use one for each execution. Imagine there will be 540 executions:

```bash
$ chmod u+x generate-replicas-config.sh
$ ./generate-replicas-config.sh replicas_config/ 540
```

__Usage:__
```bash
$ ./generate-replicas-config.sh REPLICAS_CONFIG_DIR EXECUTIONS_NUMBER 
``` 

### Run all configurations

```bash
$ chmod u+x run-all-configurations.sh
$ ./run-all-configurations.sh
```

