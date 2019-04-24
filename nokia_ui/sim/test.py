#!/usr/bin/env python3

import socket
import sys
import time
import struct
from threading import Thread

def recv_thread(s):
    while True:
        data = s.recv(4096)
        print('data:', data.hex())
        if not data: break

def main():
    s = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)

    try:
        s.connect("/tmp/simcard")
    except socket.error as msg:
        print(msg, file=sys.stderr)
        sys.exit(1)

    thread = Thread(target = recv_thread, args = (s, ))
    thread.daemon = True
    thread.start()

    #a = b"ABCDE"
    #b = b"XYZ"

    def getlen(bstr):
        return struct.pack("<H", len(bstr))

    #s.sendall(getlen(a) + a)
    #time.sleep(2)
    #s.sendall(getlen(a) + a)

    msgs = [
        #b"\xa0\xa4\x01\x02",                                #Case1
        #b"\xa0\xa4\x02\x03\x17",                            #Case2
        #b"\xa0\xa4\x02\x03\x23" + b"\xAA"*0x22 + b"\xAB",   #Case3
        #b"\xa0\xa4\x02\x03\x23" + b"\xAA"*0x22 + b"\xAB" + b"\x55",   #Case4

        #b"\xa0\xa4\x00\x00\x02\x3f\x00"
        #b"\xa0\xb0\x00\x00\xff"

        # Verify CHV1
        #b"\xa0\x20\x00\x01\x08" + b"1235" + b"\xFF\xFF\xFF\xFF"

        # Change CHV1
        #b"\xa0\x24\x00\x01\x10" + b"1234" + b"\xFF\xFF\xFF\xFF" + b"1235" + b"\xFF\xFF\xFF\xFF"

        # Disable CHV1
        #b"\xa0\x26\x00\x01\x08" + b"1234" + b"\xFF\xFF\xFF\xFF"

        # Enable CHV1
        #b"\xa0\x28\x00\x01\x08" + b"1234" + b"\xFF\xFF\xFF\xFF"

        # Unblock CHV1
        #b"\xa0\x2C\x00\x01\x10" + b"12345678" + b"1234" + b"\xFF\xFF\xFF\xFF"

        # Select MF
        b"\xa0\xa4\x00\x00\x02\x3F\x00",
        # # GET RESPONSE
        # b"\xa0\xc0\x00\x00\x17",

        # # Select IMSI
        # b"\xa0\xa4\x00\x00\x02\x6F\x07",
        # # GET RESPONSE
        # b"\xa0\xc0\x00\x00\x0F",

        # # Select LOCI
        # #b"\xa0\xa4\x00\x00\x02\x6F\x7E",


        # # GSM Algo
        # b"\xa0\x88\x00\x00\x10\x11\x22\x33\x44\x55\x66\x77\x88\x99\x0a\x0b\x0c\x0d\x0e\x0f\xff",
        # # GET RESPONSE
        # b"\xa0\xc0\x00\x00\x0F",


        # # Select ICCID
        # b"\xa0\xa4\x00\x00\x02\x2F\xE2",

        # # Read data
        # #b"\xa0\xb0\x00\x00\x0a"

        # # Write data
        # b"\xa0\xd6\x00\x07\x03\xAA\x55\x13"



        # # Select test linear file
        # b"\xa0\xa4\x00\x00\x02\x30\x39",

        # # Read current record
        # b"\xa0\xb2\x00\x04\x02",

        # # Read record 4
        # b"\xa0\xb2\x04\x04\x02",

        # # Read next records
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",

        # # Read prev records
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",



        # # Select test cyclic file
        # b"\xa0\xa4\x00\x00\x02\x30\x3a",

        # # Read current record
        # #b"\xa0\xb2\x00\x04\x02",

        # # Read record 4
        # #b"\xa0\xb2\x04\x04\x02",

        # # Read next records
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",
        # b"\xa0\xb2\x00\x02\x02",

        # # Read prev records
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",
        # b"\xa0\xb2\x00\x03\x02",



        # Select test linear file
        #b"\xa0\xa4\x00\x00\x02\x30\x39",

        # # Invalidate
        #b"\xa0\x04\x00\x00\x00",

        # Rehabilitate
        #b"\xa0\x44\x00\x00\x00",
    ]

    for msg in msgs:
        s.sendall(getlen(msg) + msg)
        time.sleep(0.5)


    thread.join()

if __name__ == '__main__':
    main()
