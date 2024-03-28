
import numpy as np
import pygame
import posix_ipc

interp = 8
kron_kernel = np.ones((interp, interp), dtype = np.uint8)
image = np.zeros((40 * interp, 30 * interp, 3), dtype = np.uint8)
avg_frame = 4
drange = 15.0
max_sig_range = 256
bf_accum = np.zeros((1200,), dtype = np.float32)
sig_concat = np.zeros((960*interp,), dtype = np.int16)

def bf2image(bf_data):
    bf_scale = np.reshape(bf_data, (30, 40))
    max_index = np.unravel_index(bf_scale.argmax(), bf_scale.shape)
    #print(max_index)
    bf_scale = ((bf_scale + drange) / drange * 255).astype(np.uint8)
    bf_interp = np.kron(bf_scale.transpose(), kron_kernel)
    image[:, :, 0] = bf_interp
    image[:, :, 1] = bf_interp
    image[:, :, 2] = bf_interp

def sig2waveform(sig_data):
    scale = (10 * interp) / max_sig_range
    width_per_sample = 2
    show_len = 40 * interp // width_per_sample
    sig_scale = np.int32(np.float32(sig_data[0:show_len]) * scale) + 10 * interp
    return ( (i * width_per_sample, sig_scale[i]) for i in range(show_len) )

def main():
    pygame.init()
    scrn = pygame.display.set_mode((40 * interp, 50 * interp))
    sig_surf = pygame.Surface((40 * interp, 20 * interp))
    index = 0
    org_index = 1
    global max_sig_range
    print("sig range = {}".format(max_sig_range))
    dt = np.dtype(np.float32).newbyteorder('>')
    dt2 = np.dtype(np.int16).newbyteorder('>')
    bf_mq = posix_ipc.MessageQueue("/bf_queue", posix_ipc.O_RDONLY, read = True, write = False)
    sig_mq = posix_ipc.MessageQueue("/sig_queue", posix_ipc.O_RDONLY, read = True, write = False)
    status = True
    while status:
        index += 1
        bf_data, _ = bf_mq.receive()
        sig_data, _ = sig_mq.receive()
        bf_np = np.frombuffer(bf_data, dtype = dt)
        sig_np = np.frombuffer(sig_data, dtype = dt2)
        sig1 = sig_np.reshape((-1, 2))[:, org_index]
        bf_max = np.max(bf_np)
        sig_max = np.max(sig1)
        #print("max = {}, sigmax = {}".format(bf_max, sig_max))
        if index % avg_frame == 1:
            bf_accum = np.zeros((1200,), dtype = np.float32)

        bf_accum = bf_accum + bf_np
        sig_concat[(index % avg_frame) * 960 : (index % avg_frame + 1) * 960] = sig1

        if index % avg_frame == 0:
            bf_log = 10 * np.log10(bf_accum)
            bf_normal = bf_log - np.max(bf_log)
            bf_normal = np.where(bf_normal > -drange, bf_normal, -drange)
            bf2image(bf_normal)
            surf = pygame.surfarray.make_surface(image)
            points = tuple(sig2waveform(sig_concat))
            sig_surf.fill((0, 0, 0))
            pygame.draw.lines(sig_surf, (255, 255, 255), False, points)
            scrn.blit(surf, (0, 0))
            scrn.blit(sig_surf, (0, 30 * interp))
            pygame.display.update()

        for i in pygame.event.get():
            if i.type == pygame.QUIT:
                status = False
            if i.type == pygame.KEYDOWN:
                if i.key == pygame.K_SPACE:
                    if org_index == 0:
                        org_index = 1
                    else:
                        org_index = 0
                if i.key == pygame.K_UP:
                    if max_sig_range < 32767:
                        max_sig_range = max_sig_range * 2
                    print("sig range = {}".format(max_sig_range))
                if i.key == pygame.K_DOWN:
                    if max_sig_range > 255:
                        max_sig_range = max_sig_range // 2
                    print("sig range = {}".format(max_sig_range))
                if i.key == pygame.K_ESCAPE:
                    status = False

    bf_mq.close()
    sig_mq.close()

    pygame.quit()

if __name__ == '__main__':
    main()
