# Socket Hello World

This is a sample application to demonstrate client/server using socket programming. The client and server send some messages when the connection establish. It has a capability to send message using TCP or UDP protocol.

## Compile for host

This program might run on different architecture. For example, the host is x86_64 architecture while the target board is RISC-V architecture. You need a gcc for x86_64 to compile it for the host. 

```
cd package/socket_hello_world/src
make
```

## Usage

```
help
-a        server IP address
-c        client mode
-m        message to send to server
-s        server mode
-p        port number. Default is 9123
-u        use UDP packet instead of TCP packet
-h        show this help message

Example usage
server,
         ./socket_hello_world -s
client,
         ./socket_hello_world -c -a <server ip>
```

Example,

Send packet using UDP protocol. By default, it use port `9123` for both client and server.

```
server,
./socket_hello_world -s -u

client,
./socket_hello_world -c -a <server ip> -u
```

Output on the server terminal

```
root@buildroot:~# ./socket_hello_world -s -u
port = 9123
protocol: UDP
msg = hello from client
mode = server
client: hello from client
server: UDP message sent
```

Output on the client terminal

```
 ./socket_hello_world -c -a 192.168.31.4 -u
port = 9123
protocol: UDP
msg = hello from client
mode = client
ipaddr = 192.168.31.4
server: Hello from server
```
