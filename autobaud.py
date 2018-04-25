#!/usr/bin/python3
# 2018 Ralph Doncaster MIT license software
# AVR debugwire autobaud - sends break and expects 0x55 response
# one optional argument: serial port name i.e. /dev/ttyUSB0
# uses pyserial library

import logging
import serial
import serial.tools.list_ports as lp
import sys
import time

debug=0

# counts the number of leading zero bits including the start bit
def bit_width(byte):
    first = -1
    for bit in range(0,8):
        if (byte & 1<<bit):
            if (first < 0): first = bit
            break
    return first+1

def baud_guess(speed, width):
    return (speed/width)

def send_break(ser):
    ser.break_condition = 1
    time.sleep(0.001)
    ser.break_condition = 0

def try_speed(device, speed):
    if(debug):print("Reading at", int(speed), "bps.")
    avg = 0
    ser=serial.Serial(port=device, baudrate=speed)
    #ser.send_break(0.01)
    send_break(ser)
    ser.read(1)                         # skip break
    bytes = ser.read(5)                 # 5 zeros in 0x55
    if(debug):print("Rx:",  bytes)
    if (bytes[0] == 0):
        if(debug):print("Target baud <", int(speed/9))
    else:
        if(debug):print("Target baud >", int(speed/8))
        width = 0
        for byte in bytes:
            width += bit_width(byte)
        avg = width/len(bytes)
        if(debug):print("total, avg:", width, avg)
    ser.close()
    return avg


speeds = [1350000, 900000, 600000, 400000]

if (len(sys.argv) == 1):
    device = lp.comports()[0].device
else:
    device = sys.argv[1]

for speed in speeds:
    avg = try_speed(device, speed)
    if (avg != 0):
        if(debug):print("rough guess:", int(baud_guess(speed, avg)))
        # try slower speeds until a different bit pattern is read
        while True:
            speed *= .985
            if not try_speed(device, speed) >= avg-0.5 : break
        break

# adjust for 0.5 bit-time
final = int(baud_guess(speed, avg-0.5))
#verify speed
ser=serial.Serial(port=device, baudrate=final*5)
send_break(ser)
ser.read(1)                         # skip break
bytes = ser.read(5)
# should read 5x 0xF0
if(debug):print("Rx:",  bytes)
if(bytes[0] != 0xF0):
    print("failed to verify baud rate")

print("Final:", final)
