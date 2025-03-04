#include "cameraDrv.h"
#include "sctdef.h"

AP_PLAT_COMMON_DATA camI2cCfg_t gc6153_1sdrRegInfo[] = 
{
	// SYS
	{0xfe, 0xa0},
	{0xfe, 0xa0},
	{0xfe, 0xa0},
	{0xfa, 0x11}, 
	{0xfc, 0x00},
	{0xf6, 0x00},
	{0xfc, 0x12}, 
	
	// ANALOG & CISCTL
	{0xfe, 0x00},
	{0x01, 0x40}, 
	{0x02, 0x12}, 
	{0x0d, 0x40}, 
	{0x14, 0x7c},  // 0x7e
	{0x16, 0x05},  // 0x05
	{0x17, 0x18}, // 0x18
	{0x1c, 0x31}, 
	{0x1d, 0xbb}, 
	{0x1f, 0x3f}, 
	{0x73, 0x20}, 
	{0x74, 0x71}, 
	{0x77, 0x22}, 
	{0x7a, 0x08}, 
	{0x11, 0x18}, 
	{0x13, 0x48}, 
	{0x12, 0xc8}, 
	{0x70, 0xc8}, 
	{0x7b, 0x18}, 
	{0x7d, 0x30}, 
	{0x7e, 0x02}, 

	{0xfe, 0x10}, 
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x10},
	{0xfe, 0x00},

	{0x49, 0x61},
	{0x4a, 0x40},
	{0x4b, 0x58},

	/*ISP*/            
	{0xfe, 0x00},
	{0x39, 0x02}, 
	{0x3a, 0x80}, 
	{0x20, 0x7e}, 
	{0x26, 0xa7}, 

	/*BLK*/         
	{0x33, 0x10}, 
	{0x37, 0x06}, 
	{0x2a, 0x21}, 

	/*GAIN*/
	{0x3f, 0x16}, 

	/*DNDD*/    
	{0x52, 0xa6},
	{0x53, 0x81},
	{0x54, 0x43},
	{0x56, 0x78},
	{0x57, 0xaa},
	{0x58, 0xff}, 

	/*ASDE*/           
	{0x5b, 0x60}, 
	{0x5c, 0x50}, 
	{0xab, 0x2a}, 
	{0xac, 0xb5},
					   
	/*INTPEE*/        
	{0x5e, 0x06}, 
	{0x5f, 0x06},
	{0x60, 0x44},
	{0x61, 0xff},
	{0x62, 0x69}, 
	{0x63, 0x13}, 

	/*CC*/           
	{0x65, 0x13}, 
	{0x66, 0x26},
	{0x67, 0x07},
	{0x68, 0xf5}, 
	{0x69, 0xea},
	{0x6a, 0x21},
	{0x6b, 0x21}, 
	{0x6c, 0xe4},
	{0x6d, 0xfb},
				
	/*YCP*/          
	{0x81, 0x3b}, // 0
	{0x82, 0x3b}, // 0 : uyvy �ڰ�
	{0x83, 0x4b},
	{0x84, 0x90},
	{0x86, 0xf0},
	{0x87, 0x1d},
	{0x88, 0x16},
	{0x8d, 0x74},
	{0x8e, 0x25},
			 
	/*AEC*/       
	{0x90, 0x36},
	{0x92, 0x43},
	{0x9d, 0x32},
	{0x9e, 0x81},
	{0x9f, 0xf4},
	{0xa0, 0xa0},
	{0xa1, 0x04},
	{0xa3, 0x2d},
	{0xa4, 0x01},
					
	/*AWB*/         
	{0xb0, 0xc2},
	{0xb1, 0x1e},
	{0xb2, 0x10},
	{0xb3, 0x20},
	{0xb4, 0x2d},
	{0xb5, 0x1b}, 
	{0xb6, 0x2e},
	{0xb8, 0x13},
	{0xba, 0x60},
	{0xbb, 0x62},
	{0xbd, 0x78}, 
	{0xbe, 0x55},
	{0xbf, 0xa0}, 
	{0xc4, 0xe7},
	{0xc5, 0x15},
	{0xc6, 0x16},
	{0xc7, 0xeb}, 
	{0xc8, 0xe4},
	{0xc9, 0x16},
	{0xca, 0x16},
	{0xcb, 0xe9},
	{0x22, 0xf8}, 

	/*SPI*/          
	{0xfe, 0x02},
	{0x01, 0x01}, 
	{0x02, 0x02}, 
	{0x03, 0x20}, 
	{0x04, 0x20}, 
	{0x0a, 0x00}, 
	{0x13, 0x10}, 
	{0x24, 0x00}, 
	{0x28, 0x03}, 
	{0xfe, 0x00},
			  
	/*OUTPUT*/        
	{0xf2, 0x03}, 
	{0xfe, 0x00},
};

uint16_t gc6153GetRegCnt(char* regName)
{
    if (strcmp(regName, "gc6153_1sdr") == 0)
    {
        return (sizeof(gc6153_1sdrRegInfo) / sizeof(gc6153_1sdrRegInfo[0]));
    }
    
    return 0;
}


