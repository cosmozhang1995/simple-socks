#!/usr/bin/env python3

import os
import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((os.environ.get('SOCKS_TEST_SERVER', 'localhost'), 1080))
sock.sendall(b'abc')
recvdata = b''
last_recv_time = time.time()
while True:
    try:
        time.sleep(0.01)
        readdata = sock.recv(1024, socket.MSG_DONTWAIT)
        if len(readdata) != 0:
            last_recv_time = time.time()
        recvdata += readdata
    except BlockingIOError:
        pass
    if time.time() - last_recv_time > 1:
        break
print(recvdata.decode('ascii'))
sock.close()
