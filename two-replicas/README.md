# Two replicas

Make sure you're in the root of the repository. Then:

```bash
$ make replicas
```

With two terminals open:

- in the __first__ one:

```bash
$ ./replica 3333 4444
```

- in the __second__:

```bash
$ ./replica 4444 3333
```

After starting the first replica, you have 5 seconds to start the second one.

You'll eventually see the following message:
```bash
Usage:
add [elems]
rmv [elems]
show
```

Then you can start updating your replicas.

Example:

- __first__ replica:

```bash
add a
add b
show
2PSet: S( a b ) T ( )
rmv a
show
2PSet: S( b ) T ( a )
```

- __second__ replica:

```bash
show 
2PSet: S( b ) T ( a )
```
