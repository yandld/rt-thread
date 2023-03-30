#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>
#include "basic_statistics.h"




//static int32_t raw_data_cos[TOTAL_SAMPLE_CNT];
//static int32_t raw_data_sin[TOTAL_SAMPLE_CNT];
//static int32_t temperature[TOTAL_SAMPLE_CNT];

    
int bs_init(bs_t *bs, uint32_t size)
{
    memset(bs, 0, sizeof(bs_t));
    
    bs->size = size;
    bs->buf = rt_malloc(size*sizeof(float));
    
    if(!bs->buf)
    {
        rt_kprintf("bs malloc failed\r\n");
        return RT_ERROR;
    }
    return RT_EOK;
}

void bs_add_sample(bs_t *bs, float data)
{
    bs->buf[bs->cnt++] = data;
}


void bs_dump(bs_t *bs, uint32_t opt)
{
    rt_kprintf("total %d sample\r\n", bs->cnt);
    rt_kprintf("max:     %f\r\n", bs->max);
    rt_kprintf("min:     %f\r\n", bs->min);
    rt_kprintf("avg:     %f\r\n", bs->avg);
    rt_kprintf("std:     %f\r\n", bs->std);
    rt_kprintf("voltage: %fV\r\n", bs->avg / 65535*3.3);
    rt_kprintf("ENOB:    %f\r\n", bs->ENOB);
    
    
//    int i;
//    for(i=0; i<bs->cnt; i++)
//    {
//        rt_kprintf("%f\r\n", bs->buf[i]);
//    }
}

void bs_calc_result(bs_t *bs)
{
    int i;
    bs->sum = 0;
     
    bs->max = bs->min = bs->buf[0];
    
    for(i=0; i<bs->cnt; i++)
    {
        if(bs->buf[i] > bs->max) bs->max = bs->buf[i];
        if(bs->buf[i] < bs->min) bs->min = bs->buf[i];
        bs->sum += bs->buf[i];
    }
    
    bs->avg = bs->sum / bs->cnt;
    
    bs->sum = 0;
    for(i=0; i<bs->cnt; i++)
    {
        bs->sum += (bs->buf[i] - bs->avg)*(bs->buf[i] - bs->avg);
    }
    
    bs->var = bs->sum / (bs->cnt - 1);
    bs->std = sqrt(bs->var);
    bs->ENOB = 16 - log(bs->max - bs->min);
}

void bs_free(bs_t *bs)
{
    rt_free(bs->buf);
}

//static int32_t find_max(int32_t *buf, uint32_t size)
//{
//    int i;
//    int32_t max = -9999999;
//    for (i = 0; i < size; i ++)
//    {
//        if (buf[i] > max)
//        {
//            max = buf[i];
//        }
//    }
//    return max;
//}

//static int16_t find_mean(int32_t *buf, uint32_t size)
//{
//    int i;
//    int32_t mean = 0;
//    for (i = 0; i < size; i ++)
//    {
//        mean += buf[i];
//    }
//    return mean / size;
//}


//static int32_t find_min(int32_t *buf, uint32_t size)
//{
//    int i;
//    int32_t min = 9999999;
//    for (i = 0; i < size; i ++)
//    {
//        if (buf[i] < min)
//        {
//            min = buf[i];
//        }
//    }
//    return min;
//}

