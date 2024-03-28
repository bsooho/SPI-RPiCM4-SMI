
#include "dma/dma.h"

// Start Variables initialization


char *spi_regstrs[] = {"CS", "FIFO", "CLK", "DLEN", "LTOH", "DC", ""};


char *dma_regstrs[] = {"DMA CS", "CB_AD", "TI", "SRCE_AD", "DEST_AD",
    "TFR_LEN", "STRIDE", "NEXT_CB", "DEBUG", ""};

uint32_t usec_start;

char stream_buff[STREAM_BUFFLEN];
uint32_t rx_buff[MAX_SAMPS];

// Virtual memory pointers to acceess GPIO, DMA and PWM from user space
MEM_MAP gpio_regs, dma_regs, clk_regs, pwm_regs;
MEM_MAP vc_mem, spi_regs, usec_regs;
MEM_MAP pwm_regs, gpio_regs, dma_regs, clk_regs;

int fifo_fd;


int in_chans=1, sample_count=0, sample_rate=SAMPLE_RATE;
int data_format, testmode, verbose, lockstep;
uint32_t pwm_range, samp_total, overrun_total, fifo_size;
char *fifo_name;


// End variables initialization


// Catastrophic failure in initial setup
void fail(char *s)
{
    printf(s);
    terminate(0);
}

// Use mmap to obtain virtual address, given physical
void *map_periph(MEM_MAP *mp, void *phys, int size)
{
    mp->phys = phys;
    mp->size = PAGE_ROUNDUP(size);
    mp->bus = (uint8_t *)phys - (uint8_t *)PHYS_REG_BASE + (uint8_t *)BUS_REG_BASE;
    mp->virt = map_segment(phys, mp->size);
    return(mp->virt);
}

// Allocate uncached memory, get bus & phys addresses
void *map_uncached_mem(MEM_MAP *mp, int size)
{
    void *ret;
    mp->size = PAGE_ROUNDUP(size);
    mp->fd = open_mbox();
    ret = (mp->h = alloc_vc_mem(mp->fd, mp->size, DMA_MEM_FLAGS)) > 0 &&
        (mp->bus = lock_vc_mem(mp->fd, mp->h)) != 0 &&
        (mp->virt = map_segment(BUS_PHYS_ADDR(mp->bus), mp->size)) != 0
        ? mp->virt : 0;
    printf("VC mem handle %u, phys %p, virt %p\n", mp->h, mp->bus, mp->virt);
    return(ret);
}

// Free mapped peripheral or memory
void unmap_periph_mem(MEM_MAP *mp)
{
    if (mp)
    {
        if (mp->fd)
        {
            unmap_segment(mp->virt, mp->size);
            unlock_vc_mem(mp->fd, mp->h);
            free_vc_mem(mp->fd, mp->h);
            close_mbox(mp->fd);
        }
        else
            unmap_segment(mp->virt, mp->size);
    }
}

// ----- GPIO -----

// Set input or output with pullups
void gpio_set(int pin, int mode, int pull)
{
    gpio_mode(pin, mode);
    gpio_pull(pin, pull);
}
// Set I/P pullup or pulldown
void gpio_pull(int pin, int pull)
{
    volatile uint32_t *reg = REG32(gpio_regs, GPIO_GPPUDCLK0) + pin / 32;

    *REG32(gpio_regs, GPIO_GPPUD) = pull;
    usleep(2);
    *reg = pin << (pin % 32);
    usleep(2);
    *REG32(gpio_regs, GPIO_GPPUD) = 0;
    *reg = 0;
}

// Set input or output
void gpio_mode(int pin, int mode)
{
    volatile uint32_t *reg = REG32(gpio_regs, GPIO_MODE0) + pin / 10, shift = (pin % 10) * 3;
    *reg = (*reg & ~(7 << shift)) | (mode << shift);
}

// Set an O/P pin
void gpio_out(int pin, int val)
{
    volatile uint32_t *reg = REG32(gpio_regs, val ? GPIO_SET0 : GPIO_CLR0) + pin/32;
    *reg = 1 << (pin % 32);
}

// Get an I/P pin value
uint8_t gpio_in(int pin)
{
    volatile uint32_t *reg = REG32(gpio_regs, GPIO_LEV0) + pin/32;
    return (((*reg) >> (pin % 32)) & 1);
}

// ----- VIDEOCORE MAILBOX -----

// Open mailbox interface, return file descriptor
int open_mbox(void)
{
   int fd;

   if ((fd = open("/dev/vcio", 0)) < 0)
       fail("Error: can't open VC mailbox\n");
   return(fd);
}
// Close mailbox interface
void close_mbox(int fd)
{
    if (fd >= 0)
        close(fd);
}

// Send message to mailbox, return first response int, 0 if error
uint32_t msg_mbox(int fd, VC_MSG *msgp)
{
    uint32_t ret=0, i;

    for (i=msgp->dlen/4; i<=msgp->blen/4; i+=4)
        msgp->uints[i++] = 0;
    msgp->len = (msgp->blen + 6) * 4;
    msgp->req = 0;
    if (ioctl(fd, _IOWR(100, 0, void *), msgp) < 0)
        printf("VC IOCTL failed\n");
    else if ((msgp->req&0x80000000) == 0)
        printf("VC IOCTL error\n");
    else if (msgp->req == 0x80000001)
        printf("VC IOCTL partial error\n");
    else
        ret = msgp->uints[0];
#if DEBUG
    disp_vc_msg(msgp);
#endif
    return(ret);
}

// Allocate memory on PAGE_SIZE boundary, return handle
uint32_t alloc_vc_mem(int fd, uint32_t size, VC_ALLOC_FLAGS flags)
{
    VC_MSG msg={.tag=0x3000c, .blen=12, .dlen=12,
        .uints={PAGE_ROUNDUP(size), PAGE_SIZE, flags}};
    return(msg_mbox(fd, &msg));
}
// Lock allocated memory, return bus address
void *lock_vc_mem(int fd, int h)
{
    VC_MSG msg={.tag=0x3000d, .blen=4, .dlen=4, .uints={h}};
    return(h ? (void *)msg_mbox(fd, &msg) : 0);
}
// Unlock allocated memory
uint32_t unlock_vc_mem(int fd, int h)
{
    VC_MSG msg={.tag=0x3000e, .blen=4, .dlen=4, .uints={h}};
    return(h ? msg_mbox(fd, &msg) : 0);
}
// Free memory
uint32_t free_vc_mem(int fd, int h)
{
    VC_MSG msg={.tag=0x3000f, .blen=4, .dlen=4, .uints={h}};
    return(h ? msg_mbox(fd, &msg) : 0);
}
uint32_t set_vc_clock(int fd, int id, uint32_t freq)
{
    VC_MSG msg1={.tag=0x38001, .blen=8, .dlen=8, .uints={id, 1}};
    VC_MSG msg2={.tag=0x38002, .blen=12, .dlen=12, .uints={id, freq, 0}};
    msg_mbox(fd, &msg1);
    disp_vc_msg(&msg1);
    msg_mbox(fd, &msg2);
    disp_vc_msg(&msg2);
    return(0);
}

// Display mailbox message
void disp_vc_msg(VC_MSG *msgp)
{
    int i;

    printf("VC msg len=%X, req=%X, tag=%X, blen=%x, dlen=%x, data ",
        msgp->len, msgp->req, msgp->tag, msgp->blen, msgp->dlen);
    for (i=0; i<msgp->blen/4; i++)
        printf("%08X ", msgp->uints[i]);
    printf("\n");
}

// ----- VIRTUAL MEMORY -----

// Get virtual memory segment for peripheral regs or physical mem
void *map_segment(void *addr, int size)
{
    int fd;
    void *mem;

    size = PAGE_ROUNDUP(size);
    if ((fd = open ("/dev/mem", O_RDWR|O_SYNC|O_CLOEXEC)) < 0)
        fail("Error: can't open /dev/mem, run using sudo\n");
    mem = mmap(0, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, (uint32_t)addr);
    close(fd);
#if DEBUG
    printf("Map %p -> %p\n", (void *)addr, mem);
#endif
    if (mem == MAP_FAILED)
        fail("Error: can't map memory\n");
    return(mem);
}
// Free mapped memory
void unmap_segment(void *mem, int size)
{
    if (mem)
        munmap(mem, PAGE_ROUNDUP(size));
}

// ----- DMA -----

// Enable and reset DMA
void enable_dma(int chan)
{
    *REG32(dma_regs, DMA_ENABLE) |= (1 << chan);
    *REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 1 << 31;
}

// Start DMA, given first control block
void start_dma(MEM_MAP *mp, int chan, DMA_CB *cbp, uint32_t csval)
{
    *REG32(dma_regs, DMA_REG(chan, DMA_CONBLK_AD)) = MEM_BUS_ADDR(mp, cbp);
    *REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 2;        // Clear 'end' flag
    *REG32(dma_regs, DMA_REG(chan, DMA_DEBUG)) = 7;     // Clear error bits
    *REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 1|csval;  // Start DMA
}

// Return remaining transfer length
uint32_t dma_transfer_len(int chan)
{
    return(*REG32(dma_regs, DMA_REG(chan, DMA_TXFR_LEN)));
}

// Halt current DMA operation by resetting controller
void stop_dma(int chan)
{
    if (dma_regs.virt)
        *REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 1 << 31;
}

// Display DMA registers
void disp_dma(int chan)
{
    volatile uint32_t *p=REG32(dma_regs, DMA_REG(chan, DMA_CS));
    int i=0;

    while (dma_regstrs[i][0])
    {
        printf("%-7s %08X ", dma_regstrs[i++], *p++);
        if (i%5==0 || dma_regstrs[i][0]==0)
            printf("\n");
    }
}

// ----- PWM -----

// Initialise PWM
void init_pwm(int freq, int range, int val)
{
    stop_pwm();
    if (*REG32(pwm_regs, PWM_STA) & 0x100)
    {
        printf("PWM bus error\n");
        *REG32(pwm_regs, PWM_STA) = 0x100;
    }
#if USE_VC_CLOCK_SET
    set_vc_clock(mbox_fd, PWM_CLOCK_ID, freq);
#else
    int divi=CLOCK_HZ / freq;
    *REG32(clk_regs, CLK_PWM_CTL) = CLK_PASSWD | (1 << 5);
    while (*REG32(clk_regs, CLK_PWM_CTL) & (1 << 7)) ;
    *REG32(clk_regs, CLK_PWM_DIV) = CLK_PASSWD | (divi << 12);
    *REG32(clk_regs, CLK_PWM_CTL) = CLK_PASSWD | 6 | (1 << 4);
    while ((*REG32(clk_regs, CLK_PWM_CTL) & (1 << 7)) == 0) ;
#endif
    usleep(100);
    *REG32(pwm_regs, PWM_RNG1) = range;
    *REG32(pwm_regs, PWM_FIF1) = val;
    usleep(100);
#if PWM_OUT
    gpio_mode(PWM_PIN, GPIO_ALT5);
#endif
}

// Start PWM operation
void start_pwm(void)
{
    *REG32(pwm_regs, PWM_CTL) = PWM_CTL_USEF1 | PWM_ENAB;
}

// Stop PWM operation
void stop_pwm(void)
{
    if (pwm_regs.virt)
    {
        *REG32(pwm_regs, PWM_CTL) = 0;
        usleep(100);
    }
}




// Free memory & peripheral mapping and exit
void terminate(int sig)
{
    printf("Closing\n");
    spi_disable();
    stop_dma(DMA_CHAN_A);
    stop_dma(DMA_CHAN_B);
    stop_dma(DMA_CHAN_C);
    unmap_periph_mem(&vc_mem);
    unmap_periph_mem(&usec_regs);
    unmap_periph_mem(&pwm_regs);
    unmap_periph_mem(&clk_regs);
    unmap_periph_mem(&spi_regs);
    unmap_periph_mem(&dma_regs);
    unmap_periph_mem(&gpio_regs);
    if (fifo_name)
        destroy_fifo(fifo_name, fifo_fd);
    if (samp_total)
        printf("Total samples %u, overruns %u\n", samp_total, overrun_total);
    exit(0);
}

// Map GPIO, DMA and SPI registers into virtual mem (user space)
// If any of these fail, program will be terminated
void map_devices(void)
{
    map_periph(&gpio_regs, (void *)GPIO_BASE, PAGE_SIZE);
    map_periph(&dma_regs, (void *)DMA_BASE, PAGE_SIZE);
    map_periph(&spi_regs, (void *)SPI0_BASE, PAGE_SIZE);
    map_periph(&clk_regs, (void *)CLK_BASE, PAGE_SIZE);
    map_periph(&pwm_regs, (void *)PWM_BASE, PAGE_SIZE);
    map_periph(&usec_regs, (void *)USEC_BASE, PAGE_SIZE);
}

// Get uncached memory
 void get_uncached_mem(MEM_MAP *mp, int size)
{
    if (!map_uncached_mem(mp, size))
        fail("Error: can't allocate uncached memory\n");
}

// Fetch single sample from ADC channel 0 or 1
int adc_get_sample(int chan)
{
    uint8_t txdata[ADC_RAW_LEN] = ADC_REQUEST(chan > 0);
    uint8_t rxdata[ADC_RAW_LEN];

    spi_cs(1);
    spi_xfer(txdata, rxdata, sizeof(txdata));
    spi_cs(0);
    if (verbose)
    {
        for (int i=0; i<ADC_RAW_LEN; i++)
            printf("%02X ", rxdata[i]);
        printf("\n");
    }
    return(ADC_RAW_VAL(*(uint16_t *)rxdata));
}


// Test SPI frequency
float test_spi_frequency(MEM_MAP *mp)
{
    TEST_DMA_DATA *dp=mp->virt;
    TEST_DMA_DATA dma_data = {
        .txd = {0,0,0,0,0,0,0,0,0,0}, .usecs = {0, 0},
        .cbs = {
        // Tx output: 2 initial transfers, then 10 timed transfers
            {SPI_TEST_TI, MEM(mp, dp->txd), REG(spi_regs, SPI_FIFO),           2*4, 0, CBS(1), 0}, // 0
            {SPI_TEST_TI, REG(usec_regs, USEC_TIME), MEM(mp, &dp->usecs[0]),     4, 0, CBS(2), 0}, // 1
            {SPI_TEST_TI, MEM(mp, dp->txd), REG(spi_regs, SPI_FIFO), TEST_NSAMPS*4, 0, CBS(3), 0}, // 2
            {SPI_TEST_TI, REG(usec_regs, USEC_TIME), MEM(mp, &dp->usecs[1]),     4, 0, 0,      0}, // 3
        }
    };
    memcpy(dp, &dma_data, sizeof(dma_data));                // Copy DMA data into uncached memory
    *REG32(spi_regs, SPI_DC) = (8<<24)|(1<<16)|(8<<8)|1;    // Set DMA priorities
    *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR;                // Clear SPI FIFOs
    start_dma(mp, DMA_CHAN_A, &dp->cbs[0], 0);              // Start SPI Tx DMA
    *REG32(spi_regs, SPI_DLEN) = (TEST_NSAMPS + 2) * 4;     // Set data length, and SPI flags
    *REG32(spi_regs, SPI_CS) = SPI_TFR_ACT | SPI_DMA_EN;
    dma_wait(DMA_CHAN_A);                                   // Wait until complete
    *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR;                // Clear accumulated Rx data
    return(dp->usecs[1] > dp->usecs[0] ?
           32.0 * TEST_NSAMPS / (dp->usecs[1] - dp->usecs[0]) : 0);
}

// Test PWM frequency
float test_pwm_frequency(MEM_MAP *mp)
{
    TEST_DMA_DATA *dp=mp->virt;
    TEST_DMA_DATA dma_data = {
        .val = pwm_range, .usecs = {0, 0},
        .cbs = {
        // Tx output: 2 initial transfers, then timed transfer
            {PWM_TI, MEM(mp, &dp->val),         REG(pwm_regs, PWM_FIF1), 4, 0, CBS(1), 0}, // 0
            {PWM_TI, MEM(mp, &dp->val),         REG(pwm_regs, PWM_FIF1), 4, 0, CBS(2), 0}, // 1
            {PWM_TI, REG(usec_regs, USEC_TIME), MEM(mp, &dp->usecs[0]),  4, 0, CBS(3), 0}, // 2
            {PWM_TI, MEM(mp, &dp->val),         REG(pwm_regs, PWM_FIF1), 4, 0, CBS(4), 0}, // 3
            {PWM_TI, REG(usec_regs, USEC_TIME), MEM(mp, &dp->usecs[1]),  4, 0, 0,      0}, // 4
        }
    };
    memcpy(dp, &dma_data, sizeof(dma_data));                // Copy DMA data into uncached memory
    *REG32(spi_regs, SPI_DC) = (8<<24)|(1<<16)|(8<<8)|1;    // Set DMA priorities
    init_pwm(PWM_FREQ, pwm_range, PWM_VALUE);               // Initialise PWM
    *REG32(pwm_regs, PWM_DMAC) = PWM_DMAC_ENAB | PWM_ENAB;  // Enable PWM DMA
    start_dma(mp, DMA_CHAN_A, &dp->cbs[0], 0);              // Start DMA
    start_pwm();                                            // Start PWM
    dma_wait(DMA_CHAN_A);                                   // Wait until complete
    stop_pwm();                                             // Stop PWM
    return(dp->usecs[1] > dp->usecs[0] ? 1e6 / (dp->usecs[1] - dp->usecs[0]) : 0);
}


// Initialise PWM-paced DMA for ADC sampling
void adc_dma_init(MEM_MAP *mp, int nsamp, int single)
{
    ADC_DMA_DATA *dp=mp->virt;
    ADC_DMA_DATA dma_data = {
        .samp_size = 2, .pwm_val = pwm_range, .txd={0xd0, in_chans>1 ? 0xf0 : 0xd0},
        .adc_csd = SPI_TFR_ACT | SPI_AUTO_CS | SPI_DMA_EN | SPI_FIFO_CLR | ADC_CE_NUM,
        .usecs = {0, 0}, .states = {0, 0}, .rxd1 = {0}, .rxd2 = {0},
        .cbs = {
        // Rx input: read data from usec clock and SPI, into 2 ping-pong buffers
            {SPI_RX_TI, REG(usec_regs, USEC_TIME), MEM(mp, &dp->usecs[0]),  4, 0, CBS(1), 0}, // 0
            {SPI_RX_TI, REG(spi_regs, SPI_FIFO),   MEM(mp, dp->rxd1), nsamp*4, 0, CBS(2), 0}, // 1
            {SPI_RX_TI, REG(spi_regs, SPI_CS),     MEM(mp, &dp->states[0]), 4, 0, CBS(3), 0}, // 2
            {SPI_RX_TI, REG(usec_regs, USEC_TIME), MEM(mp, &dp->usecs[1]),  4, 0, CBS(4), 0}, // 3
            {SPI_RX_TI, REG(spi_regs, SPI_FIFO),   MEM(mp, dp->rxd2), nsamp*4, 0, CBS(5), 0}, // 4
            {SPI_RX_TI, REG(spi_regs, SPI_CS),     MEM(mp, &dp->states[1]), 4, 0, CBS(0), 0}, // 5
        // Tx output: 2 data writes to SPI for chan 0 & 1, or both chan 0
            {SPI_TX_TI, MEM(mp, dp->txd),          REG(spi_regs, SPI_FIFO), 8, 0, CBS(6), 0}, // 6
        // PWM ADC trigger: wait for PWM, set sample length, trigger SPI
            {PWM_TI,    MEM(mp, &dp->pwm_val),     REG(pwm_regs, PWM_FIF1), 4, 0, CBS(8), 0}, // 7
            {PWM_TI,    MEM(mp, &dp->samp_size),   REG(spi_regs, SPI_DLEN), 4, 0, CBS(9), 0}, // 8
            {PWM_TI,    MEM(mp, &dp->adc_csd),     REG(spi_regs, SPI_CS),   4, 0, CBS(7), 0}, // 9
        }
    };
    if (single)                                 // If single-shot, stop after first Rx block
        dma_data.cbs[2].next_cb = 0;
    memcpy(dp, &dma_data, sizeof(dma_data));    // Copy DMA data into uncached memory
    init_pwm(PWM_FREQ, pwm_range, PWM_VALUE);   // Initialise PWM, with DMA
    *REG32(pwm_regs, PWM_DMAC) = PWM_DMAC_ENAB | PWM_ENAB;
    *REG32(spi_regs, SPI_DC) = (8<<24) | (1<<16) | (8<<8) | 1;  // Set DMA priorities
    *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR;                    // Clear SPI FIFOs
    start_dma(mp, DMA_CHAN_C, &dp->cbs[6], 0);  // Start SPI Tx DMA
    start_dma(mp, DMA_CHAN_B, &dp->cbs[0], 0);  // Start SPI Rx DMA
    start_dma(mp, DMA_CHAN_A, &dp->cbs[7], 0);  // Start PWM DMA, for SPI trigger
}

// Manage streaming output
void do_streaming(MEM_MAP *mp, char *vals, int maxlen, int nsamp)
{
    int n;

    if (!fifo_fd)
    {
        if ((fifo_fd = open_fifo_write(fifo_name)) > 0)
        {
            printf("Started streaming to FIFO '%s'\n", fifo_name);
            fifo_size = fifo_freespace(fifo_fd);
        }
    }
    if (fifo_fd)
    {
        if ((n=adc_stream_csv(mp, vals, maxlen, nsamp)) > 0)
        {
            if (!write_fifo(fifo_fd, vals, n))
            {
                printf("Stopped streaming\n");
                close(fifo_fd);
                fifo_fd = 0;
                usleep(100000);
            }
        }
        else
            usleep(10);
    }
}

// Start ADC data acquisition
void adc_stream_start(void)
{
    start_pwm();
}

// Wait until a (single) DMA cycle is complete
void adc_stream_wait(void)
{
    dma_wait(DMA_CHAN_B);
}

// Stop ADC data acquisition
void adc_stream_stop(void)
{
    stop_pwm();
}

// Fetch samples from ADC buffer, return comma-delimited integer values
// If in lockstep mode, discard new data if FIFO isn't empty
int adc_stream_csv(MEM_MAP *mp, char *vals, int maxlen, int nsamp)
{
    ADC_DMA_DATA *dp=mp->virt;
    uint32_t i, n, usec, slen=0;
    for (n=0; n<2 && slen==0; n++)
    {
        if (dp->states[n])
        {
            samp_total += nsamp;
            memcpy(rx_buff, n ? (void *)dp->rxd2 : (void *)dp->rxd1, nsamp*4);
            usec = dp->usecs[n];
            if (dp->states[n^1])
            {
                dp->states[0] = dp->states[1] = 0;
                overrun_total++;
                break;
            }
            dp->states[n] = 0;
            if (usec_start == 0)
                usec_start = usec;
            if (!lockstep || fifo_freespace(fifo_fd)>=fifo_size)
            {
                if (data_format == FMT_USEC)
                    slen += sprintf(&vals[slen], "%u", usec-usec_start);
                for (i=0; i<nsamp && slen+20<maxlen; i++)
                    slen += sprintf(&vals[slen], "%s%4.3f", slen ? "," : "",
                        ADC_VOLTAGE(ADC_RAW_VAL(rx_buff[i])));
                slen += sprintf(&vals[slen], "\n");
                if (verbose)
                {
                    for (int i=0; i<nsamp*4; i++)
                        printf("%02X ", *(((uint8_t *)rx_buff)+i));
                    printf("\n");
                }
            }
        }
    }
    vals[slen] = 0;
    return(slen);
}

// Test of SPI write cycles
// Redundant code, kept in as an explanation of SPI data length
int spi_tx_test(MEM_MAP *mp, uint16_t *buff, int nsamp)
{
    uint32_t n, a=0;

    nsamp = 8;
    *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR;
#if 1
    // Write data length to DLEN reg (with ACT clear)
    for (n=0; n<nsamp; n++)
    {
        *REG32(spi_regs, SPI_DLEN) = 2;
        *REG32(spi_regs, SPI_CS) = SPI_TFR_ACT | SPI_AUTO_CS | SPI_DMA_EN | SPI_FIFO_CLR;
        *REG32(spi_regs, SPI_FIFO) = n;
        usleep(5);
        a += *REG32(spi_regs, SPI_FIFO);
        // *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR; // Not needed, as ACT is already clear
    }
#else
    // Write data length to FIFO (with ACT set)
    *REG32(spi_regs, SPI_CS) = SPI_TFR_ACT | SPI_AUTO_CS | SPI_DMA_EN | SPI_FIFO_CLR;
    for (n=0; n<nsamp; n++)
    {
        *REG32(spi_regs, SPI_FIFO) = (2<<16) | SPI_TFR_ACT | SPI_FIFO_CLR;
        *REG32(spi_regs, SPI_FIFO) = n;
        usleep(5);
        a += *REG32(spi_regs, SPI_FIFO);
        // *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR; // Clearing ACT would disrupt comms
    }
#endif
    return(0);
}

// Wait until DMA is complete
void dma_wait(int chan)
{
    int n = 10000;

    do {
        usleep(10);
    } while (dma_transfer_len(chan) && --n);
    if (n == 0)
        printf("DMA transfer timeout\n");
}

// Create a FIFO (named pipe)
int create_fifo(char *fname)
{
    int ok=0;

    umask(0);
    if (mkfifo(fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0 && errno != EEXIST)
        printf("Can't open FIFO '%s'\n", fname);
    else
        ok = 1;
    return(ok);
}

// Open a FIFO for writing, return 0 if there is no reader
int open_fifo_write(char *fname)
{
    int f = open(fname, O_WRONLY | O_NONBLOCK);

    return(f == -1 ? 0 : f);
}

// Write to FIFO, return 0 if no reader
int write_fifo(int fd, void *data, int dlen)
{
    struct pollfd pollfd = {fd, POLLOUT, 0};

    poll(&pollfd, 1, 0);
    if (pollfd.revents&POLLOUT && !(pollfd.revents&POLLERR))
        return(fd ? write(fd, data, dlen) : 0);
    return(0);
}

// Return the free space in FIFO
uint32_t fifo_freespace(int fd)
{
    return(fcntl(fd, F_GETPIPE_SZ));
}

// Remove the FIFO
void destroy_fifo(char *fname, int fd)
{
    if (fd > 0)
        close(fd);
    unlink(fname);
}

// Initialise SPI0, given desired clock freq; return actual value
int init_spi(int hz)
{
    int f, div = (SPI_CLOCK_HZ / hz + 1) & ~1;

    gpio_set(SPI0_CE0_PIN, GPIO_ALT0, GPIO_NOPULL);
    gpio_set(SPI0_CE1_PIN, GPIO_ALT0, GPIO_NOPULL);
    gpio_set(SPI0_MISO_PIN, GPIO_ALT0, GPIO_PULLUP);
    gpio_set(SPI0_MOSI_PIN, GPIO_ALT0, GPIO_NOPULL);
    gpio_set(SPI0_SCLK_PIN, GPIO_ALT0, GPIO_NOPULL);
    while (div==0 || (f = SPI_CLOCK_HZ/div) > MAX_SPI_FREQ)
        div += 2;
    *REG32(spi_regs, SPI_CS) = 0x30;
    *REG32(spi_regs, SPI_CLK) = div;
    return(f);
}

// Clear SPI FIFOs
void spi_clear(void)
{
    *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR;
}

// Set / clear SPI chip select
void spi_cs(int set)
{
    uint32_t csval = *REG32(spi_regs, SPI_CS);

    *REG32(spi_regs, SPI_CS) = set ? csval | 0x80 : csval & ~0x80;
}

// Transfer SPI bytes
void spi_xfer(uint8_t *txd, uint8_t *rxd, int len)
{
    while (len--)
    {
        *REG8(spi_regs, SPI_FIFO) = *txd++;
        while((*REG32(spi_regs, SPI_CS) & (1<<17)) == 0) ;
        *rxd++ = *REG32(spi_regs, SPI_FIFO);
    }
}

// Disable SPI
void spi_disable(void)
{
    *REG32(spi_regs, SPI_CS) = SPI_FIFO_CLR;
    *REG32(spi_regs, SPI_CS) = 0;
}

// Display SPI registers
void disp_spi(void)
{
    volatile uint32_t *p=REG32(spi_regs, SPI_CS);
    int i=0;

    while (spi_regstrs[i][0])
        printf("%-4s %08X ", spi_regstrs[i++], *p++);
    printf("\n");
}