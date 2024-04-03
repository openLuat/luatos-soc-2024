#ifndef __SOC_DMA_H__
#define __SOC_DMA_H__
#include "dma.h"
#include "common_api.h"

typedef struct
{
	llist_head Node;
	uint32_t Len;
	uint32_t MaxLen;
	uint32_t Pos;
	uint8_t Data[0];
}DMA_TxCacheNode;

void DMA_ForceStartStreamEx(uint8_t DMAStream, uint8_t TxDir, const void *Data, uint32_t Len, DmaTransferConfig_t* Config, uint8_t NeedIrq, uint8_t NeedVote);
void DMA_ForceStartStream(uint8_t DMAStream, uint8_t TxDir, const void *Data, uint32_t Len, DmaTransferConfig_t* Config, uint8_t NeedIrq);
#endif
