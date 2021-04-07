# Message exchange service

![Build status](https://github.com/f0m41h4u7/message-broker/actions/workflows/ci.yaml/badge.svg)

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
{"method":"CONNECT","queue":"queue_name"}
```
* Write data to queue:
```
{"method":"PUBLISH","data":"some data to publish"}
```
* Read from queue:
```
{"method":"CONSUME"}
```
