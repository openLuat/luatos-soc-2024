#ifdef __LUATOS__
#include "luat_conf_bsp.h"
#ifdef LUAT_MODEL_AIR780EPM
#include "slpman.h"
uint8_t user_io_sel(uint8_t io_sel)
{
	if (io_sel)
	{
		slpManNormalIOVoltSet(IOVOLT_3_00V);
	}
	return 1;
}
#endif

#ifdef LUAT_MODEL_AIR8000
#include "slpman.h"
uint8_t user_io_sel(uint8_t io_sel)
{
	slpManNormalIOVoltSet(IOVOLT_3_30V);
	return 1;
}
#endif
#else
//#ifdef TYPE_EC718M
//#include "slpman.h"
//uint8_t user_io_sel(uint8_t io_sel)
//{
//	if (io_sel)
//	{
//		slpManNormalIOVoltSet(IOVOLT_3_30V);
//	}
//	return 1;
//}
//#endif
#endif

