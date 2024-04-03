#include "psdial_ps_ctrl.h"
#include "cms_comm.h"
#include "common_api.h"
#include "pscommtype.h"
#include "usbd_clscdc.h"
#include "usbd_func_cc.h"
#include "usbd_multi_usrcfg_common.h"
#include "system_ec7xx.h"
#include "fota_utils.h"
#define PS_DIAL_PS_UP_MEM_SIZE      (UP_BUF_MAX_SIZE)
ALIGNED_4BYTE CAT_PSPHY_SHAREDATA   UINT8   psUpMem[PS_DIAL_PS_UP_MEM_SIZE];
void *psDialGetUpMemAndSize(UINT32 *pUpMemSize)
{
    if (pUpMemSize != PNULL)
    {
        *pUpMemSize     = sizeof(psUpMem);
    }

    return (void *)psUpMem;
}
