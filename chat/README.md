# Unix Sockets

Make sure you're in the root of the repository. Then:

```bash
$ make chat
```

With (at least) two terminals open:

- in the __first__ one:

```bash
$ ./server 3333
```

- in the __others__:

```bash
$ ./client localhost 3333
```

Now you can chat!

