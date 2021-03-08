import time
import struct

class ECParse:

	def __init__(self):
		self.csv_header = "Time (s),valve_states (),pressure[0] (counts),pressure[1] (counts),pressure[2] (counts),pressure[3] (counts),pressure[4] (counts),pressure[5] (counts),e_batt (Volts),main_cycle_time (Amps),motor_cycle_time (),ivlv[0] (Amps),ivlv[1] (Amps),ivlv[2] (Amps),ivlv[3] (Amps),ivlv[4] (Amps),ivlv[5] (Amps),ivlv[6] (Amps),ivlv[7] (Amps),evlv[0] (Volts),evlv[1] (Volts),evlv[2] (Volts),evlv[3] (Volts),evlv[4] (Volts),evlv[5] (Volts),evlv[6] (Volts),evlv[7] (Volts),e3v (Volts),e5v (Volts),i5v (amps),i3v (amps),last_command_id (),i_mtr_ab[0] (amps),i_mtr_ab[1] (amps),i_mtr_ab[2] (amps),i_mtr_ab[3] (amps),i_mtr[0] (amps),i_mtr[1] (amps),zero (Volts),zero (Volts),zero (Volts),zero (Volts),zero (us),zero (ms),zero (ms),zero (ul),\n"
		self.valve_states = 0
		self.pressure = [0]*22
		self.samplerate = 0
		self.motor_setpoint = [0]*4
		self.main_cycle_time = 0
		self.motor_cycle_time = 0
		self.adc_cycle_time = 0
		self.LOG_TO_AUTO = 0
		self.auto_states = 0
		self.debug = [0]*8
		self.tmtr = 0
		self.error_code = 0
		self.LOGGING_ACTIVE = 0
		self.current_page = -1
		self.zero = 0
		self.e28v = 0
		self.i3v = 0
		self.i5v = 0
		self.micros = 
		self.log_string = ""
		self.num_items = 46
		
		self.dict = {}
		
		self.items = [''] * self.num_items
		self.items[0] = 'valve_states' 
		self.items[1] = 'pressure[0]' 
		self.items[2] = 'pressure[1]' 
		self.items[3] = 'pressure[2]' 
		self.items[4] = 'pressure[3]' 
		self.items[5] = 'pressure[4]' 
		self.items[6] = 'pressure[5]' 
		self.items[7] = 'e_batt' 
		self.items[8] = 'main_cycle_time' 
		self.items[9] = 'motor_cycle_time' 
		self.items[10] = 'ivlv[0]' 
		self.items[11] = 'ivlv[1]' 
		self.items[12] = 'ivlv[2]' 
		self.items[13] = 'ivlv[3]' 
		self.items[14] = 'ivlv[4]' 
		self.items[15] = 'ivlv[5]' 
		self.items[16] = 'ivlv[6]' 
		self.items[17] = 'ivlv[7]' 
		self.items[18] = 'evlv[0]' 
		self.items[19] = 'evlv[1]' 
		self.items[20] = 'evlv[2]' 
		self.items[21] = 'evlv[3]' 
		self.items[22] = 'evlv[4]' 
		self.items[23] = 'evlv[5]' 
		self.items[24] = 'evlv[6]' 
		self.items[25] = 'evlv[7]' 
		self.items[26] = 'e3v' 
		self.items[27] = 'e5v' 
		self.items[28] = 'i5v' 
		self.items[29] = 'i3v' 
		self.items[30] = 'last_command_id' 
		self.items[31] = 'i_mtr_ab[0]' 
		self.items[32] = 'i_mtr_ab[1]' 
		self.items[33] = 'i_mtr_ab[2]' 
		self.items[34] = 'i_mtr_ab[3]' 
		self.items[35] = 'i_mtr[0]' 
		self.items[36] = 'i_mtr[1]' 
		self.items[37] = 'zero' 
		self.items[38] = 'zero' 
		self.items[39] = 'zero' 
		self.items[40] = 'zero' 
		self.items[41] = 'zero' 
		self.items[42] = 'zero' 
		self.items[43] = 'zero' 
		self.items[44] = 'zero' 

	def parse_packet(self, packet):
		byte_rep = packet[0:4]
		self.valve_states = int((float(struct.unpack("<I", byte_rep)[0]))/1)
		self.dict[self.items[0]] = self.valve_states
		byte_rep = packet[4:6]
		self.pressure[0] = float((float(struct.unpack("<h", byte_rep)[0]))/1)
		self.dict[self.items[1]] = self.pressure[0]
		byte_rep = packet[6:8]
		self.pressure[1] = float((float(struct.unpack("<h", byte_rep)[0]))/1)
		self.dict[self.items[2]] = self.pressure[1]
		byte_rep = packet[8:10]
		self.pressure[2] = float((float(struct.unpack("<h", byte_rep)[0]))/1)
		self.dict[self.items[3]] = self.pressure[2]
		byte_rep = packet[10:12]
		self.pressure[3] = float((float(struct.unpack("<h", byte_rep)[0]))/1)
		self.dict[self.items[4]] = self.pressure[3]
		byte_rep = packet[12:14]
		self.pressure[4] = float((float(struct.unpack("<h", byte_rep)[0]))/1)
		self.dict[self.items[5]] = self.pressure[4]
		byte_rep = packet[14:16]
		self.pressure[5] = float((float(struct.unpack("<h", byte_rep)[0]))/1)
		self.dict[self.items[6]] = self.pressure[5]
		byte_rep = packet[16:18]
		self.e_batt = float((float(struct.unpack("<h", byte_rep)[0]))/1000)
		self.dict[self.items[7]] = self.e_batt
		byte_rep = packet[18:20]
		self.main_cycle_time = int((float(struct.unpack("<h", byte_rep)[0]))/100)
		self.dict[self.items[8]] = self.main_cycle_time
		byte_rep = packet[20:21]
		self.motor_cycle_time = int((float(struct.unpack("<B", byte_rep)[0]))/1)
		self.dict[self.items[9]] = self.motor_cycle_time
		byte_rep = packet[21:22]
		self.ivlv[0] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[10]] = self.ivlv[0]
		byte_rep = packet[22:23]
		self.ivlv[1] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[11]] = self.ivlv[1]
		byte_rep = packet[23:24]
		self.ivlv[2] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[12]] = self.ivlv[2]
		byte_rep = packet[24:25]
		self.ivlv[3] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[13]] = self.ivlv[3]
		byte_rep = packet[25:26]
		self.ivlv[4] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[14]] = self.ivlv[4]
		byte_rep = packet[26:27]
		self.ivlv[5] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[15]] = self.ivlv[5]
		byte_rep = packet[27:28]
		self.ivlv[6] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[16]] = self.ivlv[6]
		byte_rep = packet[28:29]
		self.ivlv[7] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[17]] = self.ivlv[7]
		byte_rep = packet[29:30]
		self.evlv[0] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[18]] = self.evlv[0]
		byte_rep = packet[30:31]
		self.evlv[1] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[19]] = self.evlv[1]
		byte_rep = packet[31:32]
		self.evlv[2] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[20]] = self.evlv[2]
		byte_rep = packet[32:33]
		self.evlv[3] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[21]] = self.evlv[3]
		byte_rep = packet[33:34]
		self.evlv[4] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[22]] = self.evlv[4]
		byte_rep = packet[34:35]
		self.evlv[5] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[23]] = self.evlv[5]
		byte_rep = packet[35:36]
		self.evlv[6] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[24]] = self.evlv[6]
		byte_rep = packet[36:37]
		self.evlv[7] = float((float(struct.unpack("<B", byte_rep)[0]))/10)
		self.dict[self.items[25]] = self.evlv[7]
		byte_rep = packet[37:41]
		self.e3v = float((float(struct.unpack("<i", byte_rep)[0]))/100)
		self.dict[self.items[26]] = self.e3v
		byte_rep = packet[41:45]
		self.e5v = float((float(struct.unpack("<i", byte_rep)[0]))/100)
		self.dict[self.items[27]] = self.e5v
		byte_rep = packet[45:46]
		self.i5v = float((float(struct.unpack("<B", byte_rep)[0]))/100)
		self.dict[self.items[28]] = self.i5v
		byte_rep = packet[46:47]
		self.i3v = float((float(struct.unpack("<B", byte_rep)[0]))/100)
		self.dict[self.items[29]] = self.i3v
		byte_rep = packet[47:49]
		self.last_command_id = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[30]] = self.last_command_id
		byte_rep = packet[49:51]
		self.i_mtr_ab[0] = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[31]] = self.i_mtr_ab[0]
		byte_rep = packet[51:53]
		self.i_mtr_ab[1] = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[32]] = self.i_mtr_ab[1]
		byte_rep = packet[53:55]
		self.i_mtr_ab[2] = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[33]] = self.i_mtr_ab[2]
		byte_rep = packet[55:57]
		self.i_mtr_ab[3] = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[34]] = self.i_mtr_ab[3]
		byte_rep = packet[57:59]
		self.i_mtr[0] = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[35]] = self.i_mtr[0]
		byte_rep = packet[59:61]
		self.i_mtr[1] = float((float(struct.unpack("<H", byte_rep)[0]))/100)
		self.dict[self.items[36]] = self.i_mtr[1]
		byte_rep = packet[61:63]
		self.zero = float((float(struct.unpack("<H", byte_rep)[0]))/1000)
		self.dict[self.items[37]] = self.zero
		byte_rep = packet[63:65]
		self.zero = float((float(struct.unpack("<H", byte_rep)[0]))/1000)
		self.dict[self.items[38]] = self.zero
		byte_rep = packet[65:67]
		self.zero = float((float(struct.unpack("<H", byte_rep)[0]))/1000)
		self.dict[self.items[39]] = self.zero
		byte_rep = packet[67:69]
		self.zero = float((float(struct.unpack("<H", byte_rep)[0]))/1000)
		self.dict[self.items[40]] = self.zero
		byte_rep = packet[69:77]
		self.zero = int((float(struct.unpack("<L", byte_rep)[0]))/1)
		self.dict[self.items[41]] = self.zero
		byte_rep = packet[77:81]
		self.zero = int((float(struct.unpack("<I", byte_rep)[0]))/1)
		self.dict[self.items[42]] = self.zero
		byte_rep = packet[81:85]
		self.zero = int((float(struct.unpack("<I", byte_rep)[0]))/1)
		self.dict[self.items[43]] = self.zero
		byte_rep = packet[85:86]
		self.zero = int((float(struct.unpack("<B", byte_rep)[0]))/1)
		self.dict[self.items[44]] = self.zero
		self.log_string = str(time.clock())+','+str(self.valve_states)+','+str(self.pressure[0])+','+str(self.pressure[1])+','+str(self.pressure[2])+','+str(self.pressure[3])+','+str(self.pressure[4])+','+str(self.pressure[5])+','+str(self.e_batt)+','+str(self.main_cycle_time)+','+str(self.motor_cycle_time)+','+str(self.ivlv[0])+','+str(self.ivlv[1])+','+str(self.ivlv[2])+','+str(self.ivlv[3])+','+str(self.ivlv[4])+','+str(self.ivlv[5])+','+str(self.ivlv[6])+','+str(self.ivlv[7])+','+str(self.evlv[0])+','+str(self.evlv[1])+','+str(self.evlv[2])+','+str(self.evlv[3])+','+str(self.evlv[4])+','+str(self.evlv[5])+','+str(self.evlv[6])+','+str(self.evlv[7])+','+str(self.e3v)+','+str(self.e5v)+','+str(self.i5v)+','+str(self.i3v)+','+str(self.last_command_id)+','+str(self.i_mtr_ab[0])+','+str(self.i_mtr_ab[1])+','+str(self.i_mtr_ab[2])+','+str(self.i_mtr_ab[3])+','+str(self.i_mtr[0])+','+str(self.i_mtr[1])+','+str(self.zero)+','+str(self.zero)+','+str(self.zero)+','+str(self.zero)+','+str(self.zero)+','+str(self.zero)+','+str(self.zero)+','+str(self.zero)+','