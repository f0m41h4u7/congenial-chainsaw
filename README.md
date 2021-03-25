# Message exchange service

### Build
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Default server port is `31337`. 

## Protocol
```
0           4                body_size
+-----------+--------------------+
| body_size |        body        |
+-----------+--------------------+
```
Body is json message with one of the methods:

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
{"method":"CONSUME"}
```
