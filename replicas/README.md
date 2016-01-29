# Replicas

Make sure you're in the root of the repository. Then:

```bash
$ make replicas
```

__Usage:__ 
```bash
$ ./replica unique_id:port [-r] [-t gossip_sleep_time] [-f fanout] [-d] [-s]
```

If __-r__ is passed as argument, __replica__ will be ran on "REPL mode".

If __-t__ is passed, it's expected an additional argument: __gossip_sleep_time__. This is the number of seconds between each gossip. By default is __10__ seconds.

If __-f__ is passed, it's expected an additional argument: __fanout__. By default is __1__. If __-1__, then it will gossip to all neighbours - __flooding__.

If __-d__ is passed, the replicas will disseminate deltas. This is the default.

If __-s__ is passed, the replicas will disseminate the whole state.


## Example with two replicas

With two terminals open:

- in the __first__ one:

```bash
$ ./replica 1:3001 -r
```

- in the __second__:

```bash
$ ./replica 2:3002 -r
```

You'll see the following message:
```bash
Usage:
add [elems]
rmv [elems]
show
connect [unique_id:host:port]
wait seconds
```

Then you can start updating your replicas.

Example:

- on the __first__ replica:

```bash
add a b
2PSet: S( a b ) T ( )
rmv b
2PSet: S( a ) T ( b )
```

- on the __second__ replica:

```bash
connect 1:localhost:3001
```

Eventually you'll see this on the __second__ replica:
```bash
2PSet: S( a ) T ( b )
```

- on the __second__ replica:
```bash
rmv a
2PSet: S( ) T ( a b )
```

Eventually you'll see this on the __first__ replica:
```bash
2PSet: S( ) T ( a b )
```

