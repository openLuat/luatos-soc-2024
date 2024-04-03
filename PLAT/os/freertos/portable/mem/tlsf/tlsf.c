#include <limits.h>
#include <string.h>
#include "FreeRTOS.h"
#if( configSUPPORT_DYNAMIC_ALLOC_HEAP == 6 )
#include "sctdef.h"
#include "task.h"
#include "mm_debug.h"//add for memory leak debug
#include "exception_process.h"
#include "cmsis_compiler.h"
#include "tlsf.h"
#include "cmsis_os2.h"

extern void * osThreadGetId (void);

#if defined(__cplusplus)
#define tlsf_decl inline
#else
#define tlsf_decl static
#endif

#define TLSF_PRINT_REDECLARE  1
#if TLSF_PRINT_REDECLARE
typedef void(* tlsf_printcallback)(char *pbuf);
static tlsf_printcallback g_tlsf_printcallback = NULL;
#endif
/*
** Architecture-specific bit manipulation routines.
**
** TLSF achieves O(1) cost for malloc and free operations by limiting
** the search for a free block to a free list of guaranteed size
** adequate to fulfill the request, combined with efficient free list
** queries using bitmasks and architecture-specific bit-manipulation
** routines.
**
** Most modern processors provide instructions to count leading zeroes
** in a word, find the lowest and highest set bit, etc. These
** specific implementations will be used when available, falling back
** to a reasonably efficient generic implementation.
**
** NOTE: TLSF spec relies on ffs/fls returning value 0..31.
** ffs/fls return 1-32 by default, returning 0 for error.
*/

/*
** Detect whether or not we are building for a 32- or 64-bit (LP/LLP)
** architecture. There is no reliable portable method at compile-time.
*/
#if defined (__alpha__) || defined (__ia64__) || defined (__x86_64__) \
    || defined (_WIN64) || defined (__LP64__) || defined (__LLP64__)
#define TLSF_64BIT
#endif

#if (portBYTE_ALIGNMENT == 8)
#define TLSF_ALIGN_64BIT
#endif

/*
** gcc 3.4 and above have builtin support, specialized for architecture.
** Some compilers masquerade as gcc; patchlevel test filters them out.
*/
#if defined (__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) \
    && defined (__GNUC_PATCHLEVEL__)

#if defined (__SNC__)
/* SNC for Playstation 3. */

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __builtin_clz(reverse);
    return bit - 1;
}

#else

FREERTOS_TLSF_TEXT_SECTION tlsf_decl int tlsf_ffs(unsigned int word)
{
    return __builtin_ffs(word) - 1;
}

#endif

FREERTOS_TLSF_TEXT_SECTION tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}

#elif defined (_MSC_VER) && (_MSC_VER >= 1400) && (defined (_M_IX86) || defined (_M_X64))
/* Microsoft Visual C++ support on x86/X64 architectures. */

#include <intrin.h>

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

tlsf_decl int tlsf_fls(unsigned int word)
{
    unsigned long index;
    return _BitScanReverse(&index, word) ? index : -1;
}

tlsf_decl int tlsf_ffs(unsigned int word)
{
    unsigned long index;
    return _BitScanForward(&index, word) ? index : -1;
}

#elif defined (_MSC_VER) && defined (_M_PPC)
/* Microsoft Visual C++ support on PowerPC architectures. */

#include <ppcintrinsics.h>

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = 32 - _CountLeadingZeros(word);
    return bit - 1;
}

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - _CountLeadingZeros(reverse);
    return bit - 1;
}

#elif defined (__ARMCC_VERSION)
/* RealView Compilation Tools for ARM */

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __clz(reverse);
    return bit - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = word ? 32 - __clz(word) : 0;
    return bit - 1;
}

#elif defined (__ghs__)
/* Green Hills support for PowerPC */

#include <ppc_ghs.h>

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __CLZ32(reverse);
    return bit - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = word ? 32 - __CLZ32(word) : 0;
    return bit - 1;
}

#else
/* Fall back to generic implementation. */

FREERTOS_TLSF_TEXT_SECTION tlsf_decl int tlsf_fls_generic(unsigned int word)
{
    int bit = 32;

    if (!word) bit -= 1;
    if (!(word & 0xffff0000)) { word <<= 16; bit -= 16; }
    if (!(word & 0xff000000)) { word <<= 8; bit -= 8; }
    if (!(word & 0xf0000000)) { word <<= 4; bit -= 4; }
    if (!(word & 0xc0000000)) { word <<= 2; bit -= 2; }
    if (!(word & 0x80000000)) { word <<= 1; bit -= 1; }

    return bit;
}

/* Implement ffs in terms of fls. */
FREERTOS_TLSF_TEXT_SECTION tlsf_decl int tlsf_ffs(unsigned int word)
{
    return tlsf_fls_generic(word & (~word + 1)) - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
    return tlsf_fls_generic(word) - 1;
}

#endif

/* Possibly 64-bit version of tlsf_fls. */
#if defined (TLSF_64BIT)
tlsf_decl int tlsf_fls_sizet(size_t size)
{
    int high = (int)(size >> 32);
    int bits = 0;
    if (high)
    {
        bits = 32 + tlsf_fls(high);
    }
    else
    {
        bits = tlsf_fls((int)size & 0xffffffff);

    }
    return bits;
}
#else
#define tlsf_fls_sizet tlsf_fls
#endif

#undef tlsf_decl

/*
** Constants.
*/

/* Public constants: may be modified. */
enum tlsf_public
{
    /* log2 of number of linear subdivisions of block sizes. Larger
    ** values require more memory in the control structure. Values of
    ** 4 or 5 are typical.
    */
    SL_INDEX_COUNT_LOG2 = 2,
};

/* Private constants: do not modify. */
enum tlsf_private
{
#if defined (TLSF_ALIGN_64BIT)
    /* All allocation sizes and addresses are aligned to 8 bytes. */
    ALIGN_SIZE_LOG2 = 3,
#else
    /* All allocation sizes and addresses are aligned to 4 bytes. */
    ALIGN_SIZE_LOG2 = 2,
#endif
    ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

    /*
    ** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
    ** However, because we linearly subdivide the second-level lists, and
    ** our minimum size granularity is 4 bytes, it doesn't make sense to
    ** create first-level lists for sizes smaller than SL_INDEX_COUNT * 4,
    ** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
    ** trying to split size ranges into more slots than we have available.
    ** Instead, we calculate the minimum threshold size, and place all
    ** blocks below that size into the 0th first-level list.
    */

#if defined (TLSF_ALIGN_64BIT)
    /*
    ** TODO: We can increase this to support larger sizes, at the expense
    ** of more overhead in the TLSF structure.
    */
    #ifdef CORE_IS_CP     /* CP */
    FL_INDEX_MAX = 17,
    #else                 /* AP */
    FL_INDEX_MAX = 19,
    #endif
#else
    #ifdef CORE_IS_CP     /* CP */
    FL_INDEX_MAX = 17,
    #else                 /* AP */
#ifdef __USER_CODE__
	FL_INDEX_MAX = 21,
#else
    FL_INDEX_MAX = 19,
#endif
    #endif
#endif
    SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
    FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
    FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

    SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};

/*
** Cast and min/max macros.
*/
#define tlsf_cast(t, exp)   ((t) (exp))
#define tlsf_min(a, b)      ((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)      ((a) > (b) ? (a) : (b))

/*
** Set memory protect & deprotect API.
*/
#define tlsf_mem_protect() //vTaskSuspendAll()
#define tlsf_mem_deprotect() //xTaskResumeAll()

/*
** Set assert macro, if it has not been provided by the user.
*/
#if !defined (tlsf_assert)
#define tlsf_assert configASSERT
#endif

/*
** Static assertion mechanism.
*/
#define _tlsf_glue(x, y) x ## y
#define tlsf_static_assert(exp) \
    typedef char _tlsf_glue(static_assert, __LINE__) [(exp) ? 1 : -1]

/* This code has been tested on 32- and 64-bit (LP/LLP) architectures. */
tlsf_static_assert(sizeof(int) * CHAR_BIT == 32);
tlsf_static_assert(sizeof(size_t) * CHAR_BIT >= 32);
tlsf_static_assert(sizeof(size_t) * CHAR_BIT <= 64);

/* SL_INDEX_COUNT must be <= number of bits in sl_bitmap's storage type. */
tlsf_static_assert(sizeof(unsigned int) * CHAR_BIT >= SL_INDEX_COUNT);

/* Ensure we've properly tuned our sizes. */
tlsf_static_assert(ALIGN_SIZE == SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
tlsf_static_assert(ALIGN_SIZE == portBYTE_ALIGNMENT);

/*
** Data structures and associated constants.
*/

#ifdef MEM_BLK_SIZE_32BIT
/* do nothing */
#else
/*
** If a block is used/busy, and the wanted size will never be LE than 64KB,
** therefore, its 'size' field will be split into 2 parts:
** - High 16 bits(31~16) represents the size that user wanted,
** - Low 16 bits(15~0) represents the size of allocated block.
*/
#define BLOCK_WANTED_SIZE_MASK  0xffff0000
#define BLOCK_ALLOC_SIZE_MASK   0x0000ffff
#endif
#define BLOCK_SIZE_FULL_MASK    0xffffffff

/*
** For block boundary checking
*/
#if defined (TLSF_ALIGN_64BIT)
#ifdef MEM_BLK_SIZE_32BIT
#define BLOCK_HEAD_BOUNDARY_SIZE  4
#else
#define BLOCK_HEAD_BOUNDARY_SIZE  8
#endif
#define BLOCK_TAIL_BOUNDARY_SIZE  8

#else
#define BLOCK_HEAD_BOUNDARY_SIZE  4
#define BLOCK_TAIL_BOUNDARY_SIZE  4
#endif
typedef unsigned char block_head_boundary_t[BLOCK_HEAD_BOUNDARY_SIZE];
typedef unsigned char block_tail_boundary_t[BLOCK_TAIL_BOUNDARY_SIZE];
/*
** Block header structure.
**
** There are several implementation subtleties involved:
** - The prev_phys_block field is only valid if the previous block is free.
** - The prev_phys_block field is actually stored at the end of the
**   previous block. It appears at the beginning of this structure only to
**   simplify the implementation.
** - The next_free / prev_free fields are only valid if the block is free.
*/
typedef struct block_header_t
{
    /* Points to the previous physical block. */
    struct block_header_t* prev_phys_block;

#ifdef MM_DEBUG_EN
#ifdef MM_HEAD_BOUNDARY
    block_head_boundary_t head_bound;
#endif
    size_t alloc_owner;
#endif
#ifdef MEM_BLK_SIZE_32BIT
    size_t size_wanted;
#else
    /*
    ** - If block is used, only low 16 bits represents the block size,
    **   while high 16 bits represents the size user wanted.
    ** - If block is free, all 32 bits represents this block size.
    */
#endif
    /* The size of this block, including block tail, but excluding the header. */
    size_t size;

    /* Next and previous free blocks. */
    struct block_header_t* next_free;
    struct block_header_t* prev_free;
} block_header_t;

/*
** For every allocated block, an extra boundray is appended to the tail
** to check the boundary corrupted or not.
*/
typedef block_tail_boundary_t  block_tail_t;

/*
** Since block sizes are always at least a multiple of 4, the two least
** significant bits of the size field are used to store the block status:
** - bit 0: whether block is busy or free
** - bit 1: whether previous block is busy or free
*/
static const size_t block_header_free_bit      = 1 << 0;
static const size_t block_header_prev_free_bit = 1 << 1;

/*
** Empiric value of device, excluded block tail
** unit: byte
*/
#define BLOCK_USER_BUF_SPLIT_THRESHOLD    12

/* User data starts directly after the 'size' field in a used block. */
#define BLOCK_USER_BUF_OFFSET      (offsetof(block_header_t, size) + sizeof(size_t))

/*
** The size of the block header exposed to used blocks is the size field.
** The 'prev_phys_block' field is stored *inside* the previous free block.
*/
#define BLOCK_HEADER_OVERHEAD      (BLOCK_USER_BUF_OFFSET - sizeof(block_header_t*))

#ifdef MM_DEBUG_EN
/*
** The value of boundary is a constant(head: 'beafdead', tail: 'deadbeaf'),
** And it should never be modified!
*/
#define BLOCK_HEAD_BOUNDARY  0xbeafdead
#define BLOCK_TAIL_BOUNDARY  0xdeadbeaf

/*
** If block tail exposed to used blocks exists,
** Its size is equal to the block_boundary('deadbeaf').
*/
#define BLOCK_TAIL_OVERHEAD  sizeof(block_tail_t)
#else
/* No boundary, no tail! */
#define BLOCK_TAIL_OVERHEAD  0
#endif

/* Threshold value: the extra free size in a block that can be trimmed off! */
#define BLOCK_SPLIT_THRESHOLD  (BLOCK_HEADER_OVERHEAD + \
                                BLOCK_TAIL_OVERHEAD + \
                                BLOCK_USER_BUF_SPLIT_THRESHOLD)

/* for fast accessing */
static const size_t block_start_offset = BLOCK_USER_BUF_OFFSET;
static const size_t block_header_overhead = BLOCK_HEADER_OVERHEAD;
static const size_t block_tail_overhead = BLOCK_TAIL_OVERHEAD;
static const size_t block_split_threshold = BLOCK_SPLIT_THRESHOLD;

/*
** A free block must be large enough to store its header and tail(if exists) minus the size
** of 'prev_phys_block' field, and no larger than the number of addressable bits for FL_INDEX.
*/
static const size_t block_size_min = sizeof(block_header_t) - sizeof(block_header_t*) + BLOCK_TAIL_OVERHEAD;
static const size_t block_size_max = tlsf_cast(size_t, 1) << FL_INDEX_MAX;

/* Record some info of pools: starting addr, size and etc. */
#define POOL_CNT_MAX    1
typedef struct
{
    unsigned int cnt;
    size_t size_total;
    struct pool_node
    {
        size_t size;
        pool_t start;
    }node[POOL_CNT_MAX];
}pool_group_t;

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
#define POOL_GROUP_NUM    2
#else
#define POOL_GROUP_NUM    1
#endif

static pool_group_t pool_group[POOL_GROUP_NUM];
uint8_t gucPoolGroupSel = 0; /* 0 is for SRAM, 1 is for PSRAM */

#ifdef HEAP_MEM_DEBUG
typedef struct
{
    unsigned int cnt_alloc;
    unsigned int cnt_free;
    unsigned int cnt_peak_used;
}block_record_t;

typedef struct
{
    size_t size_real_wanted;
    size_t size_total_alloc;
    size_t size_peak_alloc;
    unsigned int cnt_total_alloc;
    unsigned int cnt_total_free;
    unsigned int hist_min_free;
}mem_record_t;
#endif

/* The TLSF control structure. */
typedef struct control_t
{
    /* Empty lists point at this block to indicate they are free. */
    block_header_t block_null;

    /* Bitmaps for free lists. */
    unsigned int fl_bitmap;
    unsigned int sl_bitmap[FL_INDEX_COUNT];

    /* Head of free lists. */
    block_header_t* blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];

#ifdef HEAP_MEM_DEBUG
    /* debug statistic */
    mem_record_t    mem_rec;
#ifdef MEM_BLK_STAT
    block_record_t  blk_rec[FL_INDEX_COUNT][SL_INDEX_COUNT];
#endif
#endif
} control_t;

/* A type used for casting when doing pointer arithmetic. */
typedef ptrdiff_t tlsfptr_t;

/*
** block_header_t member functions.
*/
FREERTOS_TLSF_TEXT_SECTION static int block_is_free(const block_header_t* block)
{
    return tlsf_cast(int, block->size & block_header_free_bit);
}

FREERTOS_TLSF_TEXT_SECTION static int block_is_prev_free(const block_header_t* block)
{
    return tlsf_cast(int, block->size & block_header_prev_free_bit);
}

FREERTOS_TLSF_TEXT_SECTION static size_t block_size(const block_header_t* block)
{
#ifdef MEM_BLK_SIZE_32BIT
    size_t size_mask = BLOCK_SIZE_FULL_MASK;
#else
    size_t size_mask = block_is_free(block) ? BLOCK_SIZE_FULL_MASK : BLOCK_ALLOC_SIZE_MASK;
#endif

    return (block->size & ~(block_header_free_bit | block_header_prev_free_bit)) & size_mask;
}

FREERTOS_TLSF_TEXT_SECTION static void block_set_size(block_header_t* block, size_t size)
{
    const size_t oldsize = block->size;
    block->size = size | (oldsize & (block_header_free_bit | block_header_prev_free_bit));
}

FREERTOS_TLSF_TEXT_SECTION static size_t block_size_wanted(const block_header_t* block)
{
    tlsf_assert(!block_is_free(block) && "block must not be free");

#ifdef MEM_BLK_SIZE_32BIT
    return block->size_wanted;
#else
    return block_is_free(block) ? 0 : ((block->size & BLOCK_WANTED_SIZE_MASK) >> 16);
#endif
}

FREERTOS_TLSF_TEXT_SECTION static void block_set_size_wanted(block_header_t* block, size_t size)
{
    tlsf_assert(!block_is_free(block) && "block must not be free");

#ifdef MEM_BLK_SIZE_32BIT
    block->size_wanted = size;
#else
    block->size = (block->size & BLOCK_ALLOC_SIZE_MASK) | (size << 16);
#endif
}

FREERTOS_TLSF_TEXT_SECTION static int block_is_last(const block_header_t* block)
{
    return block_size(block) == 0;
}

FREERTOS_TLSF_TEXT_SECTION static void block_set_free(block_header_t* block)
{
    block->size |= block_header_free_bit;
}

FREERTOS_TLSF_TEXT_SECTION static void block_set_used(block_header_t* block)
{
    block->size &= ~block_header_free_bit;
}

FREERTOS_TLSF_TEXT_SECTION static void block_set_prev_free(block_header_t* block)
{
    block->size |= block_header_prev_free_bit;
}

FREERTOS_TLSF_TEXT_SECTION static void block_set_prev_used(block_header_t* block)
{
    block->size &= ~block_header_prev_free_bit;
}

FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_from_ptr(const void* ptr)
{
    return tlsf_cast(block_header_t*, tlsf_cast(unsigned char*, ptr) - block_start_offset);
}

FREERTOS_TLSF_TEXT_SECTION static void* block_to_ptr(const block_header_t* block)
{
    return tlsf_cast(void*, tlsf_cast(unsigned char*, block) + block_start_offset);
}

/* Return location of next block after block of given size. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* offset_to_block(const void* ptr, int size)
{
    return tlsf_cast(block_header_t*, tlsf_cast(tlsfptr_t, ptr) + size);
}

/* Return location of previous block. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_prev(const block_header_t* block)
{
    tlsf_assert(block_is_prev_free(block) && "previous block must be free");
    return block->prev_phys_block;
}

/* Return location of next existing block. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_next(const block_header_t* block)
{
    block_header_t* next = offset_to_block(block_to_ptr(block), block_size(block) - sizeof(block_header_t*));
    tlsf_assert(!block_is_last(block));
    return next;
}

/* Link a new block with its physical neighbor, return the neighbor. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_link_next(block_header_t* block)
{
    block_header_t* next = block_next(block);
    next->prev_phys_block = block;
    return next;
}

FREERTOS_TLSF_TEXT_SECTION static void block_mark_as_free(block_header_t* block)
{
    block_header_t* next = NULL;

    /* set block free first! */
    block_set_free(block);

    next = block_link_next(block);
    block_set_prev_free(next);
}

FREERTOS_TLSF_TEXT_SECTION static void block_mark_as_used(block_header_t* block)
{
    block_header_t* next = NULL;

    block_set_used(block);

    next = block_next(block);
    block_set_prev_used(next);
}

FREERTOS_TLSF_TEXT_SECTION static size_t align_up(size_t x, size_t align)
{
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return (x + (align - 1)) & ~(align - 1);
}

FREERTOS_TLSF_TEXT_SECTION static size_t align_down(size_t x, size_t align)
{
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return x - (x & (align - 1));
}

FREERTOS_TLSF_TEXT_SECTION static void* align_ptr(const void* ptr, size_t align)
{
    const tlsfptr_t aligned = (tlsf_cast(tlsfptr_t, ptr) + (align - 1)) & ~(align - 1);
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return tlsf_cast(void*, aligned);
}

/*
** Adjust an allocation size to be aligned to word size, and no smaller
** than internal minimum.
*/
FREERTOS_TLSF_TEXT_SECTION static size_t adjust_request_size(size_t size, size_t align)
{
    size_t adjust = 0;

    if (size)
    {
        const size_t aligned = align_up(size, align);

        /* aligned sized must not exceed block_size_max or we'll go out of bounds on sl_bitmap
        ** and the adjusted size includes the size of block tail.
         */
        if (aligned < block_size_max)
        {
            adjust = tlsf_max((aligned + block_tail_overhead), (block_size_min - block_header_overhead));
        }
    }

    return adjust;
}

/*
** TLSF utility functions. In most cases, these are direct translations of
** the documentation found in the white paper.
*/

FREERTOS_TLSF_TEXT_SECTION static void mapping_insert(size_t size, int* fli, int* sli)
{
    int fl, sl;

    if (size < SMALL_BLOCK_SIZE)
    {
        /* Store small blocks in first list. */
        fl = 0;
        sl = tlsf_cast(int, size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    }
    else
    {
        fl = tlsf_fls_sizet(size);
        sl = tlsf_cast(int, size >> (fl - SL_INDEX_COUNT_LOG2)) ^ (1 << SL_INDEX_COUNT_LOG2);
        fl -= (FL_INDEX_SHIFT - 1);
    }
    *fli = fl;
    *sli = sl;
}

/* This version rounds up to the next block size (for allocations) */
FREERTOS_TLSF_TEXT_SECTION static void mapping_search(size_t size, int* fli, int* sli)
{
    if (size >= SMALL_BLOCK_SIZE)
    {
        const size_t round = (1 << (tlsf_fls_sizet(size) - SL_INDEX_COUNT_LOG2)) - 1;
        size += round;
    }

    mapping_insert(size, fli, sli);
}

FREERTOS_TLSF_TEXT_SECTION static block_header_t* search_suitable_block(control_t* control, int* fli, int* sli)
{
    int fl = *fli;
    int sl = *sli;

    /*
    ** First, search for a block in the list associated with the given
    ** fl/sl index.
    */
    unsigned int sl_map = control->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map)
    {
        /* No block exists. Search in the next largest first-level list. */
        const unsigned int fl_map = control->fl_bitmap & (~0U << (fl + 1));
        if (!fl_map)
        {
            /* No free blocks available, memory has been exhausted. */
            return 0;
        }

        fl = tlsf_ffs(fl_map);
        *fli = fl;
        sl_map = control->sl_bitmap[fl];
    }

    tlsf_assert(sl_map && "internal error - second level bitmap is null");
    sl = tlsf_ffs(sl_map);
    *sli = sl;

    /* Return the first block in the free list. */
    return control->blocks[fl][sl];
}

/* Remove a free block from the free list.*/
FREERTOS_TLSF_TEXT_SECTION static void remove_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
    block_header_t* prev = block->prev_free;
    block_header_t* next = block->next_free;

    tlsf_assert((prev && next) && "prev_free & next_free field can not be null");

    next->prev_free = prev;
    prev->next_free = next;

    /* If this block is the head of the free list, set new head. */
    if (control->blocks[fl][sl] == block)
    {
        control->blocks[fl][sl] = next;

        /* If the new head is null, clear the bitmap. */
        if (next == &control->block_null)
        {
            control->sl_bitmap[fl] &= ~(1U << sl);

            /* If the second bitmap is now empty, clear the fl bitmap. */
            if (!control->sl_bitmap[fl])
            {
                control->fl_bitmap &= ~(1U << fl);
            }
        }
    }
}

/* Insert a free block into the free block list. */
FREERTOS_TLSF_TEXT_SECTION static void insert_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
    block_header_t* current = control->blocks[fl][sl];

    tlsf_assert(current && block && "free list cannot have a null entry & insert a null entry into it");

    block->next_free = current;
    block->prev_free = &control->block_null;
    current->prev_free = block;

    tlsf_assert(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE) && "block not aligned properly");
    /*
    ** Insert the new block at the head of the list, and mark the first-
    ** and second-level bitmaps appropriately.
    */
    control->blocks[fl][sl] = block;
    control->fl_bitmap |= (1U << fl);
    control->sl_bitmap[fl] |= (1U << sl);
}

/* Remove a given block from the free list. */
FREERTOS_TLSF_TEXT_SECTION static void block_remove(control_t* control, block_header_t* block)
{
    int fl, sl;

    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

/* Insert a given block into the free list. */
FREERTOS_TLSF_TEXT_SECTION static void block_insert(control_t* control, block_header_t* block)
{
    int fl, sl;

    mapping_insert(block_size(block), &fl, &sl);
    insert_free_block(control, block, fl, sl);
}

FREERTOS_TLSF_TEXT_SECTION static int block_can_split(block_header_t* block, size_t size)
{
    return block_size(block) >= block_split_threshold + size;
}

/* Split a block into two, the second of which is free. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_split(block_header_t* block, size_t size)
{
    /* Calculate the amount of space left in the remaining block. */
    block_header_t* remaining = offset_to_block(block_to_ptr(block), size - sizeof(block_header_t*));

    const size_t remain_size = block_size(block) - (size + block_header_overhead);
    //const size_t remain_size = (block->size & ~(block_header_free_bit | block_header_prev_free_bit)) - (size + block_header_overhead);

    tlsf_assert(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE) && "remaining block not aligned properly");

    block_set_size(remaining, remain_size);
    block_mark_as_free(remaining);

    tlsf_assert((block_size(remaining) + block_header_overhead >= block_split_threshold) && "block split with invalid size");

    block_set_size(block, size);

    return remaining;
}

/* Absorb a free block's storage into an adjacent previous free block. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_absorb(block_header_t* prev, block_header_t* block)
{
    tlsf_assert(!block_is_last(prev) && "previous block can't be last");

    /* Note: Leaves flags untouched. */
    //prev->size += block_size(block) + block_header_overhead;
    prev->size = (block_size(prev) + block_size(block) + block_header_overhead) | (prev->size & (block_header_free_bit | block_header_prev_free_bit));
    block_link_next(prev);
    return prev;
}

/* Merge a just-freed block with an adjacent previous free block. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_merge_prev(control_t* control, block_header_t* block)
{
    if (block_is_prev_free(block))
    {
        block_header_t* prev = block_prev(block);

        tlsf_assert((prev && block_is_free(prev)) && "prev physical block can't be null & is not free though marked as such");

        block_remove(control, prev);
        block = block_absorb(prev, block);
    }

    return block;
}

/* Merge a just-freed block with an adjacent free block. */
FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_merge_next(control_t* control, block_header_t* block)
{
    block_header_t* next = block_next(block);
    tlsf_assert(next && "next physical block can't be null");

    if (block_is_free(next))
    {
        tlsf_assert(!block_is_last(block) && "previous block can't be last");
        block_remove(control, next);
        block = block_absorb(block, next);
    }

    return block;
}

/* Trim any trailing block space off the end of a block, return to pool. */
FREERTOS_TLSF_TEXT_SECTION static void block_trim_free(control_t* control, block_header_t* block, size_t size)
{
    tlsf_assert(block_is_free(block) && "block must be free");

    if (block_can_split(block, size))
    {
        block_header_t* remaining_block = block_split(block, size);
        block_link_next(block);
        block_set_prev_free(remaining_block);
        block_insert(control, remaining_block);
    }
}

/* Trim any trailing block space off the end of a used block, return to pool. */
FREERTOS_TLSF_TEXT_SECTION static void block_trim_used(control_t* control, block_header_t* block, size_t size, uint8_t expand)
{
    tlsf_assert(!block_is_free(block) && "block must be used");

    if(expand) block_set_free(block);

    if (block_can_split(block, size))
    {
        /* If the next block is free, we must coalesce. */
        block_header_t* remaining_block = block_split(block, size);
        block_set_prev_used(remaining_block);

        remaining_block = block_merge_next(control, remaining_block);
        block_insert(control, remaining_block);
    }

    if(expand) block_set_used(block);
}

FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_trim_free_leading(control_t* control, block_header_t* block, size_t size)
{
    block_header_t* remaining_block = block;

    if (block_can_split(block, size))
    {
        /* We want the 2nd block. */
        remaining_block = block_split(block, size - block_header_overhead);
        block_set_prev_free(remaining_block);

        block_link_next(block);
        block_insert(control, block);
    }

    return remaining_block;
}

FREERTOS_TLSF_TEXT_SECTION static block_header_t* block_locate_free(control_t* control, size_t size)
{
    int fl = 0, sl = 0;
    block_header_t* block = 0;

    if (size)
    {
        mapping_search(size, &fl, &sl);

        /*
        ** mapping_search can futz with the size, so for excessively large sizes it can sometimes wind up
        ** with indices that are off the end of the block array.
        ** So, we protect against that here, since this is the only callsite of mapping_search.
        ** Note that we don't need to check sl, since it comes from a modulo operation that guarantees it's always in range.
        */
        if (fl < FL_INDEX_COUNT)
        {
            block = search_suitable_block(control, &fl, &sl);
        }
    }

    if (block)
    {
        tlsf_assert(block_size(block) >= size && "the size of found block is less than wanted!");

        remove_free_block(control, block, fl, sl);
    }

    return block;
}

FREERTOS_TLSF_TEXT_SECTION static void* block_prepare_used(control_t* control, block_header_t* block, size_t size)
{
    void* p = 0;

    if (block)
    {
        tlsf_assert(size && "size must be non-zero");

        block_trim_free(control, block, size);
        block_mark_as_used(block);
        p = block_to_ptr(block);
    }
    return p;
}

/* Clear structure and point all empty lists at the null block. */
FREERTOS_TLSF_TEXT_SECTION static void control_construct(control_t* control)
{
    int i, j;

    /* reset dummy 'block_null' */
    memset(&control->block_null, 0, sizeof(control->block_null));
    control->block_null.next_free = &control->block_null;
    control->block_null.prev_free = &control->block_null;

    control->fl_bitmap = 0;
    for (i = 0; i < FL_INDEX_COUNT; ++i)
    {
        control->sl_bitmap[i] = 0;
        for (j = 0; j < SL_INDEX_COUNT; ++j)
        {
            control->blocks[i][j] = &control->block_null;
        }
    }

#ifdef HEAP_MEM_DEBUG
    tlsf_clear_mem_record(tlsf_cast(tlsf_t, control));
#endif
}

/*
** Debugging utilities.
*/

typedef struct integrity_t
{
    int prev_status;
    int status;
} integrity_t;

#define tlsf_insist(x) { tlsf_assert(x); if (!(x)) { status--; } }


#if 0// avoid warnning
static void integrity_walker(void* ptr, size_t size, int used, void* user)
{
    block_header_t* block = block_from_ptr(ptr);
    integrity_t* integ = tlsf_cast(integrity_t*, user);
    const int this_prev_status = block_is_prev_free(block) ? 1 : 0;
    const int this_status = block_is_free(block) ? 1 : 0;
    const size_t this_block_size = block_size(block);

    int status = 0;
    (void)used;
    tlsf_insist(integ->prev_status == this_prev_status && "prev status incorrect");
    tlsf_insist(size == this_block_size && "block size incorrect");

    integ->prev_status = this_status;
    integ->status += status;
}
#endif
FREERTOS_TLSF_TEXT_SECTION int tlsf_check(tlsf_t tlsf)
{
    int i, j;

    control_t* control = tlsf_cast(control_t*, tlsf);
    int status = 0;

    /* Check that the free lists and bitmaps are accurate. */
    for (i = 0; i < FL_INDEX_COUNT; ++i)
    {
        for (j = 0; j < SL_INDEX_COUNT; ++j)
        {
            const int fl_map = control->fl_bitmap & (1U << i);
            const int sl_list = control->sl_bitmap[i];
            const int sl_map = sl_list & (1U << j);
            const block_header_t* block = control->blocks[i][j];

            /* Check that first- and second-level lists agree. */
            if (!fl_map)
            {
                tlsf_insist(!sl_map && "second-level map must be null");
            }

            if (!sl_map)
            {
                tlsf_insist(block == &control->block_null && "block list must be null");
                continue;
            }

            /* Check that there is at least one free block. */
            tlsf_insist(sl_list && "no free blocks in second-level map");
            tlsf_insist(block != &control->block_null && "block should not be null");

            while (block != &control->block_null)
            {
                int fli, sli;
                tlsf_insist(block_is_free(block) && "block should be free");
                tlsf_insist(!block_is_prev_free(block) && "blocks should have coalesced");
                tlsf_insist(!block_is_free(block_next(block)) && "blocks should have coalesced");
                tlsf_insist(block_is_prev_free(block_next(block)) && "block should be free");
                tlsf_insist(block_size(block) >= block_size_min && "block not minimum size");

                mapping_insert(block_size(block), &fli, &sli);
                tlsf_insist(fli == i && sli == j && "block size indexed in wrong list");
                block = block->next_free;
            }
        }
    }

    return status;
}

#undef tlsf_insist



FREERTOS_TLSF_TEXT_SECTION static void default_walker(void* ptr, size_t size, int prev_used, int cur_used, size_t user)
{
#if TLSF_PRINT_REDECLARE
    char walk_buf[128];
    snprintf(walk_buf, 128, " 0x%x  |  0x%x  | 0x%08x |   %s    |   %s   | 0x%08x | %d \n", \
                  ptr, \
                  (unsigned char *)ptr + size, \
                  (unsigned int)size, \
                  prev_used ? "used" : "free", \
                  cur_used ? "used" : "free", \
                  user&0xFFFFFF, \
                  (user>>24)&0xFF);
    if(g_tlsf_printcallback != NULL)
        g_tlsf_printcallback(walk_buf);
#else
    printf("0x%p | 0x%p | 0x%08x |   %s    |   %s   | 0x%08x | %d \n", \
                  ptr, \
                  (unsigned char *)ptr + size, \
                  (unsigned int)size, \
                  prev_used ? "used" : "free", \
                  cur_used ? "used" : "free", \
                  user&0xFFFFFF, \
                  (user>>24)&0xFF);
#endif
}

FREERTOS_TLSF_TEXT_SECTION static void free_block_walker(void* ptr, size_t size, int prev_used, int cur_used, size_t user)
{
    if(!cur_used) default_walker(ptr, size, prev_used, 0, user);
}

FREERTOS_TLSF_TEXT_SECTION static void used_block_walker(void* ptr, size_t size, int prev_used, int cur_used, size_t user)
{
    if(cur_used) default_walker(ptr, size, prev_used, 1, user);
}

#if TLSF_PRINT_REDECLARE
FREERTOS_TLSF_TEXT_SECTION void tlsf_set_print_callback(void* cb)
{
    g_tlsf_printcallback = (tlsf_printcallback)cb;
}
#endif

FREERTOS_TLSF_TEXT_SECTION void tlsf_walk_pool(pool_t pool, tlsf_walker walker)
{
    tlsf_walker pool_walker = walker ? walker : default_walker;
    block_header_t* block = offset_to_block(pool, -(int)sizeof(block_header_t*));
    size_t user = 0;

    if(pool_walker == default_walker || \
       pool_walker == free_block_walker || \
       pool_walker == used_block_walker)
    {
#if TLSF_PRINT_REDECLARE
        char walk_buf[128];
        snprintf(walk_buf, 128, "start addr | end  addr  |    size    |prev-state |cur-state |   funcLr   | taskId \n");
        if(g_tlsf_printcallback != NULL)
            g_tlsf_printcallback(walk_buf);
#else
        printf("start addr | end  addr  |    size    |prev-state |cur-state |   funcLr   | taskId \n");
#endif
    }

    while (block && !block_is_last(block))
    {
        if(block_is_free(block))
        {
            user = 0;
        }
    #ifdef MM_DEBUG_EN
        else
        {
            user = block->alloc_owner;
        }
    #endif
        pool_walker(block_to_ptr(block),
                    block_size(block),
                    !block_is_prev_free(block),
                    !block_is_free(block),
                    user);
        block = block_next(block);
    }
}

FREERTOS_TLSF_TEXT_SECTION int tlsf_check_pool(pool_t pool, int block_type)
{
    /* Check that the blocks are physically correct. */
    if(block_type == 0)
    {
        tlsf_walk_pool(pool, free_block_walker);
    }
    else if(block_type == 1)
    {
        tlsf_walk_pool(pool, used_block_walker);
    }
    else
    {
        tlsf_walk_pool(pool, NULL);
    }

    return 0;
}


FREERTOS_TLSF_TEXT_SECTION void tlsf_loop_block_list(tlsf_t tlsf, int fl, int sl)
{
    int cnt = 0;
    control_t* control = tlsf_cast(control_t*, tlsf);
    block_header_t* block = control->blocks[fl][sl];
    int min_block_size, max_block_size;
    min_block_size = (1 << (fl + (FL_INDEX_SHIFT - 1))) + (1 << ((fl + (FL_INDEX_SHIFT - 1)) - SL_INDEX_COUNT_LOG2)) * sl;
    max_block_size = (1 << (fl + (FL_INDEX_SHIFT - 1))) + (1 << ((fl + (FL_INDEX_SHIFT - 1)) - SL_INDEX_COUNT_LOG2)) * (sl + 1);

#if TLSF_PRINT_REDECLARE
    char walk_buf[128];
    snprintf(walk_buf, 128, "block [%8d, %8d): \r\n", min_block_size, max_block_size);
    if(g_tlsf_printcallback != NULL)
        g_tlsf_printcallback(walk_buf);
#else
    printf("block [%8d, %8d): \r\n", min_block_size, max_block_size);
#endif

    while (block && !block_is_last(block))
    {
        if(block_is_free(block))
        {
            cnt++;
        }

#if TLSF_PRINT_REDECLARE
        snprintf(walk_buf, 128, "    address=0x%x    size=%8d  bytes\r\n", (int)block, block_size(block));
        if(g_tlsf_printcallback != NULL)
            g_tlsf_printcallback(walk_buf);
#else
        printf("    address=0x%x    size=%8d  bytes\r\n", (int)block, block->size);
#endif
        block = block->next_free;
    }

}


FREERTOS_TLSF_TEXT_SECTION void tlsf_show_block_detail(tlsf_t tlsf)
{
    int fl_map, sl_map;
    int fl, sl;
    control_t* control = tlsf_cast(control_t*, tlsf);
    fl_map = control->fl_bitmap;
    fl = tlsf_fls(fl_map);

    while(fl >= 0)
    {
        sl_map = control->sl_bitmap[fl];
        sl = tlsf_fls(sl_map);
        while(sl >= 0)
        {
            tlsf_loop_block_list(tlsf, fl, sl);
            sl_map &= ~(1<<sl);
            sl = tlsf_fls(sl_map);
        }
        fl_map &= ~(1<<fl);
        fl = tlsf_fls(fl_map);
    }
}


#ifdef HEAP_MEM_DEBUG
#ifdef MEM_BLK_STAT
FREERTOS_TLSF_TEXT_SECTION static void tlsf_record_block_alloc(block_record_t* blk_rec, size_t blk_size)
{
    int fl = 0, sl = 0;

    mapping_search(blk_size, &fl, &sl);

    blk_rec += fl * SL_INDEX_COUNT + sl;
    blk_rec->cnt_alloc ++;
    tlsf_assert(blk_rec->cnt_alloc >= blk_rec->cnt_free);
    if(blk_rec->cnt_alloc - blk_rec->cnt_free > blk_rec->cnt_peak_used)
    {
        blk_rec->cnt_peak_used = blk_rec->cnt_alloc - blk_rec->cnt_free;
    }
}

FREERTOS_TLSF_TEXT_SECTION static void tlsf_record_block_free(block_record_t* blk_rec, size_t blk_size)
{
    int fl = 0, sl = 0;

    mapping_search(blk_size, &fl, &sl);

    blk_rec += fl * SL_INDEX_COUNT + sl;
    blk_rec->cnt_free ++;
}

FREERTOS_TLSF_TEXT_SECTION int tlsf_show_block_record(block_record_t* blk_rec)
{
    block_record_t* tmp_rec = blk_rec;
    unsigned int cnt_cur_occupied = 0;
    int line = 0;

    printf("================================================================\n");

    for (int fl = 0; fl < FL_INDEX_COUNT; ++fl, line = 0)
    {
        for (int sl = 0; sl < SL_INDEX_COUNT; ++sl)
        {
            tmp_rec = blk_rec + fl * SL_INDEX_COUNT + sl;

            if(!tmp_rec->cnt_alloc && \
               !tmp_rec->cnt_free && \
               !tmp_rec->cnt_peak_used) continue;

            line = 1;
            if(tmp_rec->cnt_alloc > tmp_rec->cnt_free)
            {
                cnt_cur_occupied = tmp_rec->cnt_alloc - tmp_rec->cnt_free;
            }

            printf("block[%d][%d] records\n\
            *stat*| alloc cnt : %d \n\
                  | free  cnt : %d \n\
                  | cur-used  : %d blks\n\
                  | peak-used : %d blks\n",\
                    fl, sl,\
                    tmp_rec->cnt_alloc,\
                    tmp_rec->cnt_free,\
                    cnt_cur_occupied,\
                    tmp_rec->cnt_peak_used);
        }

        if(line) printf("----------------------------------------------------------------\n");
    }

    return 0;
}
#endif

FREERTOS_TLSF_TEXT_SECTION static void tlsf_record_mem_alloc(mem_record_t* mem_rec, block_header_t* block, size_t wanted_size)
{
    size_t blk_size = block_size(block);

    mem_rec->cnt_total_alloc ++;
    mem_rec->size_real_wanted += wanted_size;
    mem_rec->size_total_alloc += blk_size - block_tail_overhead;
    if(mem_rec->size_total_alloc > mem_rec->size_peak_alloc)
    {
        mem_rec->size_peak_alloc = mem_rec->size_total_alloc;
    }
}

FREERTOS_TLSF_TEXT_SECTION static void tlsf_record_hist_min_free(control_t* control)
{
    unsigned int fl_map, sl_map;
    unsigned int fl, sl;
    if(control->mem_rec.hist_min_free == 0)
    {
        control->mem_rec.hist_min_free = (FL_INDEX_COUNT << SL_INDEX_COUNT_LOG2)+SL_INDEX_COUNT;
    }
    else
    {
        fl_map = control->fl_bitmap;
        fl = tlsf_fls(fl_map);
        sl_map = control->sl_bitmap[fl];
        sl = tlsf_fls(sl_map);
        if(((fl<<SL_INDEX_COUNT_LOG2)+sl) < control->mem_rec.hist_min_free)
            control->mem_rec.hist_min_free = (fl<<SL_INDEX_COUNT_LOG2)+sl;
    }
}

FREERTOS_TLSF_TEXT_SECTION static void tlsf_show_hist_min_max_free_block_size(control_t* control, int *range)
{
    int min_block_size, max_block_size;
    unsigned int fl, sl;
    unsigned int hist_min_free;
    char walk_buf[128];

    hist_min_free = control->mem_rec.hist_min_free;
    fl = hist_min_free >> SL_INDEX_COUNT_LOG2;
    sl = hist_min_free - (fl << SL_INDEX_COUNT_LOG2);
    min_block_size = (1 << (fl + (FL_INDEX_SHIFT - 1))) + (1 << ((fl + (FL_INDEX_SHIFT - 1)) - SL_INDEX_COUNT_LOG2)) * sl;
    max_block_size = (1 << (fl + (FL_INDEX_SHIFT - 1))) + (1 << ((fl + (FL_INDEX_SHIFT - 1)) - SL_INDEX_COUNT_LOG2)) * (sl + 1);
    if(range == NULL)
    {
        #if TLSF_PRINT_REDECLARE
        snprintf(walk_buf, 128, "Min free block in histroy: [%d, %d)\n", min_block_size, max_block_size);
        if(g_tlsf_printcallback != NULL)
            g_tlsf_printcallback(walk_buf);
        #else
        printf("Min free block in histroy: [%d, %d)\n", min_block_size, max_block_size);
        #endif
    }
    else
    {
        range[0] = min_block_size;
        range[1] = max_block_size;
    }
}


FREERTOS_TLSF_TEXT_SECTION void tlsf_show_cur_max_free_size(control_t* control, int *range)
{
    int min_block_size, max_block_size, block_size_cur = 0;
    unsigned int fl, sl;
    char walk_buf[128];
    block_header_t* block;
    uint32_t mask;

    mask = SaveAndSetIRQMask();

    fl = tlsf_fls(control->fl_bitmap);
    sl = tlsf_fls(control->sl_bitmap[fl]);
    min_block_size = (1 << (fl + (FL_INDEX_SHIFT - 1))) + (1 << ((fl + (FL_INDEX_SHIFT - 1)) - SL_INDEX_COUNT_LOG2)) * sl;
    max_block_size = (1 << (fl + (FL_INDEX_SHIFT - 1))) + (1 << ((fl + (FL_INDEX_SHIFT - 1)) - SL_INDEX_COUNT_LOG2)) * (sl + 1);
    block = control->blocks[fl][sl];
    if(block_is_free(block))
    {
        block_size_cur = block_size(block);
    }
    RestoreIRQMask(mask);

    if(range == NULL)
    {
    #if TLSF_PRINT_REDECLARE
        snprintf(walk_buf, 128, "Current max free size: %d in [%d, %d)\n", block_size_cur, min_block_size, max_block_size);
        if(g_tlsf_printcallback != NULL)
            g_tlsf_printcallback(walk_buf);
    #else
        printf("Current max free size: %d in [%d, %d)\n", block_size_cur, min_block_size, max_block_size);
    #endif
    }
    else
    {
        range[0] = min_block_size;
        range[1] = max_block_size;
        range[2] = block_size_cur;
    }
}

FREERTOS_TLSF_TEXT_SECTION void tlsf_show_hist_min_max_free_block(tlsf_t tlsf, int *range)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    tlsf_show_hist_min_max_free_block_size(control, range);
}


FREERTOS_TLSF_TEXT_SECTION void tlsf_show_cur_max_block(tlsf_t tlsf, int *range)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    tlsf_show_cur_max_free_size(control, range);
}

FREERTOS_TLSF_TEXT_SECTION static void tlsf_record_mem_free(mem_record_t* mem_rec, block_header_t* block)
{
    size_t blk_size = block_size(block);

    tlsf_assert((blk_size - block_tail_overhead <= mem_rec->size_total_alloc) && "block size overflows!");

    mem_rec->cnt_total_free++;
    mem_rec->size_total_alloc -= blk_size - block_tail_overhead;
    mem_rec->size_real_wanted -= block_size_wanted(block);
}

FREERTOS_TLSF_TEXT_SECTION static void tlsf_record_mem_resize(mem_record_t* mem_rec, block_header_t* block,
                                   size_t old_wanted_size, size_t old_blk_size)
{
    size_t new_wanted_size = block_size_wanted(block);
    size_t new_blk_size = block_size(block);

    /* refresh the size */
    tlsf_assert(mem_rec->size_real_wanted + new_wanted_size >= old_wanted_size);
    mem_rec->size_real_wanted = mem_rec->size_real_wanted + new_wanted_size - old_wanted_size;

    tlsf_assert(mem_rec->size_total_alloc + new_blk_size >= old_blk_size);
    mem_rec->size_total_alloc = mem_rec->size_total_alloc + new_blk_size - old_blk_size;
    if(mem_rec->size_total_alloc > mem_rec->size_peak_alloc)
    {
        mem_rec->size_peak_alloc = mem_rec->size_total_alloc;
    }
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_mem_size_free(tlsf_t tlsf)
{
    control_t*      control = tlsf_cast(control_t*, tlsf);
    mem_record_t*   mem_rec = &control->mem_rec;

    return (pool_group[gucPoolGroupSel].size_total - mem_rec->size_total_alloc);
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_mem_size_ever_min(tlsf_t tlsf)
{
    control_t*      control = tlsf_cast(control_t*, tlsf);
    mem_record_t*   mem_rec = &control->mem_rec;

    return (pool_group[gucPoolGroupSel].size_total - mem_rec->size_peak_alloc);
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_mem_max_block_size(tlsf_t tlsf)
{
    control_t    *control = tlsf_cast(control_t*, tlsf);
    block_header_t *block = NULL;
    unsigned int fl = 0, sl = 0;

    fl = tlsf_fls(control->fl_bitmap);
    sl = tlsf_fls(control->sl_bitmap[fl]);
    block = control->blocks[fl][sl];
    if(block_is_free(block))
    {
        return  block_size(block);
    }

    return 0;
}

FREERTOS_TLSF_TEXT_SECTION int tlsf_clear_mem_record(tlsf_t tlsf)
{
    control_t* control = tlsf_cast(control_t*, tlsf);

    memset(&control->mem_rec, 0, sizeof(control->mem_rec));
#ifdef MEM_BLK_STAT
    memset(&control->blk_rec, 0, sizeof(control->blk_rec));
#endif

    return 0;
}

FREERTOS_TLSF_TEXT_SECTION int tlsf_show_mem_record(tlsf_t tlsf)
{
    unsigned int   cnt_used = 0;
    double       util_ratio = 0.0;
    double       frag_ratio = 0.0;
    size_t        frag_size = 0;
    size_t     blk_overhead = 0;
    size_t     tot_overhead = 0;
    control_t*      control = tlsf_cast(control_t*, tlsf);
    mem_record_t*   mem_rec = &control->mem_rec;

    if(mem_rec->cnt_total_alloc > mem_rec->cnt_total_free)
    {
        cnt_used = mem_rec->cnt_total_alloc - mem_rec->cnt_total_free;
    }

    frag_size = mem_rec->size_total_alloc - mem_rec->size_real_wanted;
    blk_overhead = (block_header_overhead + block_tail_overhead) * cnt_used - block_header_overhead;
    tot_overhead = tlsf_size() + tlsf_pool_overhead() + blk_overhead + frag_size;

    util_ratio = (100.0 * mem_rec->size_real_wanted) / (mem_rec->size_total_alloc + blk_overhead + frag_size);
    frag_ratio = (100.0 * frag_size) / mem_rec->size_total_alloc;

    printf("[memory overview] %d bytes, %d pool(s)\n", pool_group[gucPoolGroupSel].size_total, pool_group[gucPoolGroupSel].cnt);
    for(int i = 0; i < POOL_CNT_MAX; i ++)
    {
        if(pool_group[gucPoolGroupSel].node[i].start)
        {
            printf("        *p[%d]*| %d bytes, start @0x%p\n", i, pool_group[gucPoolGroupSel].node[i].size, pool_group[gucPoolGroupSel].node[i].start);
        }
    }

    printf("        ------+-------------------------\n\
        *scat*| fl-map     : 0x%x \n", control->fl_bitmap);
    for (int fli = 0; fli < FL_INDEX_COUNT; ++fli)
    {
        if(control->sl_bitmap[fli])
        {
            printf("              | sl-map[%02d] : 0x%x \n", fli, control->sl_bitmap[fli]);
        }
    }

    printf("        ------+-------------------------\n\
        *size*| wanted     : %d bytes \n\
              | alloc      : %d bytes \n\
              | peak-alloc : %d bytes \n\
              | blk-ovhead : %d bytes \n\
              | tot-ovhead : %d bytes \n\
              | util-ratio : %.2f%% \n\
              | frag-ratio : %.2f%% \n\
        ------+-------------------------\n\
        *stat*| alloc cnt  : %d \n\
              | free  cnt  : %d \n",\
                mem_rec->size_real_wanted,\
                mem_rec->size_total_alloc,\
                mem_rec->size_peak_alloc,\
                blk_overhead,\
                tot_overhead,\
                util_ratio,\
                frag_ratio,\
                mem_rec->cnt_total_alloc,\
                mem_rec->cnt_total_free);

#ifdef MEM_BLK_STAT
    tlsf_show_block_record(&control->blk_rec[0][0]);
#endif

    return 0;
}
#endif

#ifdef MM_DEBUG_EN
FREERTOS_TLSF_TEXT_SECTION static void tlsf_set_block_boundary(block_header_t* block)
{
    void* ptr = block_to_ptr(block);
    unsigned char* tail = tlsf_cast(unsigned char*, ptr) + (block_size(block) - block_tail_overhead);

    *tlsf_cast(int*, tail) = BLOCK_TAIL_BOUNDARY;
#if defined (TLSF_ALIGN_64BIT)
    *tlsf_cast(int*, tail + 4) = BLOCK_TAIL_BOUNDARY;
#endif

#ifdef MM_HEAD_BOUNDARY
    *tlsf_cast(int*, block->head_bound) = BLOCK_HEAD_BOUNDARY;
#if defined (TLSF_ALIGN_64BIT) && !defined (MEM_BLK_SIZE_32BIT)
    *tlsf_cast(int*, &block->head_bound[4]) = BLOCK_HEAD_BOUNDARY;
#endif
#endif
}

FREERTOS_TLSF_TEXT_SECTION static int tlsf_check_block_boundary(block_header_t* block)
{
    void* ptr = block_to_ptr(block);
    unsigned char* tail = tlsf_cast(unsigned char*, ptr) + (block_size(block) - block_tail_overhead);

    tlsf_assert(*tlsf_cast(int*, tail) == BLOCK_TAIL_BOUNDARY && "tail boundary[1] was overlaid!");
#if defined (TLSF_ALIGN_64BIT)
    tlsf_assert(*tlsf_cast(int*, tail + 4) == BLOCK_TAIL_BOUNDARY && "tail boundary[2] was overlaid!");
#endif

#ifdef MM_HEAD_BOUNDARY
    tlsf_assert(*tlsf_cast(int*, block->head_bound) == BLOCK_HEAD_BOUNDARY && "head boundary[1] was overlaid!");
#if defined (TLSF_ALIGN_64BIT) && !defined (MEM_BLK_SIZE_32BIT)
    tlsf_assert(*tlsf_cast(int*, &block->head_bound[4]) == BLOCK_HEAD_BOUNDARY && "head boundary[2] was overlaid!");
#endif
#endif

    return 0;
}
#endif


/*
** Size of the TLSF structures in a given memory block passed to
** tlsf_create, equal to the size of a control_t
*/
FREERTOS_TLSF_TEXT_SECTION size_t tlsf_size(void)
{
    return sizeof(control_t);
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_align_size(void)
{
    return ALIGN_SIZE;
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_block_size_min(void)
{
    return block_size_min;
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_block_size_max(void)
{
    return block_size_max;
}

/*
** Overhead of the TLSF structures in a given memory block passed to
** tlsf_add_pool, equal to the overhead of a free block and the
** sentinel block.
*/
FREERTOS_TLSF_TEXT_SECTION size_t tlsf_pool_overhead(void)
{
    return 2 * block_header_overhead;
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_alloc_overhead(void)
{
    return block_header_overhead;
}

FREERTOS_TLSF_TEXT_SECTION pool_t tlsf_add_pool(tlsf_t tlsf, void* mem, size_t bytes)
{
    block_header_t* block;
    block_header_t* next;

    const size_t pool_overhead = tlsf_pool_overhead();
    const size_t pool_bytes = align_down(bytes - pool_overhead, ALIGN_SIZE);

    if (((ptrdiff_t)mem % ALIGN_SIZE) != 0)
    {
        //printf("tlsf_add_pool: Memory must be aligned by %u bytes.\n", (unsigned int)ALIGN_SIZE);
        return 0;
    }

    if (pool_bytes < block_size_min || pool_bytes > block_size_max)
    {
#if 0
    #if defined (TLSF_64BIT)
        printf("tlsf_add_pool: Memory size must be between 0x%x and 0x%x00 bytes.\n",
            (unsigned int)(pool_overhead + block_size_min),
            (unsigned int)((pool_overhead + block_size_max) / 256));
    #else
        printf("tlsf_add_pool: Memory size must be between %u and %u bytes.\n",
            (unsigned int)(pool_overhead + block_size_min),
            (unsigned int)(pool_overhead + block_size_max));
    #endif
#endif
        return 0;
    }


    /*
    ** Create the main free block. Offset the start of the block slightly
    ** so that the prev_phys_block field falls outside of the pool -
    ** it will never be used.
    */
    block = offset_to_block(mem, -(tlsfptr_t)sizeof(block_header_t*));
    block_set_size(block, pool_bytes);
    block_set_free(block);
    block_set_prev_used(block);
    block_insert(tlsf_cast(control_t*, tlsf), block);

    /* Split the block to create a zero-size sentinel block. */
    next = block_link_next(block);
    memset(next, 0, sizeof(block_header_t) - 2 * sizeof(block_header_t*));
    block_set_size(next, 0);
    block_set_used(next);
    block_set_prev_free(next);

    /* Record info of this pool */
    for(int i = 0; i < POOL_CNT_MAX; i ++)
    {
        if(pool_group[gucPoolGroupSel].node[i].start == NULL)
        {
            pool_group[gucPoolGroupSel].node[i].size  = pool_bytes;
            pool_group[gucPoolGroupSel].node[i].start = (pool_t)mem;

            pool_group[gucPoolGroupSel].size_total += pool_bytes;
            if(pool_group[gucPoolGroupSel].cnt < POOL_CNT_MAX) pool_group[gucPoolGroupSel].cnt ++;

            break;
        }
    }

    return mem;
}

FREERTOS_TLSF_TEXT_SECTION void tlsf_remove_pool(tlsf_t tlsf, pool_t pool)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    block_header_t* block = offset_to_block(pool, -(int)sizeof(block_header_t*));
    const size_t pool_bytes = block_size(block);

    int fl = 0, sl = 0;

    tlsf_assert(block_is_free(block) && "block should be free");
    tlsf_assert((!block_is_free(block_next(block)) && block_size(block_next(block)) == 0) && "next block should not be free & its size should be zero");

    /* Record info of this pool */
    for(int i = 0; i < POOL_CNT_MAX; i ++)
    {
        if(pool_group[gucPoolGroupSel].node[i].start == pool)
        {
            tlsf_assert(pool_group[gucPoolGroupSel].size_total >= pool_bytes && "pool size overflow!");

            pool_group[gucPoolGroupSel].node[i].size  = 0;
            pool_group[gucPoolGroupSel].node[i].start = NULL;

            pool_group[gucPoolGroupSel].size_total -= pool_bytes;
            if(pool_group[gucPoolGroupSel].cnt > 0) pool_group[gucPoolGroupSel].cnt --;

            break;
        }
    }

    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

/*
** TLSF main interface.
*/

#if _DEBUG
int test_ffs_fls()
{
    /* Verify ffs/fls work properly. */
    int rv = 0;
    rv += (tlsf_ffs(0) == -1) ? 0 : 0x1;
    rv += (tlsf_fls(0) == -1) ? 0 : 0x2;
    rv += (tlsf_ffs(1) == 0) ? 0 : 0x4;
    rv += (tlsf_fls(1) == 0) ? 0 : 0x8;
    rv += (tlsf_ffs(0x80000000) == 31) ? 0 : 0x10;
    rv += (tlsf_ffs(0x80008000) == 15) ? 0 : 0x20;
    rv += (tlsf_fls(0x80000008) == 31) ? 0 : 0x40;
    rv += (tlsf_fls(0x7FFFFFFF) == 30) ? 0 : 0x80;

#if defined (TLSF_64BIT)
    rv += (tlsf_fls_sizet(0x80000000) == 31) ? 0 : 0x100;
    rv += (tlsf_fls_sizet(0x100000000) == 32) ? 0 : 0x200;
    rv += (tlsf_fls_sizet(0xffffffffffffffff) == 63) ? 0 : 0x400;
#endif

    if (rv)
    {
        printf("test_ffs_fls: %x ffs/fls tests failed.\n", rv);
    }
    return rv;
}
#endif

FREERTOS_TLSF_TEXT_SECTION tlsf_t tlsf_create(void* mem)
{
     tlsf_assert(!((tlsfptr_t)mem % ALIGN_SIZE) && "mem must be aligned to 'ALIGN_SIZE'!");

    control_construct(tlsf_cast(control_t*, mem));
    memset(&pool_group[gucPoolGroupSel], 0, sizeof(pool_group_t));

    return tlsf_cast(tlsf_t, mem);
}

FREERTOS_TLSF_TEXT_SECTION tlsf_t tlsf_create_with_pool(void* mem, size_t bytes)
{
    tlsf_t tlsf = NULL;

    tlsf_assert(bytes >= tlsf_size() + tlsf_pool_overhead() && "mem size is too small!");

    tlsf_mem_protect();
    {
        tlsf = tlsf_create(mem);
        tlsf_assert(tlsf && "should not be NULL");

        tlsf_add_pool(tlsf, (char*)mem + tlsf_size(), bytes - tlsf_size());
    }
    tlsf_mem_deprotect();

    return tlsf;
}

FREERTOS_TLSF_TEXT_SECTION void tlsf_destroy(tlsf_t tlsf)
{
    /* Nothing to do. */
    (void)tlsf;
}

FREERTOS_TLSF_TEXT_SECTION pool_t tlsf_get_pool(tlsf_t tlsf)
{
    return tlsf_cast(pool_t, (char*)tlsf + tlsf_size());
}

FREERTOS_TLSF_TEXT_SECTION size_t tlsf_block_size(void* ptr)
{
    size_t size = 0;

    if (ptr)
    {
        const block_header_t* block = block_from_ptr(ptr);
        size = block_size(block);
    }
    return size;
}

FREERTOS_TLSF_TEXT_SECTION void* tlsf_malloc(tlsf_t tlsf, size_t size, size_t funcPtr)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
    block_header_t* block = NULL;
    void* alloc = NULL;

    tlsf_mem_protect();
    {
        block = block_locate_free(control, adjust);
        if(block)
        {
            alloc = block_prepare_used(control, block, adjust);
            block_set_size_wanted(block, size);

        #ifdef MM_DEBUG_EN
            osThreadId_t taskHandle;
            size_t taskNum;
            /* set the head & tail boundary & thread owner! */
            tlsf_set_block_boundary(block);
            //block->alloc_owner = osThreadGetId();
            taskHandle = (osThreadId_t)osThreadGetId();   //get task handle to keep thread safe
            taskNum = uxTaskGetTaskNumber(taskHandle);
            taskNum = taskNum > 0xFF ? 0xFF : taskNum;
            block->alloc_owner = (funcPtr & 0xFFFFFF) | (taskNum<<24);
        #endif

        #ifdef HEAP_MEM_DEBUG
            tlsf_record_mem_alloc(&control->mem_rec, block, size);
            tlsf_record_hist_min_free(control);
            #ifdef MEM_BLK_STAT
            tlsf_record_block_alloc(&control->blk_rec[0][0], block_size(block));
            #endif
        #endif
        }
    }
    tlsf_mem_deprotect();

    return alloc;
}

FREERTOS_TLSF_TEXT_SECTION void* tlsf_memalign(tlsf_t tlsf, size_t align, size_t size, size_t funcPtr)
{
    void* alloc = NULL;
    control_t* control = tlsf_cast(control_t*, tlsf);
    const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

    /*
    ** We must allocate an additional minimum block size bytes so that if
    ** our free block will leave an alignment gap which is smaller, we can
    ** trim a leading free block and release it back to the pool. We must
    ** do this because the previous physical block is in use, therefore
    ** the prev_phys_block field is not valid, and we can't simply adjust
    ** the size of that block.
    */
    const size_t gap_minimum = block_size_min;
    const size_t size_with_gap = adjust_request_size(adjust + align + gap_minimum, align);

    /*
    ** If alignment is less than or equals base alignment, we're done.
    ** If we requested 0 bytes, return null, as tlsf_malloc(0) does.
    */
    const size_t aligned_size = (adjust && align > ALIGN_SIZE) ? size_with_gap : adjust;

    tlsf_mem_protect();
    {
        block_header_t* block = block_locate_free(control, aligned_size);

        /* This can't be a static assert. */
        tlsf_assert(sizeof(block_header_t) == block_size_min + sizeof(block_header_t*) - block_tail_overhead);

        if (block)
        {
            void* ptr = block_to_ptr(block);
            void* aligned = align_ptr(ptr, align);
            size_t gap = tlsf_cast(size_t, tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));

            /* If gap size is too small, offset to next aligned boundary. */
            if (gap && gap < gap_minimum)
            {
                const size_t gap_remain = gap_minimum - gap;
                const size_t offset = tlsf_max(gap_remain, align);
                const void* next_aligned = tlsf_cast(void*, tlsf_cast(tlsfptr_t, aligned) + offset);

                aligned = align_ptr(next_aligned, align);
                gap = tlsf_cast(size_t, tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));
            }

            if (gap)
            {
                tlsf_assert(gap >= gap_minimum && "gap size too small");
                block = block_trim_free_leading(control, block, gap);
            }

            alloc = block_prepare_used(control, block, adjust);
            block_set_size_wanted(block, size);

        #ifdef MM_DEBUG_EN
            osThreadId_t taskHandle;
            size_t taskNum;
            /* set head & tail boundary & thread owner! */
            tlsf_set_block_boundary(block);
//          block->alloc_owner = osThreadGetId();
            taskHandle = (osThreadId_t)osThreadGetId();   //get task handle to keep thread safe
            taskNum = uxTaskGetTaskNumber(taskHandle);
            taskNum = taskNum > 0xFF ? 0xFF : taskNum;
            block->alloc_owner = (funcPtr & 0xFFFFFF) | (taskNum<<24);
        #endif

        #ifdef HEAP_MEM_DEBUG
            tlsf_record_mem_alloc(&control->mem_rec, block, size);
            tlsf_record_hist_min_free(control);
            #ifdef MEM_BLK_STAT
            tlsf_record_block_alloc(&control->blk_rec[0][0], block_size(block));
            #endif
        #endif
        }
    }
    tlsf_mem_deprotect();

    return alloc;
}

FREERTOS_TLSF_TEXT_SECTION void tlsf_free(tlsf_t tlsf, void* ptr)
{
    tlsf_mem_protect();

    /* Don't attempt to free a NULL pointer. */
    if (ptr)
    {
        control_t* control = tlsf_cast(control_t*, tlsf);
        block_header_t* block = block_from_ptr(ptr);

        tlsf_assert(!block_is_free(block) && "block already marked as free");

    #ifdef MM_DEBUG_EN
        /* check head & tail boundary */
        tlsf_check_block_boundary(block);
    #endif
    #ifdef HEAP_MEM_DEBUG
        tlsf_record_mem_free(&control->mem_rec, block);
        #ifdef MEM_BLK_STAT
        tlsf_record_block_free(&control->blk_rec[0][0], block_size(block));
        #endif
    #endif
        /* clear wanted size */
        block_set_size_wanted(block, 0);

        block_mark_as_free(block);
        block = block_merge_prev(control, block);
        block = block_merge_next(control, block);
        block_insert(control, block);
    }

    tlsf_mem_deprotect();
}


FREERTOS_TLSF_TEXT_SECTION int32_t tlsf_check_free_size(tlsf_t tlsf, void* ptr)
{
    int32_t size = -1;
    tlsf_mem_protect();

    /* Don't attempt to free a NULL pointer. */
    if (ptr)
    {
        block_header_t* block = block_from_ptr(ptr);
        tlsf_assert(!block_is_free(block) && "block already marked as free");
        size = block_size(block);
    }
    tlsf_mem_deprotect();

    return size;
}
/*
** The TLSF block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
FREERTOS_TLSF_TEXT_SECTION void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size, size_t funcPtr)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    void* p = 0;

    /* Zero-size requests are treated as free. */
    if (ptr && size == 0)
    {
        tlsf_free(tlsf, ptr);
    }
    /* Requests with NULL pointers are treated as malloc. */
    else if (!ptr)
    {
        p = tlsf_malloc(tlsf, size, funcPtr);
    }
    else
    {
        tlsf_mem_protect();

        block_header_t* block = block_from_ptr(ptr);
        block_header_t* next = block_next(block);

    #ifdef MM_DEBUG_EN
        /* check head & tail boundary */
        tlsf_check_block_boundary(block);
    #endif

        const size_t oldwant = block_size_wanted(block);
        const size_t cursize = block_size(block);
        const size_t combined = cursize + block_size(next) + block_header_overhead;
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

        tlsf_assert(!block_is_free(block) && "block already marked as free");

        /*
        ** If the next block is used, or when combined with the current
        ** block, does not offer enough space, we must reallocate and copy.
        */
        if (adjust > cursize && (!block_is_free(next) || adjust > combined))
        {
            p = tlsf_malloc(tlsf, size, funcPtr);
            if (p)
            {
                const size_t minsize = tlsf_min(cursize, size);
                memcpy(p, ptr, minsize);
                tlsf_free(tlsf, ptr);
            }
        }
        else
        {
            p = ptr;

            /* Do we need to expand to the next block? */
            if (adjust > cursize)
            {
                block_merge_next(control, block);
                block_mark_as_used(block);
            }

            /* Trim the resulting block and return the original pointer. */
            block_trim_used(control, block, adjust, adjust > cursize);
            block_set_size_wanted(block, size);

        #ifdef MM_DEBUG_EN
            /* set the head & tail boundary & thread owner! */
            tlsf_set_block_boundary(block);
        #endif

        #ifdef HEAP_MEM_DEBUG
            tlsf_record_mem_resize(&control->mem_rec, block, oldwant, cursize);
            #ifdef MEM_BLK_STAT
            tlsf_record_block_free(&control->blk_rec[0][0], cursize);
            tlsf_record_block_alloc(&control->blk_rec[0][0], block_size(block));
            #endif
        #endif
        }
        tlsf_mem_deprotect();
    }

    return p;
}

#ifdef __USER_CODE__
FREERTOS_TLSF_TEXT_SECTION void tlsf_mem_get_record(tlsf_t tlsf, uint32_t *alloc, uint32_t *peak)
{
    control_t*      control = tlsf_cast(control_t*, tlsf);
    mem_record_t*   mem_rec = &control->mem_rec;
    *alloc = (uint32_t)mem_rec->size_total_alloc;
    *peak = (uint32_t)mem_rec->size_peak_alloc;
}
#endif

#endif


