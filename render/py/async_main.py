import GLOB
import asyncio
import socket
import numpy as np
import datetime
import struct
import time
import copy

import matplotlib
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

matplotlib.use("TkAgg")

socket_path = "/tmp/fpga_stream_export.sock"

bf_data = np.zeros([30,40])
rms_data = np.zeros(112)
mic_data = np.zeros(8000)
bf_mic_data = np.zeros(8000)

lock = asyncio.Lock()

async def task(lock):


    global socket_path

    global bf_data, rms_data, mic_data, bf_mic_data


    while True:

        await asyncio.sleep(1)

        client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

        client.connect(socket_path)

        print("connected to export socket")

        while True:

            await asyncio.sleep(0.1)

            cycle_start = datetime.datetime.now()

            if GLOB.CMD_TYPE == 1:

                await lock.acquire()

                message = "1"

                client.sendall(message.encode())

                flag_set_byte = client.recv(GLOB.FLAG_SET_BYTE)

                flag_set_arr = [ x for x in flag_set_byte ]

                #print(flag_set_arr)

                response = client.recv(GLOB.CMD_TYPE_1_BYTE_LEN)

                #print(len(response))

                if flag_set_arr[0] != 1 :

                    print("invalid flag")

                    continue

                # render_plot = render.cmd_type_1(response, render_plot)


                #print("cmd_type_1")


                data_raw = []


                response_bytes_len = len(response)

                if response_bytes_len != GLOB.CMD_TYPE_1_BYTE_LEN :

                    print("invalid byte length: " + str(response_bytes_len))

                    tmp_byte = GLOB.CMD_TYPE_1_BYTE_LEN - response_bytes_len

                    tmp_response = client.recv(tmp_byte)

                    print("tmp byte: "+str(tmp_byte))
                    print("tmp response: " + str(len(tmp_response)))

                    if(len(tmp_response) != tmp_byte):
                        
                        print("failed reading, reconnect")

                        client.close()

                        break

                    continue


                for i in range(GLOB.CMD_TYPE_1_LEN):

                    start_index = i * GLOB.DOUBLE_T
                    end_index = start_index + GLOB.DOUBLE_T

                    value_double = struct.unpack("d", response[start_index:end_index])[0]

                    data_raw.append(value_double)
                


                #print(len(data_raw))

                i = 0

                for x in range(GLOB.BF_DATA_X):

                    for y in range(GLOB.BF_DATA_Y):

                        bf_data[x][y] = data_raw[i]

                        i += 1       

                

                for x in range(GLOB.RMS_DATA):

                    rms_data[x] = data_raw[i]

                    i += 1

                await asyncio.sleep(1)

                print(bf_data[0])

                lock.release()


            if GLOB.CMD_TYPE == 2:

                await lock.acquire()

                message = "2"

                client.sendall(message.encode())

                flag_set_byte = client.recv(GLOB.FLAG_SET_BYTE)

                flag_set_arr = [ x for x in flag_set_byte ]

                #print(flag_set_arr)

                response = client.recv(GLOB.CMD_TYPE_2_BYTE_LEN)

                #print(len(response))

                if flag_set_arr[1] != 1 :

                    print("invalid flag")

                    continue

                # render_plot = render.cmd_type_1(response, render_plot)


                #print("cmd_type_2")


                data_raw = []


                response_bytes_len = len(response)

                if response_bytes_len != GLOB.CMD_TYPE_2_BYTE_LEN :

                    print("invalid byte length: " + str(response_bytes_len))

                    tmp_byte = GLOB.CMD_TYPE_2_BYTE_LEN - response_bytes_len

                    tmp_response = client.recv(tmp_byte)

                    print("tmp byte: "+str(tmp_byte))
                    print("tmp response: " + str(len(tmp_response)))

                    if(len(tmp_response) != tmp_byte):
                        
                        print("failed reading, reconnect")

                        client.close()

                        break

                    continue

                for i in range(GLOB.CMD_TYPE_2_LEN):

                    start_index = i * GLOB.DOUBLE_T
                    end_index = start_index + GLOB.DOUBLE_T

                    value_double = struct.unpack("d", response[start_index:end_index])[0]

                    data_raw.append(value_double)
                


                #print(len(data_raw))

                i = 0

                for x in range(GLOB.MIC_DATA):

                    mic_data[x] = data_raw[i]

                    i += 1       

                

                for x in range(GLOB.BF_MIC_DATA):

                    bf_mic_data[x] = data_raw[i]

                    i += 1

                lock.release()

            #cycle_end = datetime.datetime.now()

            #t_delta = cycle_end - cycle_start

            #lapse_ms = int(t_delta.microseconds / 1000)

            #print("lapse_ms: " + str(lapse_ms))

        


    return 

async def printer(lock):
    
    global bf_data, rms_data, mic_data, bf_mic_data

    x = None
    fig = None
    gs = None
    ax1 = None
    ax2 = None
    ax3 = None

    flag_bf = None
    flag_mic = None

    image = None
    line2 = None
    line3 = None

    x = np.linspace(0,GLOB.TS * (GLOB.VLEN-1),GLOB.VLEN)

    await asyncio.sleep(1)

    if GLOB.CMD_TYPE == 1:

        await lock.acquire()
        
        fig = plt.figure(figsize=(6, 10))
        gs = gridspec.GridSpec(10, 1)
        ax1 = fig.add_subplot(gs[:6, 0])
        ax2 = fig.add_subplot(gs[7:8, 0])
        ax3 = fig.add_subplot(gs[9, 0])
        plt.subplots_adjust(hspace=0.2)	

        bf_data[10,10] = 20
        image = ax1.imshow(bf_data, cmap='viridis', animated=True)
        plt.colorbar(image)
        ax2.set_title('MSL line')
        ax3.set_title('mic RMS')
        line2, = ax2.plot(np.zeros(40), animated=True)	
        line3, = ax3.plot(np.zeros(112), animated=True)	

        ax1.draw_artist(image)
        ax2.draw_artist(line2)
        ax3.draw_artist(line3)

        bg1 = fig.canvas.copy_from_bbox(ax1.bbox)
        bg2 = fig.canvas.copy_from_bbox(ax2.bbox)
        bg3 = fig.canvas.copy_from_bbox(ax3.bbox)

        lock.release()

    elif GLOB.CMD_TYPE == 2:

        await lock.acquire()

        fig = plt.figure('time data')

        ax1 = fig.add_subplot(211)
        ax2 = fig.add_subplot(212)
        plt.subplots_adjust(hspace=0.4)		

        ax1.set_title('mic[%d] signal'%(GLOB.MIC_SEL))
        ax2.set_title('center mic')
        line1, = ax1.plot(x, np.zeros(GLOB.VLEN), animated=True)
        line2, = ax2.plot(x, np.zeros(GLOB.VLEN), animated=True)


        ax1.draw_artist(line1)
        ax2.draw_artist(line2)

        bg1 = fig.canvas.copy_from_bbox(ax1.bbox)
        bg2 = fig.canvas.copy_from_bbox(ax2.bbox)

        lock.release()

    plt.show(block=False)

    print("initiated plot rendering")

    while True:

        await asyncio.sleep(0.1)

        if GLOB.CMD_TYPE == 1:

            await lock.acquire()

            fig.canvas.restore_region(bg1)
            fig.canvas.restore_region(bg2)
            fig.canvas.restore_region(bg3)
            
            
            bf_db = GLOB.SPL_MAX + 10 * np.log10(bf_data * GLOB.POW_CONSTANT)
            # ~ print(bf_db.max())
            
            bf_max = bf_db.max()
            bf_min = bf_db.min()
            bf_diff = bf_max - bf_min
            print('min max diff = %f, min = %f max = %f'%(bf_diff, bf_min, bf_max))        

            #bf_db = bf_db - bf_min + 0.0001
            # bf_db = bf_db / bf_db.max() #* 250
            
            image.set_data(bf_db)     
            image.set_clim(vmin=bf_min, vmax=bf_max)

            ax1.draw_artist(image)

            ax1.set_title('min max diff = %.2f, min = %.2f max = %.2f'%(bf_diff, bf_min, bf_max))
            
            
            #-------------------------------------------------------------
            # mic msl
            max_idx = np.argmax(bf_db)
            max_idx = np.unravel_index(max_idx, bf_db.shape) #max value row index
                    
            msl_line = bf_db[max_idx[0],:]
            
            y2 = msl_line
            line2.set_ydata(y2)

            ymax2 = y2.max()
            ymin2 = ymax2 - 30 #y2.min()
            
            ax2.set_ylim(ymin2, ymax2 + 0)       
            ax2.grid(True)  
            ax2.set_title('MSL min max diff = %.2f, min = %.2f max = %.2f'%(bf_diff, bf_min, bf_max))

            ax2.draw_artist(ax2.patch)
            ax2.draw_artist(line2)   
            
            
        
            #-------------------------------------------------------------        
            # mic rms
            
            # find mic error index
            rms_th = rms_data.mean() * 0.1
            mic_low = np.where(rms_data < rms_th)[0]
            
            rms_db = GLOB.SPL_MAX + 10 * np.log10(rms_data * GLOB.POW_RMS_CONSTANT + 0.0000000000000001)  #prevent 0 dB error
            
            rms_max = rms_db.max()
            rms_min = rms_db.min()
            rms_av = rms_db.mean()
            rms_diff = rms_max - rms_min
            # ~ print(rms_db[:20])
            print('Mic rms max[%.2f] min[%.2f] av[%.2f]\nlow power mic idx %s'%(rms_max, rms_min, rms_av, mic_low))
            
            ax3.set_title('Mic rms max[%.2f] min[%.2f] av[%.2f]\nlow power mic idx %s'%(rms_max, rms_min, rms_av, mic_low))

            
            y3 = copy.deepcopy(rms_db)
            line3.set_ydata(y3)
            
            
            ymin3, ymax3 = y3.min(), y3.max()
            ymin3 = 0
            yspan3 = ymax3 - ymin3
            ymargin3 = yspan3 * 0.1
            ax3.set_ylim(ymin3 - ymargin3, ymax3 + ymargin3) 

            ax3.draw_artist(ax3.patch)
            ax3.draw_artist(line3)   

            fig.canvas.blit(ax1.bbox)
            fig.canvas.blit(ax2.bbox)
            fig.canvas.blit(ax3.bbox)
            
            #fig.canvas.blit(fig.bbox)
            fig.canvas.flush_events()                      
            # ~ print(gv.rms_data)


            plt.pause(0.1)

            lock.release()

        if GLOB.CMD_TYPE == 2:

            await lock.acquire()


            fig.canvas.restore_region(bg1)
            fig.canvas.restore_region(bg2)

            y1 = mic_data[:GLOB.VLEN]
            y2 = bf_mic_data[:GLOB.VLEN]
            y2 = y2 * 2**7 / 112
            line1.set_ydata(y1)
            line2.set_ydata(y2)
            
            
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

            ax1.draw_artist(ax1.patch)
            ax1.draw_artist(line1)
            ax2.draw_artist(ax2.patch)
            ax2.draw_artist(line2)      
            #----------------------------------
            # refresh      
            fig.canvas.blit(ax1.bbox)
            fig.canvas.blit(ax2.bbox)
            fig.canvas.flush_events()


            plt.pause(0.1)

            lock.release()


    return


async def async_main():

    lock = asyncio.Lock()

    res = await asyncio.gather(task(lock), printer(lock))

    return res


res = asyncio.run(async_main())

print("DONE.")



