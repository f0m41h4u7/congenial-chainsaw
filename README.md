# Message exchange service

### Build
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Commands
* Connect to queue:
```
{"method":"CONNECT", "queue":"queue_name"}
```
* Write data to queue:
```
{"method":"PUBLISH", "data":"some data to publish"}
```
* Read from queue:
```
{"method":"CONSUME", "queue":"queue_name"}
```
