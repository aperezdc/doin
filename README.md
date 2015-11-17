doin
====

Executes *any* binary in *any* namespace-based Linux container.
Without platform-dependant kludges.


Usage
=====

```sh
# Build doin.so (does not need root)
make

# Now as "root":
PID=$(docker inspect --format='{{.State.Pid}}' my-minimal-container)
./doin $PID tree /
```

The latest line above is roughly equivalent to:

```sh
LD_PRELOAD=$(pwd)/doin.so __DOIN_ATTACH_PID=${PID} tree /
```

doin vs. …
==========

Infilter
--------

[Infilter](https://github.com/yadutaf/infilter) uses the
[ptrace()](http://linux.die.net/man/2/ptrace) system call to be notified when
the program makes it first system calls, and modify CPU registers directly
to inject calls to the `setns()` system call. On the other hand, `doin` works
independently of the platform, as it does not access hardware features
directly.


How does it work?
=================

The `doin.so` shared object is loaded by the dynamic linker before the rest of
shared objects needed by the binary being executed — including the C library.
The C library provides the
[__libc_start_main](http://refspecs.linuxbase.org/LSB_3.1.1/LSB-Core-generic/LSB-Core-generic/baselib---libc-start-main-.html),
which is called by the `_start` function of every program (which is where
execution really starts, this code resides in `crt1.o` and is linked in every
program) which `doin.so` overrides to arrange calls to
[setns()](http://linux.die.net/man/2/setns) *before* calling the original
`__libc_start_main` provided by the C library, which in turn will call the
`main()` function in the program.


License
=======

[MIT](http://opensource.org/licenses/mit)

