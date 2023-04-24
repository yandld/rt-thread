/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-22     Magicoe      The first version for LPC55S6x
 */

#include <rthw.h>
#include <rtdevice.h>
#include <rtthread.h>
#include "drv_pin.h"
#include "drv_st7735.h"




st7735_spi_t st7735;



extern st7735_spi_t st7735;

static void lcd_spi_send8(uint8_t dat)
{
    rt_spi_send(st7735.spi_dev, &dat, 1);
}

void lcd_spi_send16(uint16_t dat)
{
    uint8_t send_buf[2];
    send_buf[0] = dat>>8;
    send_buf[1] = dat;

    rt_spi_send(st7735.spi_dev, &send_buf, 2);
}

void lcd_spi_write_reg(uint8_t dat)
{
    rt_pin_write(st7735.dc_pin, PIN_LOW);
    lcd_spi_send8(dat);
    rt_pin_write(st7735.dc_pin, PIN_HIGH);
}



int st7735_init(const char *spi_dev_name, uint8_t dc_pin)
{
    int i;

    st7735.spi_dev = (struct rt_spi_device *)rt_device_find(spi_dev_name);
    RT_ASSERT(st7735.spi_dev != RT_NULL);

    st7735.dc_pin = dc_pin;

    struct rt_spi_configuration spi_config =
    {
        RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_MASTER,
        8,
        0,
        24*1000*1000,
    };

    rt_spi_configure(st7735.spi_dev, &spi_config);

    rt_pin_mode(st7735.dc_pin, PIN_MODE_OUTPUT);

    lcd_spi_write_reg(0x11);     //Sleep out
    rt_thread_mdelay(120);                //Delay 120ms
    lcd_spi_write_reg(0xB1);     //Normal mode
    lcd_spi_send8(0x05);
    lcd_spi_send8(0x3C);
    lcd_spi_send8(0x3C);
    lcd_spi_write_reg(0xB2);     //Idle mode
    lcd_spi_send8(0x05);
    lcd_spi_send8(0x3C);
    lcd_spi_send8(0x3C);
    lcd_spi_write_reg(0xB3);     //Partial mode
    lcd_spi_send8(0x05);
    lcd_spi_send8(0x3C);
    lcd_spi_send8(0x3C);
    lcd_spi_send8(0x05);
    lcd_spi_send8(0x3C);
    lcd_spi_send8(0x3C);
    lcd_spi_write_reg(0xB4);     //Dot inversion
    lcd_spi_send8(0x03);
    lcd_spi_write_reg(0xC0);     //AVDD GVDD
    lcd_spi_send8(0xAB);
    lcd_spi_send8(0x0B);
    lcd_spi_send8(0x04);
    lcd_spi_write_reg(0xC1);     //VGH VGL
    lcd_spi_send8(0xC5);   //C0
    lcd_spi_write_reg(0xC2);     //Normal Mode
    lcd_spi_send8(0x0D);
    lcd_spi_send8(0x00);
    lcd_spi_write_reg(0xC3);     //Idle
    lcd_spi_send8(0x8D);
    lcd_spi_send8(0x6A);
    lcd_spi_write_reg(0xC4);     //Partial+Full
    lcd_spi_send8(0x8D);
    lcd_spi_send8(0xEE);
    lcd_spi_write_reg(0xC5);     //VCOM
    lcd_spi_send8(0x0F);
    lcd_spi_write_reg(0xE0);     //positive gamma
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x0E);
    lcd_spi_send8(0x08);
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x10);
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x02);
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x09);
    lcd_spi_send8(0x0F);
    lcd_spi_send8(0x25);
    lcd_spi_send8(0x36);
    lcd_spi_send8(0x00);
    lcd_spi_send8(0x08);
    lcd_spi_send8(0x04);
    lcd_spi_send8(0x10);
    lcd_spi_write_reg(0xE1);     //negative gamma
    lcd_spi_send8(0x0A);
    lcd_spi_send8(0x0D);
    lcd_spi_send8(0x08);
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x0F);
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x02);
    lcd_spi_send8(0x07);
    lcd_spi_send8(0x09);
    lcd_spi_send8(0x0F);
    lcd_spi_send8(0x25);
    lcd_spi_send8(0x35);
    lcd_spi_send8(0x00);
    lcd_spi_send8(0x09);
    lcd_spi_send8(0x04);
    lcd_spi_send8(0x10);

    lcd_spi_write_reg(0xFC);
    lcd_spi_send8(0x80);

    lcd_spi_write_reg(0x3A);
    lcd_spi_send8(0x05);
    lcd_spi_write_reg(0x36);
    if(USE_HORIZONTAL==0)lcd_spi_send8(0x08);
    else if(USE_HORIZONTAL==1)lcd_spi_send8(0xC8);
    else if(USE_HORIZONTAL==2)lcd_spi_send8(0x78);
    else lcd_spi_send8(0xA8);
    lcd_spi_write_reg(0x21);     //Display inversion
    lcd_spi_write_reg(0x29);     //Display on
    lcd_spi_write_reg(0x2A);     //Set Column Address
    lcd_spi_send8(0x00);
    lcd_spi_send8(0x1A);  //26
    lcd_spi_send8(0x00);
    lcd_spi_send8(0x69);   //105
    lcd_spi_write_reg(0x2B);     //Set Page Address
    lcd_spi_send8(0x00);
    lcd_spi_send8(0x01);    //1
    lcd_spi_send8(0x00);
    lcd_spi_send8(0xA0);    //160
    lcd_spi_write_reg(0x2C);


    return rt_device_register(&st7735.parent, "st7735", RT_DEVICE_FLAG_RDWR);
}






void lcd_set_addr(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
    if(USE_HORIZONTAL==0)
    {
        lcd_spi_write_reg(0x2a);//列地址设置
        lcd_spi_send16(x1+26);
        lcd_spi_send16(x2+26);
        lcd_spi_write_reg(0x2b);//行地址设置
        lcd_spi_send16(y1+1);
        lcd_spi_send16(y2+1);
        lcd_spi_write_reg(0x2c);//储存器写
    }
    else if(USE_HORIZONTAL==1)
    {
        lcd_spi_write_reg(0x2a);//列地址设置
        lcd_spi_send16(x1+26);
        lcd_spi_send16(x2+26);
        lcd_spi_write_reg(0x2b);//行地址设置
        lcd_spi_send16(y1+1);
        lcd_spi_send16(y2+1);
        lcd_spi_write_reg(0x2c);//储存器写
    }
    else if(USE_HORIZONTAL==2)
    {
        lcd_spi_write_reg(0x2a);//列地址设置
        lcd_spi_send16(x1+1);
        lcd_spi_send16(x2+1);
        lcd_spi_write_reg(0x2b);//行地址设置
        lcd_spi_send16(y1+26);
        lcd_spi_send16(y2+26);
        lcd_spi_write_reg(0x2c);//储存器写
    }
    else
    {
        lcd_spi_write_reg(0x2a);//列地址设置
        lcd_spi_send16(x1+1);
        lcd_spi_send16(x2+1);
        lcd_spi_write_reg(0x2b);//行地址设置
        lcd_spi_send16(y1+26);
        lcd_spi_send16(y2+26);
        lcd_spi_write_reg(0x2c);//储存器写
    }
}







void st7735_fill_color(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint16_t i,j;
    lcd_set_addr(xsta,ysta,xend-1,yend-1);
    for(i=ysta;i<yend;i++)
    {
        for(j=xsta;j<xend;j++)
        {
            lcd_spi_send16(color);
        }
    }
}


void st7735_point(uint16_t x,uint16_t y,uint16_t color)
{
    lcd_set_addr(x, y, x, y);
    lcd_spi_send16(color);
}



void st7735_fill(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[])
{
    uint16_t i,j;
    uint32_t k=0;
    lcd_set_addr(x, y, x+length-1, y+width-1);
    rt_spi_send(st7735.spi_dev, pic, length*width*2);
}


