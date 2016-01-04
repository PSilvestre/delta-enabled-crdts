# Replicas

Make sure you're in the root of the repository. Then:

```bash
$ make replicas
```

__Usage:__ 
```bash
$ ./replica unique_id:port
```

## Example with two replicas

With two terminals open:

- in the __first__ one:

```bash
$ ./replica 1:3001
```

- in the __second__:

```bash
$ ./replica 2:3002
```

You'll see the following message:
```bash
Usage:
add [elems]
rmv [elems]
connect [unique_id:port]
show
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
connect 1:3001
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

