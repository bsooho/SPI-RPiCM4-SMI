
import time
import spidev
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import copy
import sys
import runpy
#--------------------------------
import general_variable as gv


#=======================================================================
# set common register
#=======================================================================

write_cmd = [0] *4*3

write_cmd[0] = 0xFC
write_cmd[1] = 0x00
write_cmd[2] = 0x03

head = 0xFC0003
csum = head;

#----------------common 0 just test
wdata = 0
csum = (csum + wdata) & 0xFFFFFF
byte0 = (wdata >> 16) & 0xFF
byte1 = (wdata >> 8) & 0xFF
byte2 = wdata & 0xFF

write_cmd[3] = byte0
write_cmd[4] = byte1
write_cmd[5] = byte2

#----------------common 1 reg
common1 = gv.TEST_FREQ << 16 | gv.TEST_ENABLE << 9 | gv.IS_MIC_AFE << 8 | gv.MIC_SEL
wdata = common1

csum = (csum + wdata) & 0xFFFFFF
byte0 = (wdata >> 16) & 0xFF
byte1 = (wdata >> 8) & 0xFF
byte2 = wdata & 0xFF

write_cmd[6] = byte0
write_cmd[7] = byte1
write_cmd[8] = byte2


write_cmd[9] = (csum >> 16) & 0xFF
write_cmd[10] = (csum >> 8) & 0xFF
write_cmd[11] = csum & 0xFF

print('---------------------------------')
print('cmd common write data')
print('MIC sel [%d] TEST EN[%d] TEST FREQ [%d] is mic AFE [%d]'%(gv.MIC_SEL , gv.TEST_ENABLE, gv.TEST_FREQ , gv.IS_MIC_AFE))
print(write_cmd)

gv.spi1.xfer2(copy.deepcopy(write_cmd))
time.sleep(0.1)

# -------------------------------------------
# register 1 command read

read_cmd = [0xAC, 0x00, 0x03]

gv.spi1.xfer2(copy.deepcopy(read_cmd))
time.sleep(0.1)


data_0 = [0] *4*3
read_data = gv.spi1.xfer2(copy.deepcopy(data_0))

#-------------------------------------
# csum from FPGA 
blen = len(read_data) #byte length
wlen = int(blen/3)  #word length
csum_fpga = read_data[-3] << 16 | read_data[-2]<< 8 | read_data[-1];
#-------------------------------------
# csum from received data    
csum_cal = 0
for k in range(wlen-1):
	word = read_data[k*3] << 16 | read_data[k*3+1]<< 8 | read_data[k*3+2];
	csum_cal = csum_cal + word
            
csum_cal = csum_cal & 0xFFFFFF
csum_check = csum_fpga == csum_cal

print('---------------------------------')
print('cmd common read data')
print(read_data)
print("rdata length = %d"%(blen))
print("head %X csum fpga %X  csum cal %X (%s)"%(head, csum_fpga, csum_cal, csum_check))                
print('---------------------------------')


#=======================================================================
# set grid distance
#=======================================================================

#1m distance z**2 value
grid_x = [-10041, -9526, -9011, -8496, -7981, -7466, -6951, -6436, -5922, -5407, -4892, -4377, -3862, -3347, -2832, -2317, -1803, -1288, -773, -258, 257, 772, 1287, 1802, 2316, 2831, 3346, 3861, 4376, 4891, 5406, 5921, 6435, 6950, 7465, 7980, 8495, 9010, 9525, 10040]
grid_y = [-7467, -6952, -6437, -5922, -5407, -4892, -4377, -3863, -3348, -2833, -2318, -1803, -1288, -773, -258, 257, 772, 1287, 1802, 2317, 2832, 3347, 3862, 4376, 4891, 5406, 5921, 6436, 6951, 7466]
distance_z = 131072


write_cmd = [0] *73*3

write_cmd[0] = 0xFC
write_cmd[1] = 0x00
write_cmd[2] = 0x01

head = 0xFC0001
csum = head;

#-------------------------- grid x
for n in range(40):
	wdata = grid_x[n] & 0x3FFFF
	csum = (csum + wdata) & 0xFFFFFF
	byte0 = (wdata >> 16) & 0x3
	byte1 = (wdata >> 8) & 0xFF
	byte2 = wdata & 0xFF
	# ~ print(n, grid_x[n],byte0, byte1, byte2, csum )
	write_cmd[(n+1)*3] = byte0
	write_cmd[(n+1)*3+1] = byte1
	write_cmd[(n+1)*3+2] = byte2

#-------------------------- grid y
for n in range(30):
	wdata = grid_y[n] & 0x3FFFF
	csum = (csum + wdata) & 0xFFFFFF
	byte0 = (wdata >> 16) & 0x3
	byte1 = (wdata >> 8) & 0xFF
	byte2 = wdata & 0xFF
	write_cmd[(n+41)*3] = byte0
	write_cmd[(n+41)*3+1] = byte1
	write_cmd[(n+41)*3+2] = byte2
	
#-------------------------- distance z
wdata = distance_z & 0xFFFFFF
csum = (csum + wdata) & 0xFFFFFF
byte0 = (wdata >> 16) & 0xFF
byte1 = (wdata >> 8) & 0xFF
byte2 = wdata & 0xFF
write_cmd[(71)*3] = byte0
write_cmd[(71)*3+1] = byte1
write_cmd[(71)*3+2] = byte2

#-------------------------- check sum
# ~ csum = csum % 2**24

write_cmd[72*3] = (csum >> 16) & 0xFF
write_cmd[72*3+1] = (csum >> 8) & 0xFF
write_cmd[72*3+2] = csum & 0xFF

print('---------------------------------')
print('cmd xyz write data')
print(write_cmd)

gv.spi1.xfer2(copy.deepcopy(write_cmd))
time.sleep(0.1)



# -------------------------------------------
# register 0 command read

read_cmd = [0xAC, 0x00, 0x01]

gv.spi1.xfer2(copy.deepcopy(read_cmd))
time.sleep(0.1)

data_0 = [0] *73*3
read_data = gv.spi1.xfer2(copy.deepcopy(data_0))

#-------------------------------------
# csum from FPGA 
blen = len(read_data) #byte length
wlen = int(blen/3)  #word length
csum_fpga = read_data[-3] << 16 | read_data[-2]<< 8 | read_data[-1];
#-------------------------------------
# csum from received data    
csum_cal = 0
for k in range(wlen-1):
	word = read_data[k*3] << 16 | read_data[k*3+1]<< 8 | read_data[k*3+2];
	csum_cal = csum_cal + word
            
csum_cal = csum_cal % 2**24
csum_check = csum_fpga == csum_cal

print('---------------------------------')
print('cmd xyz read data')
print(read_data)
print("rdata length = %d"%(blen))
print("head %X csum fpga %X  csum cal %X (%s)"%(head, csum_fpga, csum_cal, csum_check))                
print('---------------------------------')

#=======================================================================
# set IIR coef
#=======================================================================

if gv.IS_SET_BPF == 1:
	# -------------------------------------------
	# register 0 command write

	write_cmd = [0] *11*3

	write_cmd[0] = 0xFC
	write_cmd[1] = 0x00
	write_cmd[2] = 0x02

	head = 0xFC0002
	csum = head;

	#20k ~ 30kHz BW BPF
	if gv.IS_IIR_COEF_20_30 == 1:
		coef_b1 = 0xA # 1010  0 = minus, 1 = plus
		coef_gain = 4 #1563
		coef_gain = int(coef_gain * gv.GAIN)
		coef_a1a2 = [-41416, 58510, -47335, 59339, -37733, 62346, -51992, 63192]	
	#2k ~ 50kHz BW BPF
	else:
		
		coef_b1 = 0x6 # 0110  0 = minus, 1 = plus
		coef_gain = 1563 #1563
		coef_gain = int(coef_gain * gv.GAIN)
		coef_a1a2 = [-25114, 24378, -434, 50257, -61111, 57541, -64863, 64451]
		# ~ coef_a1a2 = [0,0,0,0,0,0,0,0]

	#------------------------ wdata

	for n in range(9):
		if n == 0: #20bits
			wdata = (coef_b1 << 16 | coef_gain) & 0xFFFFF		
		else: #18bits
			wdata = coef_a1a2[n-1] & 0x3FFFF
		
		byte0 = (wdata >> 16) & 0xFF
		byte1 = (wdata >> 8) & 0xFF
		byte2 = wdata & 0xFF		
		write_cmd[n*3 + 3] = byte0
		write_cmd[n*3 + 4] = byte1
		write_cmd[n*3 + 5] = byte2
		
		csum = (csum + wdata) & 0xFFFFFF
		# ~ print(n, wdata, byte0, byte1, byte2, csum)
		
	# ~ csum = csum % 2**24

	write_cmd[10*3] = (csum >> 16) & 0xFF
	write_cmd[10*3+1] = (csum >> 8) & 0xFF
	write_cmd[10*3+2] = csum & 0xFF
    
	print('---------------------------------')
	print('iir bpf coef write data')
	print(write_cmd)

	gv.spi1.xfer2(copy.deepcopy(write_cmd))
	time.sleep(0.1)


	# -------------------------------------------
	# register 0 command read

	read_cmd = [0xAC, 0x00, 0x02]

	gv.spi1.xfer2(copy.deepcopy(read_cmd))
	time.sleep(0.1)

	data_0 = [0] *11*3
	read_data = gv.spi1.xfer2(copy.deepcopy(data_0))


	#-------------------------------------
	# csum from FPGA 
	blen = len(read_data) #byte length
	wlen = int(blen/3)  #word length
	csum_fpga = read_data[-3] << 16 | read_data[-2]<< 8 | read_data[-1];
	#-------------------------------------
	# csum from received data    
	csum_cal = 0
	for k in range(wlen-1):
		word = read_data[k*3] << 16 | read_data[k*3+1]<< 8 | read_data[k*3+2];
		csum_cal = csum_cal + word
				
	csum_cal = csum_cal % 2**24
	csum_check = csum_fpga == csum_cal

	print('---------------------------------')
	print('iir bpf read data')
	print(read_data)
	print("rdata length = %d"%(blen))
	print("head %X csum fpga %X  csum cal %X (%s)"%(head, csum_fpga, csum_cal, csum_check))                
	print('---------------------------------')




