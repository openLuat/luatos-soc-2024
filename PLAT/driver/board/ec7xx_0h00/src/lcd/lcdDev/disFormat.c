#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bsp.h"
#include "bsp_custom.h"
#include "lcdDrv.h"
#include "lcdComm.h"
/**
  \fn          
  \brief    
  \return
*/
void yuv422ToRgb565(const void* inbuf, void* outbuf, int width, int height) 
{
    const uint8_t* yuv422_buf = (const uint8_t*)inbuf;
    uint16_t* rgb565_buf = (uint16_t*)outbuf;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Extract Y, U, and V components
            int y = *yuv422_buf++;
            int u = *yuv422_buf++;
            int v = *yuv422_buf++;

            // Convert YUV to RGB
            int r = y + ((v - 128) * 1404) / 1000;
            int g = y - ((u - 128) * 343) / 1000 - ((v - 128) * 711) / 1000;
            int b = y + ((u - 128) * 1772) / 1000;

            // Clamp RGB values to the valid range
            r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
            g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
            b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

            // Convert RGB to 565 format
            uint16_t r5 = (r * 249 + 1014) >> 11;
            uint16_t g6 = (g * 253 + 505) >> 10;
            uint16_t b5 = (b * 249 + 1014) >> 11;

            // Combine RGB components into a 16-bit value
            uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;

            // Store the RGB565 value in the output buffer
            *rgb565_buf++ = rgb565;
        }
    }
}
/**
  \fn          
  \brief    
  \return
*/
void yuv420ToRgb565(const void* inbuf, void* outbuf, int width, int height) {
    const uint8_t* yuv420_buf = (const uint8_t*)inbuf;
    uint16_t* rgb565_buf = (uint16_t*)outbuf;

    int y_size = width * height;
    int uv_size = y_size / 4;
    const uint8_t* y_plane = yuv420_buf;
    const uint8_t* u_plane = yuv420_buf + y_size;
    const uint8_t* v_plane = u_plane + uv_size;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int y_index = y * width + x;
            int uv_index = (y / 2) * (width / 2) + (x / 2);

            // Extract Y, U, and V components
            int yy = y_plane[y_index];
            int uu = u_plane[uv_index] - 128;
            int vv = v_plane[uv_index] - 128;

            // Convert YUV to RGB
            int r = yy + vv + ((vv * 103) >> 8);
            int g = yy - ((uu * 88) >> 8) - ((vv * 183) >> 8);
            int b = yy + uu + ((uu * 198) >> 8);

            // Clamp RGB values to the valid range
            r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
            g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
            b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

            // Convert RGB to 565 format
            uint16_t r5 = (r * 249 + 1014) >> 11;
            uint16_t g6 = (g * 253 + 505) >> 10;
            uint16_t b5 = (b * 249 + 1014) >> 11;

            // Combine RGB components into a 16-bit value
            uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;

            // Store the RGB565 value in the output buffer
            *rgb565_buf++ = rgb565;
        }
    }
}
/**
  \fn          
  \brief    
  \return
*/
void rgb565ToYuv422(const void* inbuf, void* outbuf, int width, int height) {
    const uint16_t* rgb565 = (const uint16_t*)inbuf;
    uint8_t* yuv422 = (uint8_t*)outbuf;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t rgb = *rgb565++;
            uint8_t r = (rgb >> 11) & 0x1F;
            uint8_t g = (rgb >> 5) & 0x3F;
            uint8_t b = rgb & 0x1F;

            // Convert RGB to YUV
            int yy = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            int uu = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            int vv = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

            // Clamp YUV values to the valid range
            yy = (yy < 0) ? 0 : ((yy > 255) ? 255 : yy);
            uu = (uu < 0) ? 0 : ((uu > 255) ? 255 : uu);
            vv = (vv < 0) ? 0 : ((vv > 255) ? 255 : vv);

            // Store YUV values
            *yuv422++ = yy;
            if (x % 2 == 0) {
                *yuv422++ = uu;
                *yuv422++ = vv;
            }
        }
    }
}
/**
  \fn          
  \brief    
  \return
*/
void rgb565ToYuv420(const void* inbuf, void* outbuf, int width, int height) {
    const uint16_t* rgb565 = (const uint16_t*)inbuf;
    uint8_t* yuv420 = (uint8_t*)outbuf;
    uint8_t* yplane = yuv420;
    uint8_t* uplane = yuv420 + width * height;
    uint8_t* vplane = uplane + ((width + 1) / 2) * ((height + 1) / 2);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t rgb = *rgb565++;
            uint8_t r = (rgb >> 11) & 0x1F;
            uint8_t g = (rgb >> 5) & 0x3F;
            uint8_t b = rgb & 0x1F;

            // Convert RGB to YUV
            int yy = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            int uu = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            int vv = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

            // Clamp YUV values to the valid range
            yy = (yy < 0) ? 0 : ((yy > 255) ? 255 : yy);
            uu = (uu < 0) ? 0 : ((uu > 255) ? 255 : uu);
            vv = (vv < 0) ? 0 : ((vv > 255) ? 255 : vv);

            // Store YUV values
            *yplane++ = yy;
            if (y % 2 == 0 && x % 2 == 0) {
                *uplane++ = uu;
                if (x < width - 1) {
                    *vplane++ = vv;
                }
            }
        }
    }
}

/**
  \fn          
  \brief    
  \return
*/

// Define RGB565 and YCbCr pixel structures
typedef struct {
    uint16_t rgb; // 5 bits for R, 6 bits for G, 5 bits for B
} RGB565Pixel;

// Define RGB888 and YCbCr pixel structures
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB888Pixel;

typedef struct {
    uint8_t y;
    uint8_t cb;
    uint8_t cr;
} YCbCrPixel;

// Function to convert RGB565 to RGB888
RGB888Pixel rgb565ToRgb888(RGB565Pixel rgb565) {
    RGB888Pixel rgb888;
    rgb888.r = (rgb565.rgb >> 11) & 0x1F; // Red component
    rgb888.g = (rgb565.rgb >> 5) & 0x3F;  // Green component
    rgb888.b = rgb565.rgb & 0x1F;         // Blue component

    // Scale up to 8 bits
    rgb888.r = (rgb888.r << 3) | (rgb888.r >> 2);
    rgb888.g = (rgb888.g << 2) | (rgb888.g >> 4);
    rgb888.b = (rgb888.b << 3) | (rgb888.b >> 2);

    return rgb888;
}

// Function to convert RGB888 to YCbCr
YCbCrPixel rgb888ToYCbCr(RGB888Pixel rgb888) {
    YCbCrPixel ycbcr;
    ycbcr.y  = (uint8_t)( 0.299 * rgb888.r + 0.587 * rgb888.g + 0.114 * rgb888.b);
    ycbcr.cb = (uint8_t)(-0.169 * rgb888.r - 0.331 * rgb888.g + 0.500 * rgb888.b + 128);
    ycbcr.cr = (uint8_t)( 0.500 * rgb888.r - 0.419 * rgb888.g - 0.081 * rgb888.b + 128);
    return ycbcr;
}

// Function to convert RGB565 to YCbCr
void rgb565ToYCbCr(const void* inbuf, void* outbuf, int width, int height) {
    const RGB565Pixel* inRGB565 = (const RGB565Pixel*)inbuf;
    YCbCrPixel* outYCbCr = (YCbCrPixel*)outbuf;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            RGB565Pixel rgb565 = inRGB565[y * width + x];

            // Convert RGB565 to RGB888
            RGB888Pixel rgb888 = rgb565ToRgb888(rgb565);

            // Convert RGB888 to YCbCr
            YCbCrPixel ycbcr = rgb888ToYCbCr(rgb888);

            // Store YCbCr in the output buffer
            outYCbCr[y * width + x] = ycbcr;
        }
    }
}

// Function to convert YCbCr to RGB888
RGB888Pixel yCbCrToRgb888(YCbCrPixel ycbcr) {
    RGB888Pixel rgb888;
    int r = ycbcr.y + ((ycbcr.cr - 128) * 1.402);
    int g = ycbcr.y - ((ycbcr.cb - 128) * 0.344) - ((ycbcr.cr - 128) * 0.714);
    int b = ycbcr.y + ((ycbcr.cb - 128) * 1.772);

    // Clamp the RGB values to the valid range [0, 255]
    rgb888.r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
    rgb888.g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
    rgb888.b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

    return rgb888;
}

// Function to convert RGB888 to RGB565
RGB565Pixel rgb888ToRgb565(RGB888Pixel rgb888) {
    RGB565Pixel rgb565;
    rgb565.rgb = ((rgb888.r >> 3) << 11) | ((rgb888.g >> 2) << 5) | (rgb888.b >> 3);
    return rgb565;
}

// Function to convert YCbCr to RGB565
void yCbCrToRgb565(const void* inbuf, void* outbuf, int width, int height) {
    const YCbCrPixel* inYCbCr = (const YCbCrPixel*)inbuf;
    RGB565Pixel* outRGB565 = (RGB565Pixel*)outbuf;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            YCbCrPixel ycbcr = inYCbCr[y * width + x];

            // Convert YCbCr to RGB888
            RGB888Pixel rgb888 = yCbCrToRgb888(ycbcr);

            // Convert RGB888 to RGB565
            RGB565Pixel rgb565 = rgb888ToRgb565(rgb888);

            // Store RGB565 in the output buffer
            outRGB565[y * width + x] = rgb565;
        }
    }
}