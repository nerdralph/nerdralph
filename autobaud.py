#!/usr/bin/env python
# 2018 Ralph Doncaster MIT license software
# AVR DEBUGwire autobaud - sends break and expects 0x55 response
# one optional argument: serial port name i.e. /dev/ttyUSB0
#  uses first serial port found if none specified
# uses pyserial library, tested only on python3

import serial
import serial.tools.list_ports as lp
import sys
import time

DEBUG = 0

# counts the number of leading zero bits including the start bit
def bit_width(byte):
    first = -1
    for bit in range(0, 8):
        if byte & 1<<bit:
            if first < 0: first = bit
            break
    return first+1

def baud_guess(speed, width):
    return speed/width

def send_break(ser):
    ser.break_condition = 1
    time.sleep(0.001)
    ser.break_condition = 0

def try_speed(device, speed):
    if DEBUG: print("Reading at", int(speed), "bps.")
    avg = 0
    ser = serial.Serial(port=device, baudrate=speed, timeout=2)
    #ser.send_break(0.01)
    send_break(ser)
    ser.read(1)                         # skip break
    bytes = ser.read(5)                 # 5 zeros in 0x55
    if DEBUG: print("Rx:", bytes)
    if len(bytes) == 0: return 0
    if bytes[0] == 0:
        if DEBUG: print("Target baud <", int(speed/9))
    else:
        if DEBUG: print("Target baud >", int(speed/8))
        width = 0
        for byte in bytes:
            width += bit_width(byte)
        avg = width/len(bytes)
        if DEBUG: print("total, avg:", width, avg)
    ser.close()
    return avg


SPEEDS = [1350000, 900000, 600000, 400000]

if len(sys.argv) == 1:
    device = lp.comports()[0].device
else:
    device = sys.argv[1]

for speed in SPEEDS:
    avg = try_speed(device, speed)
    if avg != 0:
        if DEBUG: print("rough guess:", int(baud_guess(speed, avg)))
        # try slower speeds until a different bit pattern is read
        while True:
            speed *= .985
            if not try_speed(device, speed) >= avg-0.5: break
        break
    else:
        if speed == SPEEDS[len(SPEEDS)-1]:
            print("0")
            # tried all speeds; no target found
            sys.exit(1)

# adjust for slightly more than 0.5 bit-time
final = int(baud_guess(speed, avg-0.555))
#verify speed
ser = serial.Serial(port=device, baudrate=final*5, timeout=2)
send_break(ser)
ser.read(1)                         # skip break
bytes = ser.read(5)
ser.close()
# should read 5x 0xF0
if DEBUG: print("Rx:", bytes)
if bytes[0] != 0xF0:
    print("failed to verify baud rate")

ser = serial.Serial(port=device, baudrate=final)
ser.write(b'\x30')                  # exit dW & run
ser.close()
print (final)
