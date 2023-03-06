/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "lcd_ssd1963_drv.h"
#include "zoo_rotate.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define GT4(number) (number>=4? number-4 : number)

/*******************************************************************************
 * Variables
 ******************************************************************************/
 AreaPoints_t PicZooArea[4] = {
    {  0U, 0U, LCD_H_POINTS/2U-1U, LCD_V_POINTS/2U-1U  },
    {  LCD_H_POINTS/2U, 0U, LCD_H_POINTS-1U, LCD_V_POINTS/2U-1U },
    {  LCD_H_POINTS/2U, LCD_V_POINTS/2U, LCD_H_POINTS-1U, LCD_V_POINTS-1U },
    {  0U, LCD_V_POINTS/2U, LCD_H_POINTS/2U-1U, LCD_V_POINTS-1U  }
};
 AreaPoints_t Pic_camera_Area_VGA[8] = {
     { 0, 0,    640-1, 60*1-1 },//640*480 / 8
     { 0, 60*1, 640-1, 60*2-1 },//640*480 / 8
     { 0, 60*2, 640-1, 60*3-1 },//640*480 / 8
     { 0, 60*3, 640-1, 60*4-1 },//640*480 / 8
     { 0, 60*4, 640-1, 60*5-1 },//640*480 / 8
     { 0, 60*5, 640-1, 60*6-1 },//640*480 / 8
     { 0, 60*6, 640-1, 60*7-1 },//640*480 / 8
     { 0, 60*7, 640-1, 60*8-1 },//640*480 / 8 
};

 AreaPoints_t Pic_camera_Area_VGA_ST7789S[4] = {
     { 0, 0,    320-1, 60*1-1 },//320*240 / 4
     { 0, 60*1, 320-1, 60*2-1 },//320*240 / 4
     { 0, 60*2, 320-1, 60*3-1 },//320*240 / 4
     { 0, 60*3, 320-1, 60*4-1 },//320*240 / 4

};
 AreaPoints_t Pic_camera_Area_QVGA[2] = {
    {  0U, 0U, 320U-1U, 240U-1U  },//align edge
    {  80U, 40U, 80+320U-1U, 40+240U-1U  },//align center
};
AreaPoints_t Pic_camera_Area_320x320[4] = {
    {  0U, 0U, 320-1, 320-1  },//align edge
    {  80U, 0U,80+320U-1U, 320U-1U  },//align center
    {  0U, 80U,320U-1U, 80+320U-1U  },//align center
    {  0U, 80U,160-1U, 80+160-1U  },//align center

};


AreaPoints_t Pic_camera_Area_QQVGA[4] = {
    {  0U, 0U, 160U-1U, 120U-1U  },//align edge
    {  160U, 100U, 160+160U-1U, 100+120U-1U  },//ALIGN center
    {  320U, 100U, 320+160U-1U, 100+120U-1U  },//ALIGN center
    {  640,  180,    640+160-1, 180+120-1  },//ALIGN center
};

uint16_t * ppb[4];

/*******************************************************************************
 * Functions
 ******************************************************************************/
/*!
 * @brief Display 
 */
void DispCamera_VGA(uint32_t coord_index, uint16_t * buf)
{
        /* Use multi-beat and DMA method to update image */
//    LCD_FillPicDMA(&Pic_camera_Area_VGA_ST7789S[coord_index],buf);
  LCD_FillPicDMA(&Pic_camera_Area_QVGA[0], buf);
 //  LCD_FillPicDMA(&Pic_camera_Area_320x320[2],buf);
}


/* EOF */
