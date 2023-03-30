

#ifndef _BASIC_STATISTICS_H_
#define _BASIC_STATISTICS_H_


#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

typedef struct
{
    float       *buf;
    uint32_t    size;
    uint32_t    cnt;
    double      max;
    double      min;
    double      avg;
    double      var;
    double      std;
    double      sum;
    double      ENOB;    
}bs_t;
 

int bs_init(bs_t *bs, uint32_t size);
void bs_add_sample(bs_t *bs, float data);
void bs_dump(bs_t *bs, uint32_t opt);
void bs_calc_result(bs_t *bs);
void bs_free(bs_t *bs);


#endif

