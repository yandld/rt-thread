#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "kptl.h"
#include "cm_math.h"

static kptl_t pkt;
static uint8_t tmr_evt;
static rt_sem_t sem;

#define GYR_FRQ         (100)

typedef struct __attribute__((packed))
{
    uint8_t     tag;                /* data packet tag */
    uint8_t     id;
    uint8_t     rev[1];
    int8_t      temp;
    float       prs;
    uint32_t    ts;                 /* timestamp */
    float       acc[3];
    float       gyr[3];
    float       mag[3];
    float       eul[3];             /* eular angles:R/P/Y */
    float       quat[4];            /* quaternion */
} id0x91_t;


static int bin_0x91data(uint8_t *buf, uint8_t id, uint32_t ts, cm_type_t *acc, cm_type_t *gyr, cm_type_t *quat, cm_type_t *mag, cm_type_t pitch, cm_type_t roll, cm_type_t yaw, cm_type_t prs, cm_type_t temperature)
{
    int len;
    id0x91_t *i0x91 = (id0x91_t *)pkt.payload;
    i0x91->tag = 0x91;
    i0x91->acc[0] = acc[0];
    i0x91->acc[1] = acc[1];
    i0x91->acc[2] = acc[2];
    i0x91->gyr[0] = gyr[0];
    i0x91->gyr[1] = gyr[1];
    i0x91->gyr[2] = gyr[2];
    i0x91->mag[0] = mag[0]*50;
    i0x91->mag[1] = mag[1]*50;
    i0x91->mag[2] = mag[2]*50;
    i0x91->eul[0] = pitch;
    i0x91->eul[1] = roll;
    i0x91->eul[2] = yaw;

    i0x91->ts = ts;
    i0x91->temp= temperature;

    i0x91->id = id;

    i0x91->quat[0] = quat[0];
    i0x91->quat[1] = quat[1];
    i0x91->quat[2] = quat[2];
    i0x91->quat[3] = quat[3];

    kptl_frame_packet_begin(&pkt, kFramingPacketType_Data);
    kptl_frame_packet_add(&pkt, pkt.payload, sizeof(id0x91_t));
    kptl_frame_packet_final(&pkt);
    len = kptl_get_frame_size(&pkt);
    memcpy(buf, &pkt, len);
    return len;
}


static void irq_tmr(void *parameter)
{ 
    rt_sem_release(sem);
}

static void thread_sensor_fusion_entry(void *parameter)
{
    uint32_t send_div = 0;
    uint32_t still_cntr = 0;
    uint32_t tx_len;
    rt_device_t console = rt_console_get_device();
    int i;
    static uint8_t tx_buf[256];
    float acc[3] = {0}, gyr[3] = {0}, mag[3] = {0}, eul[3], acc_mean[3] = {0}, gb[3] = {0}, gyr_sum[3] = {0}, gyr_corrected[3] = {0};
    float q[4];
    
    rt_device_t fxos = rt_device_find("fxos8700");
    rt_device_t fxas = rt_device_find("fxas2100");
    
    RT_ASSERT(fxos != RT_NULL);
    RT_ASSERT(fxas != RT_NULL);
    
    if(rt_device_open(fxos, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("cannot open fxos\r\n");
        rt_thread_delete(rt_thread_self());
    }
    
    if(rt_device_open(fxas, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("cannot open fxas\r\n");
        rt_thread_delete(rt_thread_self());
    }
    
    filter_mahony_set_kp_acc(100);
    sem = rt_sem_create("sem", 0, RT_IPC_FLAG_FIFO);
    rt_timer_start(rt_timer_create("mtmr", irq_tmr, RT_NULL, rt_tick_from_millisecond(1000/GYR_FRQ), RT_TIMER_FLAG_PERIODIC));
    
    while(1)
    {
        rt_sem_take(sem, RT_WAITING_FOREVER);
        rt_device_read(fxos, 0, acc, 6);
        rt_device_read(fxas, 0, gyr, 6);
        
        vsub(gyr, gb, gyr_corrected, 3);
        
        filter_mahony_imu_update(q, acc, gyr_corrected, mag, 1.0 / GYR_FRQ);
        q2eul(q, eul, "321");
        tx_len = bin_0x91data(tx_buf, 0, 1, acc, gyr_corrected, q, mag, eul[0]*RadToDeg, eul[1]*RadToDeg, eul[2]*RadToDeg, 0, 25);
        
        /* bias cancel */
        if(fabs(gyr[0]) < 5 && fabs(gyr[1]) < 5 && fabs(gyr[2]) < 5)
        {
            vadd2(gyr_sum, gyr, 3);
            still_cntr++;
        }
        else
        {
            still_cntr = 0;
            vset(gyr_sum, 3, 0);
        }
        
        if(still_cntr > 500)
        {
            gb[0] = gyr_sum[0] / still_cntr;
            gb[1] = gyr_sum[1] / still_cntr;
            gb[2] = gyr_sum[2] / still_cntr;
            vset(gyr_sum, 3, 0);
            still_cntr = 0;
        }
        
        send_div++; send_div %= 5;
        
        if(!send_div)
        {
            rt_device_write(console, 0, tx_buf, tx_len);
        }
    }
}


static int sensor_fusion(void)
{
    return rt_thread_startup(rt_thread_create("t3d", thread_sensor_fusion_entry, RT_NULL, 1024, 21, 1));
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(sensor_fusion, sensor_fusion);
#endif

//INIT_APP_EXPORT(sensor_fusion);