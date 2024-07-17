/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-17     26339       the first version
 */
#ifndef APPLICATIONS_LCD_H_
#define APPLICATIONS_LCD_H_

#include <board.h>
#include <stdint.h>

#define WHITE 0xFFFF
#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define CYAN 0x07FF

typedef enum
{
    Portrait = 0, // 竖屏
    Landscape = 1 // 横屏
} dir_e;

rt_err_t lcd_init(void);
void lcd_flush();
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color);

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color);

#endif /* APPLICATIONS_LCD_H_ */
