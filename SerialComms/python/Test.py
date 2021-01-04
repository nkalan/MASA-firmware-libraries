
import serial
import serial.tools.list_ports
import struct



print("Ylol")

ser = serial.Serial(port='COM3', baudrate=int(115200), timeout=1.5)


for p in serial.tools.list_ports.comports():
    print(p)

last = 0;
if(ser.is_open):
    while(1==1):
        # Read a packet
        try:
            packet = ser.read_until(b'\x00')
            print(packet)
            #print(packet)
            #print(len(packet))
            byte_rep = packet[10:14]
            byte_rep2 = packet[0:1]
            #https://docs.python.org/3.0/library/struct.html
            print(int((float(struct.unpack("<B", byte_rep2)[0])) / 1))
            #print(packet[0])
            micro = int((float(struct.unpack("<I", byte_rep)[0])) / 1)
            #print(micro-last)
            #last = micro
        except:
            print("FAIL");



#print(packet[9:13])




