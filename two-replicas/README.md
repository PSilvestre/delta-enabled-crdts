# Two replicas

Make sure you're in the root of the repository. Then:

```bash
$ make replicas
```

With two terminals open:

- in the first one:

```bash
$ ./replica 3333 4444
```

- in the second:

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
