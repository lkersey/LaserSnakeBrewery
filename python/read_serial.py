import serial
import re
import sql
import time

ser = serial.Serial('/dev/ttyACM0', 9600)

#Expected format to extract from the serial port
expected = r"temperature_status;[0-9.]+;[0-9.]+;[0-9.]+;[0-9]"

while ser.isOpen():
    line = ser.readline()
    m = re.match(expected, line)
    if (m != None):
        found = m.group(0)
        data = found.split(';')
        print data
        sql.write_status(time.time(), float(data[1]), float(data[2]), float(data[3]), int(data[4]))
