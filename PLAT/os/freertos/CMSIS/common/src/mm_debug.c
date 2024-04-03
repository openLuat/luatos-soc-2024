/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    mm_debug.c
 * Description:  EC618 memory management debug log
 * History:      04/02/2018    Originated by bchang
 *
 ****************************************************************************/


/**************************************************************************************
use node to record allocated memory block, maintain free node list for free node to use
add node to a hash list when malloc, then remove node after free,
then clear the node and add to free list. this feature is mainly used to debug
memory leak and memory related bugs
**********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"
#include "mm_debug.h"
#include "debug_trace.h"
#include "sctdef.h"

#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#include "system_ec618.h"
#elif defined CHIP_EC718 || defined CHIP_EC716
#include "system_ec7xx.h"
#endif
#include "exception_process.h"
#include DEBUG_LOG_HEADER_FILE




mm_trace_node_t trace_node[MM_TRACE_MAX];//20 bytes * MEMTRACE_MAX
mm_trace_node_t *node_hash[MM_TRACE_HASH_SIZE];//4 bytes * MEMTRACE_HASH_SIZE
mm_trace_node_t *free_node;



/*----------------------------------------------------------------------------
 Brief:          memory management debug strucure init
 Details:        init free node list, main structure
 Input:          none
 Output:         none
 Note:           internal use
 History:         04/02/2018    Originated by bchang
------------------------------------------------------------------------------*/
MMDEBUG_TEXT_SECTION void mm_trace_init(void)
{
    struct mm_trace_node *node;
    unsigned long index;

    memset(trace_node, 0, sizeof(trace_node));
    memset(node_hash, 0, sizeof(node_hash));

    free_node = &trace_node[0];
    node = &trace_node[0];

    /* add all item to the free list */
    for (index = 1; index < MM_TRACE_MAX; index ++)
    {
        node->next = &trace_node[index];
        node = node->next;
    }

    node->next = NULL;
}



/*----------------------------------------------------------------------------
 Brief:          add memory management malloc trace
 Details:        reord malloc info to free node
 Input:          allocated buffer ptr, length
 Output:         none
 Note:           internal use
 History:         04/02/2018    Originated by bchang
------------------------------------------------------------------------------*/
MMDEBUG_TEXT_SECTION void mm_malloc_trace(void* buffer, unsigned long length, unsigned int func_lr)
{
    struct mm_trace_node* node;
    unsigned long index;
    const char *CurrentTaskName;

    /* get free node */
    uint32_t mask = SaveAndSetIRQMask();
    if (free_node == NULL)
    {
        /* no free node, just return */
        RestoreIRQMask(mask);
        return;
    }
    node = free_node;
    free_node = free_node->next;
    RestoreIRQMask(mask);

    CurrentTaskName=osThreadGetName (osThreadGetId());

    memcpy(node->task_name, (void *)CurrentTaskName, 8);

    node->task_name[7] = '\0';
    node->memptr = buffer;
    node->funcptr = func_lr;
    node->length = length;
    node->next = NULL;

#if MM_TRACE_ON == 1
    ECPLAT_PRINTF(UNILOG_PLA_STRING, mm_malloc_trace_0, P_SIG, "task_name: %s, funclr=0x%x", node->task_name, func_lr);
#endif

    /* add to mem trace node hash list */
    index = ((unsigned long)buffer) % MM_TRACE_HASH_SIZE;

    mask = SaveAndSetIRQMask();

    if (node_hash[index] != NULL)
    {
        /* add node to the tail*/
        struct mm_trace_node* tail;

        /* find tail of hash node list */
        tail = node_hash[index];
        while (tail->next != NULL)
            tail = tail->next;

        tail->next = node;
        EC_ASSERT(tail!= node, tail->next, node, index);
    }
    else
    {
        /* empty hash list, add to head */
        node_hash[index] = node;
    }
    RestoreIRQMask(mask);

#if MM_TRACE_ON == 1
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, mm_malloc_trace_1, P_SIG, "allocate mem @ 0x%x, len %d,node 0x%x, index %d", node->memptr,node->length,node,index);
#elif MM_TRACE_ON == 2
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, mm_malloc_trace_export, P_WARNING, ",%s,0x%x,%d,0x%x,0x%x,", node->task_name, node->memptr, node->length, node, func_lr);
#endif
}



/*----------------------------------------------------------------------------
 Brief:          add memory management free trace
 Details:        remove malloc info when free called
 Input:          free buffer ptr
 Output:         none
 Note:           internal use
 History:         04/02/2018    Originated by bchang
------------------------------------------------------------------------------*/
MMDEBUG_TEXT_SECTION void mm_free_trace(void* buffer)
{
    struct mm_trace_node* node;
    unsigned long index;

    /* get mm node */
    index = ((unsigned long)buffer) % MM_TRACE_HASH_SIZE;


    if (node_hash[index] != NULL)
    {
        uint32_t mask = SaveAndSetIRQMask();

        node = node_hash[index];
        if (node->memptr == buffer)
        {
            /* free node from list */
            node_hash[index] = node->next;
            RestoreIRQMask(mask);
        }
        else
        {

            while (node->next != NULL && node->next->memptr != buffer)

                node = node->next;

            /* free node from list */
            if (node->next != NULL)
            {
                struct mm_trace_node* tmp;

                tmp = node->next;
                node->next = node->next->next;
                RestoreIRQMask(mask);

                node = tmp;
            }
            else
            {
                /* not match ptr found */
                #if (MM_TRACE_ON != 0) 
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, mm_free_trace_0, P_SIG, "No match block 0x%x, ptr 0x%x\r\n", node->memptr,buffer);
                #endif
                RestoreIRQMask(mask);
                return;
            }
        }

#if MM_TRACE_ON == 1
        ECPLAT_PRINTF(UNILOG_PLA_STRING, mm_free_trace_1, P_SIG, "task_name: %s", node->task_name);
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, mm_free_trace_2, P_SIG, "free mem 0x%x, len %d,node 0x%x, index %d", node->memptr, node->length,node,index);
#elif MM_TRACE_ON == 2
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, mm_free_trace_export, P_WARNING, ",%s,0x%x,%d,0x%x,        ,", node->task_name, node->memptr, node->length, node);
#endif
        /* clear node */
        memset(node, 0, sizeof(struct mm_trace_node));

        /* add node to free list */
        mask = SaveAndSetIRQMask();
        node->next = free_node;
        free_node = node;
        RestoreIRQMask(mask);

    }
}



/*----------------------------------------------------------------------------
 Brief:          dump all memory management node
 Details:        only none free buffer was recorded and dumped
 Input:          none
 Output:         none
 Note:           internal use
 History:         04/02/2018    Originated by bchang
------------------------------------------------------------------------------*/
MMDEBUG_TEXT_SECTION void show_mem_trace(void)
{
    unsigned long index;
    struct mm_trace_node* node;

    for (index = 0; index < MM_TRACE_MAX; index ++)
    {
        node = &trace_node[index];

        /* dump memory trace item */
        if (node->task_name[0] != '\0')
        {
            #if MM_TRACE_ON == 1
            ECPLAT_PRINTF(UNILOG_PLA_STRING, show_mem_trace_0, P_INFO, "task_name: %s", node->task_name);
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, show_mem_trace_1, P_INFO, "malloc %d bytes @ 0x%x\r\n", node->length, node->memptr);
            #endif
        }
        
    }
}


