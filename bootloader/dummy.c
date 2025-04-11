#include "slpman.h"
#ifdef TYPE_EC718M

#ifdef __LUATOS__
#include "luat_conf_bsp.h"
#endif

uint8_t user_io_sel(uint8_t io_sel)
{
#ifdef LUAT_MODEL_AIR8000
	slpManNormalIOVoltSet(IOVOLT_3_30V);
#else
	if (io_sel)
	{
		slpManNormalIOVoltSet(IOVOLT_3_00V);
	}
#endif
	return 1;
}
#endif



