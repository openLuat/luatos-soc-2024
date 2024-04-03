/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    GT911.c
 * Description:  EC7xx touchpanel driver source file
 * History:      Rev1.0   2023-09-18
 *
 ****************************************************************************/
#ifdef FEATURE_TP_GT911_ENABLE
#include "ec7xx.h"
#include "bsp.h"
#include "gt911.h"
#include "adc.h"
#include "string.h"
#include "stdio.h"

#include "pad.h"
#include "gpio.h"
#include "slpman.h"
#include "tpDrv.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

uint16_t x[10]={0};
uint16_t y[10]={0};
uint16_t sta=0;
int touchtype=0;
extern ARM_DRIVER_I2C	*i2cMasterDrv;
/**
  \fn          
  \brief    
  \return
*/
int tp_gt911_send(uint16_t regAddr, uint8_t* buf,uint8_t len)
{
	uint8_t reg[2]={0};
	reg[0] = regAddr >>8 ;
	reg[1] = regAddr & 0xff ;
    i2cMasterDrv->MasterTransmit(GT911_ADDR, reg, 2, true); 
    i2cMasterDrv->MasterTransmit(GT911_ADDR, buf, len, false);
    return 1;   
}

/**
  \fn          
  \brief    
  \return
*/

int tp_gt911_read(uint16_t regAddr, uint8_t* buf,uint8_t len)
{
	uint8_t reg[2]={0};
	reg[0] = regAddr >>8 ;
	reg[1] = regAddr & 0xff ;
	// printf("regAddr:0x%04x,reg0:0x%02x,reg1:0x%02x\r\n",regAddr,reg[0],reg[1]);
    i2cMasterDrv->MasterTransmit(GT911_ADDR, reg, 2, true);
    i2cMasterDrv->MasterReceive(GT911_ADDR, buf,len,false);
    return 1;   
}

/**
  \fn          
  \brief    
  \return
*/
static int tp_gt911_init(void* cb)
{
    #ifdef TP_IRQ_GPIO_PIN
    PadConfig_t padConfig = {0};
    PAD_getDefaultConfig(&padConfig);
    padConfig.pullUpEnable = PAD_PULL_UP_DISABLE;
    padConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    padConfig.mux = TP_IRQ_PAD_ALT_FUNC;
    PAD_setPinConfig(TP_IRQ_PAD_INDEX, &padConfig);

    GpioPinConfig_t gpioCfg = {0};
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 0;
    GPIO_pinConfig(TP_IRQ_GPIO_INSTANCE, TP_IRQ_GPIO_PIN, &gpioCfg);
    #endif
	uint8_t temp[5]={0}; 
	// GPIO_pinWrite(TP_IRQ_GPIO_INSTANCE,1<<TP_IRQ_GPIO_PIN,0<<TP_IRQ_GPIO_PIN);
	GPIO_pinWrite(TP_RST_GPIO_INSTANCE,1<<TP_RST_GPIO_PIN,0<<TP_RST_GPIO_PIN);		//32
	delay_us(10*1000);
	GPIO_pinWrite(TP_IRQ_GPIO_INSTANCE,1<<TP_IRQ_GPIO_PIN,1<<TP_IRQ_GPIO_PIN);
	delay_us(500);
	GPIO_pinWrite(TP_RST_GPIO_INSTANCE,1<<TP_RST_GPIO_PIN,1<<TP_RST_GPIO_PIN);		//32
	delay_us(10*1000);
	gpioCfg.pinDirection = GPIO_DIRECTION_INPUT;
	GPIO_pinConfig(TP_IRQ_GPIO_INSTANCE, TP_IRQ_GPIO_PIN, &gpioCfg);
	delay_us(100*1000);
	tp_gt911_read(GT_PID_REG,temp,4);
	temp[4]=0;
	SYSLOG_PRINT(SL_INFO,"ID:0x%c%c%c%c \r\n",temp[0],temp[1],temp[2],temp[3]);	//打印ID
	if(strcmp((char*)temp,"911")==0)//ID==911
	{
		temp[0]=0X02;		
		tp_gt911_send(GT_CTRL_REG,temp,1);//软复位GT911
 		tp_gt911_read(GT_CFGS_REG,temp,1);//读取GT_CFGS_REG寄存器
		
		delay_us(10*1000);
		temp[0]=0X00;
		tp_gt911_send(GT_CTRL_REG,temp,1);	//结束复位
		// printf("reset gt911\r\n");
	}	
}
/**
  \fn          
  \brief    
  \return
*/
uint8_t gt911_mode = 0;
uint8_t tp_gt911_scan(int16_t *pos)
{
	uint8_t buf[4*5];
	uint8_t i=0;
	int finger_num=0;
	tp_gt911_read(GT_GSTID_REG,&gt911_mode,1);//读取触摸点的状态 
	if((gt911_mode&0XF)&&((gt911_mode&0XF)<6))
	{
		finger_num = gt911_mode&0XF;
		// SYSLOG_PRINT(SL_INFO, "fingers:%d\r\n",finger_num);
		for(i=0;i<finger_num;i++)	//触摸有效
		{
			tp_gt911_read(GT_TP1_REG+8*i,buf,4);	//读取XY坐标值
			if(touchtype&0X01)//横屏
			{
				y[i]=((uint16_t)buf[1]<<8)+buf[0];
				x[i]=(((uint16_t)buf[3]<<8)+buf[2]);
			}else
			{
				x[i]=((uint16_t)buf[1]<<8)+buf[0];
				y[i]=((uint16_t)buf[3]<<8)+buf[2];
			}  
			// SYSLOG_PRINT(SL_INFO, "x[%d]:%d,y[%d]:%d\r\n",i,x[i],i,y[i]);
		}			
		if(x[0]==0 && y[0]==0){
			gt911_mode=0;	//读到的数据都是0,则忽略此次数据
			finger_num=0;
		}
	}
	if((gt911_mode&0X80)&&(gt911_mode&0XF)<6)
	{
		uint8_t temp=0;
		tp_gt911_send(GT_GSTID_REG,&temp,1);	//清标志 	
		pos[0]=x[0];
    	pos[1]=y[0];
	}	
	return finger_num;

}

/**
  \fn          
  \brief    
  \return
*/
tpDrvFunc_t gt911Drv = 
{
    .init           = tp_gt911_init,
    .send           = tp_gt911_send,
    .read           = tp_gt911_read,
    .scan           = tp_gt911_scan,
};
/**
  \fn          
  \brief    
  \return
*/
tpDrvPra_t gt911Pra = 
{
    .id     = 0x911,
    .width  = 240,
    .height = 320,
};

#endif