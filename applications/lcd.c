#include <lcd_port.h>
#include <string.h>
#include "lcd.h"
#include "lcdfont.h"

#define DRV_DEBUG
#define LOG_TAG "lcd"
#include <drv_log.h>

struct drv_lcd_device *g_lcd_handle;

uint32_t g_back_color = 0xFFFFFFFF; /* 背景色 */

void lcd_display_dir(dir_e dir)
{
    g_lcd_handle->direction = dir; /* 竖屏/横屏 */
}

rt_err_t lcd_init(void)
{
    g_lcd_handle = (struct drv_lcd_device *)rt_device_find("lcd");
    if (g_lcd_handle == RT_NULL)
    {
        LOG_E("init lcd failed!\n");
        return RT_ERROR;
    }
    else
    {
        lcd_display_dir(Portrait); /* 默认为竖屏 */
        return RT_EOK;
    }
}

/**
 * @brief       设置LCD显示方向
 * @param       dir:0,竖屏; 1,横屏
 * @retval      无
 */

void lcd_flush()
{
    g_lcd_handle->parent.control(&g_lcd_handle->parent, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
}
// 画点函数
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    uint32_t offset;
    // 检查坐标是否超出屏幕范围
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT)
        return;

    // 计算帧缓冲区中的偏移量，注意屏幕的宽度
    if (g_lcd_handle->direction == Landscape)

        offset = (y * LCD_WIDTH + x) * 2; // 横屏
    else
        offset = (LCD_WIDTH * (LCD_HEIGHT - x - 1) + y) * 2; // 竖屏

    // 更新帧缓冲区中的颜色
    g_lcd_handle->lcd_info.framebuffer[offset] = color & 0xFF;            // 低字节
    g_lcd_handle->lcd_info.framebuffer[offset + 1] = (color >> 8) & 0xFF; // 高字节
}

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    delta_x = x2 - x1; /* 计算坐标增量 */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1; /* 设置单步方向 */
    }
    else if (delta_x == 0)
    {
        incx = 0; /* 垂直线 */
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0; /* 水平线 */
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
    {
        distance = delta_x; /* 选取基本增量坐标轴 */
    }
    else
    {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++) /* 画线输出 */
    {
        lcd_draw_point(row, col, color); /* 画点 */
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * @brief       在指定位置显示一个字符
 * @param       x,y  : 坐标
 * @param       chr  : 要显示的字符:" "--->"~"
 * @param       size : 字体大小 12/16/24/32
 * @param       mode : 叠加方式(1); 非叠加方式(0);
 * @param       color: 字体颜色
 * @retval      无
 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = 0;
    uint8_t *pfont = 0;

    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); /* 得到字体一个字符对应点阵集所占的字节数 */
    chr = chr - ' ';                                        /* 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库） */

    switch (size)
    {
    case 12:
        pfont = (uint8_t *)asc2_1206[chr]; /* 调用1206字体 */
        break;

    case 16:
        pfont = (uint8_t *)asc2_1608[chr]; /* 调用1608字体 */
        break;

    case 24:
        pfont = (uint8_t *)asc2_2412[chr]; /* 调用2412字体 */
        break;

    case 32:
        pfont = (uint8_t *)asc2_3216[chr]; /* 调用3216字体 */
        break;

    default:
        return;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t]; /* 获取字符的点阵数据 */

        for (t1 = 0; t1 < 8; t1++) /* 一个字节8个点 */
        {
            if (temp & 0x80) /* 有效点,需要显示 */
            {
                lcd_draw_point(x, y, color); /* 画点出来,要显示这个点 */
            }
            else if (mode == 0) /* 无效点,不显示 */
            {
                lcd_draw_point(x, y, g_back_color); /* 画背景色,相当于这个点不显示(注意背景色由全局变量控制) */
            }

            temp <<= 1; /* 移位, 以便获取下一个位的状态 */
            y++;

            if (y >= LCD_HEIGHT)
                return; /* 超区域了 */

            if ((y - y0) == size) /* 显示完一列了? */
            {
                y = y0; /* y坐标复位 */
                x++;    /* x坐标递增 */

                if (x >= LCD_WIDTH)
                {
                    return; /* x坐标超区域了 */
                }

                break;
            }
        }
    }
}

/**
 * @brief       显示字符串
 * @param       x,y         : 起始坐标
 * @param       width,height: 区域大小
 * @param       size        : 选择字体 12/16/24/32
 * @param       p           : 字符串首地址
 * @param       color       : 字体颜色
 * @retval      无
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint8_t x0 = x;

    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) /* 判断是不是非法字符! */
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)
        {
            break; /* 退出 */
        }

        lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}