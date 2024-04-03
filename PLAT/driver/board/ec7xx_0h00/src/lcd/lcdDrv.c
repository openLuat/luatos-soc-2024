#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "bsp_custom.h"
#include "lcdDrv.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
static lcdDev_t lcdDevList[SUPPORT_LCD_NUM] ;
static lcdObj_t lcdObjList[SUPPORT_LCD_NUM] = 
{
    {"st7789",    0x7789},
    {"st7796",    0x7796},
    {"nv3037",    0x3037},
    {"gc9307",    0x9307},
    {"st7567",    0x7567},
};
//支持列表用于查询设备信息

#ifdef FEATURE_LCD_ST7796_ENABLE
#include "lcdDev_7796.h"
extern lcdDrvFunc_t st7796Drv;
extern lcdDrvPra_t st7796Pra;
#endif 
#ifdef FEATURE_LCD_ST7789_ENABLE
#include "lcdDev_7789.h"
extern lcdDrvFunc_t st7789Drv;
extern lcdDrvPra_t st7789Pra;
#endif 
#ifdef FEATURE_LCD_GC9307_ENABLE
#include "lcdDev_9307.h"
extern lcdDrvFunc_t gc9307Drv;
extern lcdDrvPra_t gc9307Pra;
#endif
#ifdef FEATURE_LCD_NV3037_ENABLE
#include "lcdDev_3037.h"
extern lcdDrvFunc_t nv3037Drv;
extern lcdDrvPra_t nv3037Pra;
#endif

/**
 * \fn          lcdFind
 * \brief       Populates a list of LCD devices based on enabled features
 * \param       lcdList  Pointer to the list of LCD devices to populate
 * \return      uint8_t  The count of populated LCD devices
 */
static uint8_t lcdFind(lcdDev_t *lcdList)
{
    uint8_t cnt = 0; // Counter for the number of populated LCD devices
    
    // If the lcdList is NULL, return the total number of supported LCDs
    if (lcdList == NULL) return SUPPORT_LCD_NUM;
    
    // Iterate over the list of supported LCD objects
    for (uint8_t i = 0; i < SUPPORT_LCD_NUM; i++)
    {
        lcdDev_t *pdev = NULL; // Pointer to the current LCD device
        
        // Clear the memory of the current LCD device
        memset(lcdList + cnt, 0, sizeof(lcdDev_t));
        
        // Check for the ST7789 LCD feature
        #ifdef FEATURE_LCD_ST7789_ENABLE
        if (lcdObjList[i].id == st7789Pra.id)
        {
            pdev = lcdList + cnt; // Set the pointer to the current LCD device
            cnt++; // Increment the counter
            
            // Assign the parameters to the current LCD device
            pdev->pra = &st7789Pra;
            pdev->drv = &st7789Drv;
            pdev->obj = &lcdObjList[i];
            // Log the information (commented out for brevity)
            // SYSLOG_PRINT(SL_INFO,"%d,%d,0x%X,%s\r\n",i,cnt,pdev->obj->id,pdev->obj->name);
        }
        #endif
        
        // Check for the NV3037 LCD feature
        #ifdef FEATURE_LCD_NV3037_ENABLE
        if (lcdObjList[i].id == nv3037Pra.id)
        {
            pdev = lcdList + cnt; // Set the pointer to the current LCD device
            cnt++; // Increment the counter
            
            // Assign the parameters to the current LCD device
            pdev->pra = &nv3037Pra;
            pdev->drv = &nv3037Drv;
            pdev->obj = &lcdObjList[i];
            
            // Log the information (commented out for brevity)
            // SYSLOG_PRINT(SL_INFO,"%d,0x%X,%s\r\n",cnt,pdev->obj->id,pdev->obj->name); 
        }
        #endif
        // Check for the ST7796 LCD feature
        #ifdef FEATURE_LCD_ST7796_ENABLE
        if (lcdObjList[i].id == st7796Pra.id)
        {
            pdev = lcdList + cnt; // Set the pointer to the current LCD device
            cnt++; // Increment the counter
            
            // Assign the parameters to the current LCD device
            pdev->pra = &st7796Pra;
            pdev->drv = &st7796Drv;
            pdev->obj = &lcdObjList[i];
        }
        #endif
        #ifdef FEATURE_LCD_GC9307_ENABLE
        if (lcdObjList[i].id == gc9307Pra.id)
        {
            pdev = lcdList + cnt; // Set the pointer to the current LCD device
            cnt++; // Increment the counter
            // Assign the parameters to the current LCD device
            pdev->pra = &gc9307Pra;
            pdev->drv = &gc9307Drv;
            pdev->obj = &lcdObjList[i];
        }
        #endif
    }
    // Return the count of populated LCD devices
    return cnt;
}
/**
 * \fn          lcdInit
 * \brief       Initializes a list of LCD devices
 * \param       spi_cb  A pointer to a SPI callback function
 * \return      The number of initialized LCD devices
 */
int lcdInit(void* spi_cb)
{
    // Get the number of available LCD devices
    uint8_t num = lcdFind(lcdDevList);
    
    // If the number of available devices is greater than the maximum number to install,
    // limit the number of devices to install to the maximum number
    if (num > INSTALL_LCD_NUM) num = INSTALL_LCD_NUM;
    
    // Iterate over the list of available LCD devices
    for (uint8_t i = 0; i < num; i++) 
    {
        // If the device ID is not zero (indicating a valid device)
        if (lcdDevList[i].obj->id) 
        {
            // Log the device information (commented out for brevity)
            // SYSLOG_PRINT(SL_INFO,"%d,%s\r\n",i,lcdDevList[i].obj->name);
            // SYSLOG_PRINT(SL_INFO,"%d,0x%X,%s\r\n",i,lcdDevList[i].obj->id,lcdDevList[i].obj->name); 
            
            // Get a pointer to the current LCD device
            lcdDev_t *pdev = &lcdDevList[i]; 
            
            // Assert that the device parameters are not NULL
            // ASSERT(pdev->pra != NULL);
            // ASSERT(pdev->drv != NULL);
            
            // Set the height and width of the LCD from the device parameters
            pdev->height = pdev->pra->height;
            pdev->width  = pdev->pra->width;  
            
            // Initialize the LCD device using the provided SPI callback
            pdev->drv->init(pdev, spi_cb);
            
            // Set the handle of the LCD device to -1 (indicating not initialized)
            pdev->handle = -1;               
        }
    }
    
    // Return the number of initialized LCD devices
    return num;
}
/**
 * \fn          lcdOpen
 * \brief       Opens a specific LCD device by name
 * \param       name  The name of the LCD device to open
 * \return      A pointer to the opened LCD device, or NULL if not found
 */
lcdDev_t* lcdOpen(char* name)
{
    // If the name is NULL, return NULL
    if (name == NULL) return NULL;
    
    // Iterate over the list of installed LCD devices
    for (uint8_t i = 0; i < INSTALL_LCD_NUM; i++)
    {
        // Get a pointer to the current LCD device
        lcdDev_t *pdev = &lcdDevList[i];
        
        // Log the device information (commented out for brevity)
        // SYSLOG_PRINT(SL_INFO,"0x%X,%s,%s\r\n",pdev->obj->id,pdev->obj->name,name);
        
        // Check if the device ID is not zero and if the names match (case-insensitive)
        if (pdev->obj->id != 0 && strcasecmp(name, pdev->obj->name) == 0)
        {
            // Assert that the device parameters are not NULL
            // ASSERT(pdev->pra != NULL);
            // ASSERT(pdev->drv != NULL);
            
            // If the backlight control is enabled, set the backlight to full brightness
            #ifdef LCD_BL_PWM_ENABLE
            pdev->drv->backLight(pdev, 100);
            #else
            pdev->drv->onoff(pdev, 1);
            #endif
            
            // Return the pointer to the opened LCD device
            return pdev;
        }
    }
    
    // If no matching device is found, return NULL
    return NULL;
}
/**
 * \fn          lcdClose
 * \brief       Closes a specific LCD device
 * \param       pdev  Pointer to the LCD device to close
 * \return      0 if successful, 1 if the device pointer is NULL
 */
int lcdClose(lcdDev_t *pdev)
{
    // If the device pointer is NULL, return 1 to indicate an error
    if (pdev == NULL) return 1;
    
    // Call the close function of the device driver
    pdev->drv->close(pdev);
    
    // Set the handle of the device to -1 to indicate that it is no longer in use
    pdev->handle = -1;
    
    // Set the device pointer to NULL
    pdev = NULL;
    
    // Return 0 to indicate success
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
uint16_t lcdCheck(lcdDev_t *pdev)
{
   return pdev->drv->check(pdev); 
}

/**
  \fn          
  \brief        
  \return
*/

int lcdDirection(lcdDev_t *pdev,DisDirection_e dir)
{
   return pdev->drv->direction(pdev,dir); 
}
/**
  \fn          
  \brief        
  \return
*/

void camPreview(lcdDev_t *pdev, camPreviewStartStop_e previewStartStop)
{
    pdev->drv->startStopPreview(pdev, previewStartStop);

}


int lcdDrawPoint(lcdDev_t* lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    if (lcd == NULL)
    {
        return -1;
    }

    return lcd->drv->drawPoint(lcd, x, y, dataWrite);
}

int lcdPrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    if (lcd == NULL)
    {
        return -1;
    }
    return lcd->drv->prepareDisplay(lcd, sx, ex, sy, ey);
}

/**
 * \brief Fills an area of the LCD with the data from a buffer.
 *
 * This function fills an area of the LCD specified by the start and end coordinates
 * with the data from a buffer. The DMA trunk length parameter is not used in this
 * function and is mentioned in the comment as not being used.
 *
 * \param lcd A pointer to the LCD device structure.
 * \param sx The starting x-coordinate for the fill operation.
 * \param sy The starting y-coordinate for the fill operation.
 * \param ex The ending x-coordinate for the fill operation.
 * \param ey The ending y-coordinate for the fill operation.
 * \param buf A pointer to the buffer containing the data to be filled into the LCD.
 * \param dmaTrunkLength The length of the DMA trunk, which is not used in this function.
 *
 * \return Returns the result of the fill operation from the LCD driver.
 *         Returns -1 if the `lcd` pointer is `NULL`.
 */
int lcdFill(lcdDev_t* lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t* buf, uint32_t dmaTrunkLength)
{
    if (lcd == NULL)
    {
        return -1;
    }
    //y = vertically
    return lcd->drv->fill(lcd, sx, sy, ex, ey, buf, dmaTrunkLength);
}
/**
  \fn          
  \brief        
  \return
*/
int lcdDump(lcdDev_t* lcd, uint16_t sx, uint16_t sy, uint32_t* buf, uint32_t Length)
{
    if (lcd == NULL || buf == NULL)
    {
        return -1;
    }
    return lcd->drv->dump(lcd, sx, sy,buf, Length);
}
/**
  \fn          
  \brief        
  \return
*/
int lcdOnoff(lcdDev_t* lcd, uint8_t sta)
{
    if (lcd == NULL)
    {
        return -1;
    }
    return lcd->drv->onoff(lcd, sta);
}
/**
  \fn          
  \brief        
  \return
*/
void lcdBackLight(lcdDev_t* lcd, uint8_t sta)
{
    if (lcd == NULL)
    {
        return;
    }
    return lcd->drv->backLight(lcd, sta);
}
/**
  \fn          
  \brief        
  \return
*/
void lcdClear(lcdDev_t* lcd, uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{
    if (lcd == NULL)
    {
        return;
    }

    return lcd->drv->clear(lcd, buf, lcdHeight, lcdWidth, dmaTrunkLength);
}
/**
  \fn          
  \brief        
  \return
*/
void line(lcdDev_t* lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
    uint16_t t;
    int deltaX, deltaY, distance, xerr = 0, yerr = 0;
    int incx, incy, uRow, uCol;
    //uint8_t color1 = color>>8, color2 = color;
    uint32_t dataWrite = (color) | (color<<16);//(color1) | (color2<<8) | (color1<<16) | (color2<<24); 
    
    deltaX = ex - sx;
    deltaY = ey - sy;
    uRow = sx;
    uCol = sy;

    // x
    if (deltaX > 0) incx = 1; // set increase direction
    else if (deltaX == 0) incx = 0; // vertical line
    else {incx = -1; deltaX = -deltaX;}

    // y
    if (deltaY > 0) incy = 1; // set increase direction
    else if (deltaY == 0) incy = 0; // horizontal line
    else {incy = -1; deltaY = -deltaY;}

    //
    if (deltaX > deltaY) distance = deltaX;
    else distance = deltaY;

    //lcd->drv->writeSetup(lcd);
    //lcd->drv->addrSet(lcd)

    // draw line
    for (t = 0; t < distance + 1; t = t + 2)
    {
        lcdDrawPoint(lcd, uRow, uCol, dataWrite);
        xerr += deltaX;
        yerr += deltaY;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    } 
}


