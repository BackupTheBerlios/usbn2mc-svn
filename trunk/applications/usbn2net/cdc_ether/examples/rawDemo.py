#!/usr/bin/python

import sys
import string
import struct
from socket import *

proto = 0x55aa

s = socket(AF_PACKET, SOCK_RAW, proto)
s.bind(("usb0",proto))
#s.bind(("eth1",proto))

ifName,ifProto,pktType,hwType,hwAddr = s.getsockname()

srcAddr = hwAddr
dstAddr ="\xEA\x3D\x22\x1A\x00\x20"
#dstAddr = "\x01\x02\x03\x04\x05\x06"
ethData = "here is some data for an ethernet packet"

txFrame = struct.pack("!6s6sh",dstAddr,srcAddr,proto) + ethData

print "Tx[%d]: "%len(ethData) + string.join(["%02x"%ord(b) for b in ethData]," ")
		      
s.send(txFrame)
'''
rxFrame = s.recv(2048)

dstAddr,srcAddr,proto = struct.unpack("!6s6sh",rxFrame[:14])
ethData = rxFrame[14:]

print "Rx[%d]: "%len(ethData) + string.join(["%02x"%ord(b) for b in ethData]," ")
'''
s.close()
