
import time
import spidev
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import copy
import RPi.GPIO as GPIO
import sys
#--------------------------------user
import general_variable as gv
import runpy

#=======================================================================
# parameter
#=======================================================================

gv.CMD_TYPE = 3  # 0=count, 1=bf_rms, 2=mic, 3=all
gv.PKT_BYTE_LEN_MAX = 4095 #3072 #multiple of 3!!!

gv.IS_SET_BPF = 1
gv.IS_IIR_COEF_20_30 = 0

#---------------set common reg
gv.MIC_SEL = 0 #111 #0~111
gv.TEST_ENABLE = 1
gv.TEST_FREQ = 25 #1~50, unit:kHz
gv.IS_MIC_AFE = 0 #if 0, after BPF, if 1, mic data is AFE output (before BPF)

gv.GAIN = 1 #(0.02 = -40dB when fpga 0.5 mag gen)

VLEN = 100


#=======================================================================
# enable SPI
#=======================================================================

#------------------------------------------------
# Enable SPI0
gv.spi0 = spidev.SpiDev()
gv.spi0.open(0,0)

gv.spi0.mode = 0
gv.spi0.lsbfirst=False
gv.spi0.max_speed_hz = 20000000 #20MHz

#------------------------------------------------
# Enable SPI1
gv.spi1 = spidev.SpiDev()
gv.spi1.open(1,0) #cs0 = pin12/GPIO18

gv.spi1.mode = 0
gv.spi1.lsbfirst=False
gv.spi1.max_speed_hz = 6000000 #6MHz

#------------------------------------------------
# GPIO

GPIO.setmode(GPIO.BCM) #number meaning = gpio

# GPIO numbers of Main board; Left 1.0 / Right 2.0
gpio_sleep_enable = 7		    # 6 / 7
gpio_rom_update_enable = 14	    # 7 / 14
gpio_new_bf_data = 16		    # 4 / 16
gpio_spi_reset_n = 17		    # 5 / 17

GPIO.setup(gpio_sleep_enable, GPIO.OUT)
GPIO.setup(gpio_rom_update_enable, GPIO.OUT)
# ~ GPIO.setup(gpio_new_bf_data, GPIO.IN) # pull_up_down=GPIO.PUD_UP
GPIO.setup(gpio_spi_reset_n, GPIO.OUT)

#------------------------------------------------
# GPIO - default value
GPIO.output(gpio_sleep_enable , GPIO.LOW)
GPIO.output(gpio_rom_update_enable , GPIO.LOW)

#------------------------------------------------
# reset SPI0/ SPI1
GPIO.output(gpio_spi_reset_n , GPIO.LOW)
time.sleep(0.1)
GPIO.output(gpio_spi_reset_n , GPIO.HIGH)
time.sleep(0.1)


#=======================================================================
# set SPI1 register
#=======================================================================

runpy.run_path('set_spi1_reg.py');

#=======================================================================
# read spi0 data
#=======================================================================
time.sleep(0.1)

gv.read_cmd = [0]*3


if gv.CMD_TYPE == 0:
    gv.read_cmd[0] = 0xFC
    gv.read_cmd[1] = 0x55
    gv.read_cmd[2] = 0x00
    gv.read_word_len = 3

elif gv.CMD_TYPE == 1:
    gv.read_cmd[0] = 0xFC
    gv.read_cmd[1] = 0x55
    gv.read_cmd[2] = 0x01    
    gv.read_word_len = 2627
elif gv.CMD_TYPE == 2:
    gv.read_cmd[0] = 0xFC
    gv.read_cmd[1] = 0x55
    gv.read_cmd[2] = 0x02    
    gv.read_word_len = 16003      
elif gv.CMD_TYPE == 3:
    gv.read_cmd[0] = 0xFC
    gv.read_cmd[1] = 0x55
    gv.read_cmd[2] = 0x77
    gv.read_word_len = 18627

gv.cnt_pre = 0

# 1 word = 3 byte
gv.read_byte_len = gv.read_word_len * 3
gv.cnt_repeat = 0

# transfer packet times
gv.num_repeat = int(gv.read_byte_len/gv.PKT_BYTE_LEN_MAX)+1
# final packet length
gv.num_final = gv.read_byte_len % gv.PKT_BYTE_LEN_MAX

# bf data initailize
gv.bf_data = np.zeros([30,40])
gv.rms_data = np.zeros(112)
gv.flag_bf = 0
# mic data  intialize
gv.mic_data = np.zeros(8000)
gv.bf_mic_data = np.zeros(8000)
gv.flag_mic = 0
#=======================================================================
# set plot for debug
#=======================================================================
FS = 200
TS = 1/FS
x = np.linspace(0,TS * (VLEN-1),VLEN)
plt.ion()


if gv.CMD_TYPE == 1:
	fig = plt.figure(figsize=(6, 10));
	gs = gridspec.GridSpec(10, 1)
	ax1 = fig.add_subplot(gs[:6, 0])
	ax2 = fig.add_subplot(gs[7:8, 0])
	ax3 = fig.add_subplot(gs[9, 0])
	plt.subplots_adjust(hspace=0.2)	
	
	gv.bf_data[10,10] = 20
	image = ax1.imshow(gv.bf_data, cmap='viridis')
	plt.colorbar(image)
	ax2.set_title('MSL line')
	ax3.set_title('mic RMS')
	line2, = ax2.plot(np.zeros(40))	
	line3, = ax3.plot(np.zeros(112))	
	
if gv.CMD_TYPE == 2:
	fig = plt.figure('time data');
	
	ax1 = fig.add_subplot(211)
	ax2 = fig.add_subplot(212)
	plt.subplots_adjust(hspace=0.4)		
	
	ax1.set_title('mic[%d] signal'%(gv.MIC_SEL))
	ax2.set_title('center mic')
	line1, = ax1.plot(x, np.zeros(VLEN))
	line2, = ax2.plot(x, np.zeros(VLEN))


#=======================================================================
# gpio interrupt setting
#=======================================================================
gv.cnt = 0
def gpio_interrupt_handler(channel):
    # ~ print('.')
    gv.cnt = gv.cnt + 1;
    gv.rdata = []
    
    #-------------------------------------------------------------------
    # read command
    gv.spi0.xfer2(copy.deepcopy(gv.read_cmd))    
    
    #-------------------------------------------------------------------
    # read data
    for n in range (gv.num_repeat):
        
        #----------------------------------------------------------------
        # set data length      
        if n == gv.num_repeat-1:
            gv.data0 = [0] * gv.num_final
        else:
            gv.data0 = [0] * gv.PKT_BYTE_LEN_MAX
        
        #----------------------------------------------------------------
        # read data       
        gv.rdata = gv.rdata + gv.spi0.xfer2(gv.data0)

    #-------------------------------------------------------------------
    # check data ( 1 sec period
    if gv.cnt >= 25 :        
        
        gv.cnt = 0
        
        rlen = len(gv.rdata) #byte length
        wlen = int(rlen/3)  #word length
        
        print(rlen)
        #-------------------------------------
        # head
        head = gv.rdata[0] << 16 | gv.rdata[1]<< 8 | gv.rdata[2];
        #-------------------------------------
        # count        
        cnt_now = gv.rdata[3] << 16 | gv.rdata[4]<< 8 | gv.rdata[5];
        cnt_diff = cnt_now - gv.cnt_pre;
        gv.cnt_pre = cnt_now 
        #-------------------------------------
        # csum from FPGA 
        csum = gv.rdata[-3] << 16 | gv.rdata[-2]<< 8 | gv.rdata[-1];
        #-------------------------------------
        # csum from received data    
        csum_cal = 0
        for k in range(wlen-1):
            word = gv.rdata[k*3] << 16 | gv.rdata[k*3+1]<< 8 | gv.rdata[k*3+2];
            csum_cal = csum_cal + word
            
        csum_cal = csum_cal % 2**24
        csum_check = csum == csum_cal
        #-------------------------------------
        # display result     
        print("count %d (%d)"%(cnt_now, cnt_diff))
        print("rdata length = %d: %s %s"%(rlen, gv.rdata[0:15], gv.rdata[-15:]))
        print("head %X csum %X  csum cal %X (%s)"%(head, csum, csum_cal, csum_check))                
        #-------------------------------------
        # BF RMS data
        if gv.CMD_TYPE == 1:
            #------------------------------------------------
            # BF
            rn = 29
            cn = 0
            for k in range(1200):
                tmp = gv.rdata[k*6+6] << 40 | gv.rdata[k*6+7] << 32 | gv.rdata[k*6+8] << 24 | gv.rdata[k*6+9] << 16 | gv.rdata[k*6+10] << 8 | gv.rdata[k*6+11];
                gv.bf_data[rn, cn] = tmp
                rn = rn - 1
                
                if rn == -1:
                    rn = 29
                    cn = cn + 1
            #------------------------------------------------
            # RMS
            for k in range(112):
                tmp = gv.rdata[k*6+7206] << 40 | gv.rdata[k*6+7207] << 32 | gv.rdata[k*6+7208] << 24 | gv.rdata[k*6+7209] << 16 | gv.rdata[k*6+7210] << 8 | gv.rdata[k*6+7211];
                gv.rms_data[k] = tmp; #gv.rdata[k*3+7206] << 16 | gv.rdata[k*3+7207]<< 8 | gv.rdata[k*3+7208];
            
            
            if csum_check == True:
                gv.flag_bf = 1


        #-------------------------------------
        # mic/center sum mic
        if gv.CMD_TYPE == 2:
            #------------------------------------------------
            # mic : even / odd
            for k in range(8000):
                tmp = gv.rdata[k*6+6] << 16 | gv.rdata[k*6+7] << 8 | gv.rdata[k*6+8]
                if tmp >= 2**23:
                    tmp = tmp - 2**24                
                gv.mic_data[k] = tmp
                
                tmp = gv.rdata[k*6+9] << 16 | gv.rdata[k*6+10] << 8 | gv.rdata[k*6+11]
                if tmp >= 2**23:
                    tmp = tmp - 2**24                                  
                gv.bf_mic_data[k] = tmp
                
            if csum_check == True:
                gv.flag_mic = 1                 

     
GPIO.setup(gpio_new_bf_data, GPIO.IN)      
GPIO.add_event_detect(gpio_new_bf_data, GPIO.RISING, callback=gpio_interrupt_handler)


#=======================================================================
# loop
#=======================================================================

#---------------------------------- fixed parameter

SPL_MAX = 122 #dB
numOfMic = 112
BF_BLOCK_LEN = 8000


pow_constant = 1/(numOfMic**2 * BF_BLOCK_LEN)
#integer u48.0 ->  u25.23 format : /2^23
pow_constant  = pow_constant  / 2**23

# rms mic conversion
# 8000 times accumulation U13.35
#integer u48 -> u13.35 : /2^35
pow_rms_constant = 1/(2**35  * BF_BLOCK_LEN)

if gv.CMD_TYPE == 1 or gv.CMD_TYPE == 2:
    fig.canvas.draw()

while True:
    time.sleep(0.1)
    # ~ gpio_interrupt_handler(0) #test
    
	#-----------------------------------------------------
	# view bf data    
    if gv.flag_bf == 1:
        gv.flag_bf = 0
        
        #-------------------------------------------------------------
        # bf map
        # ~ print('>>>>>>>>>>>>>>')
        # ~ print(gv.bf_data.max())

        bf_db = SPL_MAX + 10 * np.log10(gv.bf_data * pow_constant)
        # ~ print(bf_db.max())
        
        bf_max = bf_db.max()
        bf_min = bf_db.min()
        bf_diff = bf_max - bf_min
		# print('min max diff = %f, min = %f max = %f'%(bf_diff, bf_min, bf_max))        
       
        #bf_db = bf_db - bf_min + 0.0001
        # bf_db = bf_db / bf_db.max() #* 250
        
        image.set_data(bf_db)     
        image.set_clim(vmin=bf_min, vmax=bf_max)

        ax1.set_title('min max diff = %.2f, min = %.2f max = %.2f'%(bf_diff, bf_min, bf_max))
        
        
        #-------------------------------------------------------------
        # mic msl
        max_idx = np.argmax(bf_db)
        max_idx = np.unravel_index(max_idx, bf_db.shape) #max value row index
                
        msl_line = bf_db[max_idx[0],:]
        
        y2 = msl_line
        line2.set_ydata(y2)
        
        ax2.draw_artist(ax2.patch)
        ax2.draw_artist(line2)   
        
        ymax2 = y2.max()
        ymin2 = ymax2 - 30 #y2.min()
        
        ax2.set_ylim(ymin2, ymax2 + 0)       
        ax2.grid(True)  
        ax2.set_title('MSL min max diff = %.2f, min = %.2f max = %.2f'%(bf_diff, bf_min, bf_max))
        #-------------------------------------------------------------        
        # mic rms
        
        # find mic error index
        rms_th = gv.rms_data.mean() * 0.1
        mic_low = np.where(gv.rms_data < rms_th)[0]
        
        rms_db = SPL_MAX + 10 * np.log10(gv.rms_data * pow_rms_constant + 0.0000000000000001)  #prevent 0 dB error
        
        rms_max = rms_db.max()
        rms_min = rms_db.min()
        rms_av = rms_db.mean()
        rms_diff = rms_max - rms_min
        # ~ print(rms_db[:20])
        print('Mic rms max[%.2f] min[%.2f] av[%.2f]\nlow power mic idx %s'%(rms_max, rms_min, rms_av, mic_low))
        
        ax3.set_title('Mic rms max[%.2f] min[%.2f] av[%.2f]\nlow power mic idx %s'%(rms_max, rms_min, rms_av, mic_low))

        
        y3 = copy.deepcopy(rms_db)
        line3.set_ydata(y3)
        
        ax3.draw_artist(ax3.patch)
        ax3.draw_artist(line3)   
        
        ymin3, ymax3 = y3.min(), y3.max()
        ymin3 = 0
        yspan3 = ymax3 - ymin3
        ymargin3 = yspan3 * 0.1
        ax3.set_ylim(ymin3 - ymargin3, ymax3 + ymargin3) 
 
        fig.canvas.blit(ax1.bbox)
        fig.canvas.blit(ax2.bbox)
        fig.canvas.blit(ax3.bbox)
        fig.canvas.flush_events()                      
        # ~ print(gv.rms_data)
        plt.pause(0.1)
                
	#-----------------------------------------------------
	# view mic data
    if gv.flag_mic == 1:
        gv.flag_mic = 0
        print(gv.mic_data[:20])
        
        #----------------------------------
        # update y data  
        y1 = gv.mic_data[:VLEN]
        y2 = gv.bf_mic_data[:VLEN]
        y2 = y2 * 2**7 / 112
        line1.set_ydata(y1)
        line2.set_ydata(y2)
        
        ax1.draw_artist(ax1.patch)
        ax1.draw_artist(line1)
        ax2.draw_artist(ax2.patch)
        ax2.draw_artist(line2)        
        
        #----------------------------------
        # y axis with margin
        ymin1, ymax1 = y1.min(), y1.max()
        yspan1 = ymax1 - ymin1
        ymargin1 = yspan1 * 0.1
        ax1.set_ylim(ymin1 - ymargin1, ymax1 + ymargin1)  
        
        ymin2, ymax2 = y2.min(), y2.max()
        yspan2 = ymax2 - ymin2
        ymargin2 = yspan2 * 0.1
        ax2.set_ylim(ymin2 - ymargin2, ymax2 + ymargin2)
        #----------------------------------
        # refresh      
        fig.canvas.blit(ax1.bbox)
        fig.canvas.blit(ax2.bbox)
        fig.canvas.flush_events()
        plt.pause(0.1)

#eof
