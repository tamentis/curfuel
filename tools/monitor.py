#!/usr/bin/python
import sys
import serial

port = serial.Serial("/dev/ttyACM0", 9600, timeout=1)

try:
    while True:
        sys.stdout.write(port.read())
        # print(port.read(),)
        sys.stdout.flush()
except KeyboardInterrupt:
    port.read(16)
    print("\rInterrupted...")
except:
    raise
finally:
    port.close()

