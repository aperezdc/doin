doin
====

Executes *any* binary in *any* namespace-based Linux container.
Without platform-dependant kludges.


Usage
=====

```sh
make
PID=$(docker inspect --format='{{.State.Pid}}' my-minimal-container)
LD_PRELOAD=$(pwd)/doin.so __DOIN_ATTACH_PID=${PID} tree /
```


License
=======

[MIT](http://opensource.org/licenses/mit)

