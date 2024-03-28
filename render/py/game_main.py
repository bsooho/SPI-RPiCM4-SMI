import GLOB
import socket
import time
import struct
import copy


import numpy as np


import datetime


import pygame



pygame.init()


socket_path = "/tmp/fpga_stream_export.sock"


client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

client.connect(socket_path)


print("connected to export socket")


KEEP = True


# render_plot = render.init_plot()

fig_interp = 80
fig = None
interp = 8
kron_kernel = np.ones((interp, interp), dtype = np.uint8)
drange = 15.0
image = np.zeros((30 * interp, 40 * interp), dtype = np.uint8)

sig_surf1 = None
sig_surf2 = None
max_sig_range = 255
scale = 5
width_per_sample = 10

bf_data = np.zeros([30,40])
rms_data = np.zeros(112)
mic_data = np.zeros(8000)
bf_mic_data = np.zeros(8000)




if GLOB.CMD_TYPE == 1:

    fig = pygame.display.set_mode((6 * fig_interp, 10 * fig_interp))
    sig_surf1 = pygame.Surface((6* fig_interp, 2 * fig_interp))
    sig_surf2 = pygame.Surface((6* fig_interp, 2 * fig_interp))

elif GLOB.CMD_TYPE == 2:

    fig = pygame.display.set_mode((6 * fig_interp, 10 * fig_interp))
    sig_surf1 = pygame.Surface((6* fig_interp, 2 * fig_interp))
    sig_surf2 = pygame.Surface((6* fig_interp, 2 * fig_interp))


print("initiated plot rendering")



while KEEP == True:

    client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    client.connect(socket_path)


    print("connected to export socket")


    while KEEP == True:


        if GLOB.CMD_TYPE == 1:


            message = "1"

            client.sendall(message.encode())

            flag_set_byte = client.recv(GLOB.FLAG_SET_BYTE)

            flag_set_arr = [ x for x in flag_set_byte ]

            print(flag_set_arr)

            response = client.recv(GLOB.CMD_TYPE_1_BYTE_LEN)

            print(len(response))

            if flag_set_arr[0] != 1 :

                print("invalid flag")

                continue

            # render_plot = render.cmd_type_1(response, render_plot)


            print("cmd_type_1")


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
            


            print(len(data_raw))

            i = 0

            for x in range(GLOB.BF_DATA_X):

                for y in range(GLOB.BF_DATA_Y):

                    bf_data[x][y] = data_raw[i]

                    i += 1       

            

            for x in range(GLOB.RMS_DATA):

                rms_data[x] = data_raw[i]

                i += 1


            
            
            bf_db = GLOB.SPL_MAX + 10 * np.log10(bf_data * GLOB.POW_CONSTANT)
            # ~ print(bf_db.max())
            
            bf_max = bf_db.max()
            bf_min = bf_db.min()
            bf_diff = bf_max - bf_min
            print('min max diff = %f, min = %f max = %f'%(bf_diff, bf_min, bf_max))        

            #bf_db = bf_db - bf_min + 0.0001
            # bf_db = bf_db / bf_db.max() #* 250
            bf_scale = ((bf_db + drange) / drange * 255).astype(np.uint8)
            bf_interp = np.kron(bf_scale, kron_kernel)

            image[:, :] = bf_interp
            #image[:, :, 1] = bf_interp
            #image[:, :, 2] = bf_interp


            surf = pygame.surfarray.make_surface(image)


            fig.blit(surf, (0, 0))

    
            

            
            #-------------------------------------------------------------
            # mic msl

            max_idx = np.argmax(bf_db)
            max_idx = np.unravel_index(max_idx, bf_db.shape) #max value row index
                    
            msl_line = bf_db[max_idx[0],:]
            
            y2 = msl_line
            

            ymax2 = y2.max()
            ymin2 = ymax2 - 30 #y2.min()



            show_len = len(y2)
            sig_scale = np.int32(np.float32(y2[0:show_len]) * scale) 

            points1 = tuple(( (i * width_per_sample, sig_scale[i]) for i in range(show_len) ))

            sig_surf1.fill((0,0,0))

            pygame.draw.lines(sig_surf1, (255, 255, 0), False, points1)

            fig.blit(sig_surf1, (0, 5 * fig_interp))
         
            #-------------------------------------------------------------        
            # mic rms
            

            rms_th = rms_data.mean() * 0.1
            mic_low = np.where(rms_data < rms_th)[0]
            
            rms_db = GLOB.SPL_MAX + 10 * np.log10(rms_data * GLOB.POW_RMS_CONSTANT + 0.0000000000000001)  #prevent 0 dB error
            
            rms_max = rms_db.max()
            rms_min = rms_db.min()
            rms_av = rms_db.mean()
            rms_diff = rms_max - rms_min
            # ~ print(rms_db[:20])
            print('Mic rms max[%.2f] min[%.2f] av[%.2f]\nlow power mic idx %s'%(rms_max, rms_min, rms_av, mic_low))
            

            y3 = rms_db

            show_len = len(y3)
            sig_scale = np.int32(np.float32(y3[0:show_len]) * scale) 
            points2 = tuple(( (i * width_per_sample, sig_scale[i]) for i in range(show_len) ))

            sig_surf2.fill((0,0,0))

            pygame.draw.lines(sig_surf2, (255, 255, 0), False, points2)

            fig.blit(sig_surf2, (0, 7 * fig_interp))

            pygame.display.update()



        if GLOB.CMD_TYPE == 2:


            message = "2"

            client.sendall(message.encode())

            flag_set_byte = client.recv(GLOB.FLAG_SET_BYTE)

            flag_set_arr = [ x for x in flag_set_byte ]

            print(flag_set_arr)

            response = client.recv(GLOB.CMD_TYPE_2_BYTE_LEN)

            print(len(response))

            if flag_set_arr[1] != 1 :

                print("invalid flag")

                continue

            # render_plot = render.cmd_type_1(response, render_plot)


            print("cmd_type_2")


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
            


            print(len(data_raw))

            i = 0

            for x in range(GLOB.MIC_DATA):

                mic_data[x] = data_raw[i]

                i += 1       

            

            for x in range(GLOB.BF_MIC_DATA):

                bf_mic_data[x] = data_raw[i]

                i += 1



            
            #----------------------------------
            # y axis with margin

            #----------------------------------
            # refresh      
                
            y1 = mic_data[:GLOB.VLEN]
            y2 = bf_mic_data[:GLOB.VLEN]
            y2 = y2 * 2**7 / 112

            show_len = len(y1)
            sig_scale = np.int32(np.float32(y1[0:show_len]) * scale) 
            points1 = tuple(( (i * width_per_sample, sig_scale[i]) for i in range(show_len) ))

            sig_surf1.fill((0,0,0))

            pygame.draw.lines(sig_surf1, (255, 255, 0), False, points1)

            fig.blit(sig_surf1, (0, 1 * fig_interp))

            show_len = len(y2)
            sig_scale = np.int32(np.float32(y2[0:show_len]) * scale) 
            points2 = tuple(( (i * width_per_sample, sig_scale[i]) for i in range(show_len) ))

            sig_surf2.fill((0,0,0))

            pygame.draw.lines(sig_surf2, (255, 255, 0), False, points2)

            fig.blit(sig_surf2, (0, 6 * fig_interp))


            pygame.display.update()


        for i in pygame.event.get():
            if i.type == pygame.QUIT:
                KEEP = False
            if i.type == pygame.KEYDOWN:
                if i.key == pygame.K_ESCAPE:
                    KEEP = False

client.close()
pygame.quit()