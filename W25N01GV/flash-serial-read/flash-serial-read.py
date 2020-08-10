#from datetime import datetime
#datetime.now().strftime('%Y-%m-%d %H:%M:%S')

"""
import serial
ser = serial.Serial('COM4', 115200)

while (True):
    ser_bytes = ser.readline()
    print(ser_bytes)
    print("hello")
"""    
    

import serial
import numpy as np

addr  = 'COM4'
baud  = 115200
fname = 'log.txt'
fmode = 'ab'
reps  = 10

a = np.uint16(2)
print(a)
b = chr(a)
print(b)

file = open('log.txt', 'w')
file.write(chr(1) + '\n')
file.write('This is our new text file\n')
file.write('and this is another line.\n')
file.write('Why? Because we can.\n')
 
file.close()

"""
try:
    with serial.Serial(addr,baud) as port, open(fname,fmode) as outf:
        while (True):
            x = port.read(size=1)
            print(x)
            outf.write(x)
            outf.flush()
except KeyboardInterrupt:
    print('Interrupted')
    outf.close()
    port.close()
    sys.exit(0)

outf.close()
port.close()
"""
