#from datetime import datetime
#datetime.now().strftime('%Y-%m-%d %H:%M:%S')


import serial
import numpy as np


""" Exception for when you try to use a 'byte' that's not 8bits long """
class ByteSizeException(Exception):
    pass
    
'''
addr  = 'COM4'
baud  = 115200
fname = 'log.txt'
fmode = 'ab'
reps  = 10
'''

'''
a = np.uint16(2)
print(a)
b = chr(a)
print(b)
'''

'''
file = open('log.txt', 'w')
file.write(chr(1) + '\n')
file.write('This is our new text file\n')
file.write('and this is another line.\n')
file.write('Why? Because we can.\n')
 
file.close()
'''

"""
Converts a uint8t to a 2-character hexadecimal number
with the format '0xXY', where X and Y are hex digits.

@param byte <uint8> an 8bit integer to be converted to hexadecimal
@retval A string representing the input byte in hexadecimal
"""
def byte_to_hex(byte):
    # Raise an exception if the user tries to convert a float or an int
    # of the wrong size
    """
    if byte > 0xFF or byte < 0x00 or type(byte) == float:
        raise ByteSizeException

    # Idk how this works, I found it on stackoverflow and it just works
    if byte < 0x10:
        return "0x0%X" % np.uint8(byte)
    else:
        return "0x%X" % np.uint8(byte)
    """
    
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

def main():
    """
    # Literally copied word for word from stackoverflow
    # https://stackoverflow.com/questions/676172/full-examples-of-using-pyserial-package
    ser = serial.Serial('COM4')
    #ser.port = "COM4"
    ser.baudrate = 115200
    ser.bytesize = serial.EIGHTBITS     #number of bits per bytes
    ser.parity = serial.PARITY_NONE     #set parity check: no parity
    ser.stopbits = serial.STOPBITS_ONE  #number of stop bits
    ser.timeout = 1                     #non-block read
    ser.xonxoff = False                 #disable software flow control
    ser.rtscts = False                  #disable hardware (RTS/CTS) flow control
    ser.dsrdtr = False                  #disable hardware (DSR/DTR) flow control
    ser.writeTimeout = 2                #timeout for write
    
    try: 
        ser.open()
    except serial.SerialException as e:
        print("error open serial port: " + str(e))
        exit()
    """
    addr  = 'COM4'
    baud  = 115200
    fname = 'log.txt'
    fmode = 'ab'
    reps  = 10

    '''
    file = open('log.txt', 'w')
    file.write(chr(1) + '\n')
    file.write('This is our new text file\n')
    file.write('and this is another line.\n')
    file.write('Why? Because we can.\n')
     
    file.close()
    '''
    
    try:
        with serial.Serial(addr, baud) as ser, open(fname,fmode) as outf:
            #ser.port = "COM4"
            #ser.baudrate = 115200
            #ser.bytesize = serial.EIGHTBITS     #number of bits per bytes
            #ser.parity = serial.PARITY_NONE     #set parity check: no parity
            #ser.stopbits = serial.STOPBITS_ONE  #number of stop bits
            #ser.timeout = 1                     #non-block read
            #ser.xonxoff = False                 #disable software flow control
            #ser.rtscts = False                  #disable hardware (RTS/CTS) flow control
            #ser.dsrdtr = False                  #disable hardware (DSR/DTR) flow control
            #ser.writeTimeout = 2                #timeout for write
            while (True):
                x = ser.read(size=1)
                print(x)
                outf.write(x)
                outf.flush()
    except KeyboardInterrupt:
        print('Interrupted')
        outf.close()
        ser.close()
        sys.exit(0)

    outf.close()
    port.close()
    

        

if __name__ == '__main__':
    main()