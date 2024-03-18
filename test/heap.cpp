#include "tests.hpp"
#include <mem/heap.hpp>
#include <logger.hpp>

#define HEAP_TEST_BLOCK_COUNT 32

#define HEAP_TEST_BASE  0x200000
#define HEAP_TEST_LIMIT 0x400000

typedef Heap*(*RetrieveHeapObject)(void*, void*);

static LCHeap test_lcheap(nullptr, nullptr);

static Heap* retrieve_lcheap(void* base, void* limit) {
    test_lcheap.set_range(base, limit);
    return &test_lcheap;
}

static const char* htest_run(void* param);

UnitTest lcheap_test = { "LCHeap Test", htest_run, (void*)retrieve_lcheap };

void test_heap_init() {
    UnitTest::add_test(lcheap_test);
}

struct MemBlock {
    bool  present;
    u8    align;
    void* ptr;
    usize size;
};

static MemBlock htest_blocks[HEAP_TEST_BLOCK_COUNT];

static Heap* htest_heap = nullptr;

const char* htest_error = nullptr;

static inline usize align_bits_mask(u8 align) {
    return ~(((usize)0 - 1) << align);
}

static void htest_clear() {
    for (usize i = 0; i < HEAP_TEST_BLOCK_COUNT; i++)
        htest_blocks[i].present = false;

    htest_heap = nullptr;
}

static void htest_set_error(const char* err) {
    if (!htest_error)
        htest_error = err;
}

static void htest_validate() {
    usize total_size = 0;
    u32 block_count = 0;

    for (usize i = 0; i < HEAP_TEST_BLOCK_COUNT; i++) {
        MemBlock* block1 = &htest_blocks[i];
        if (!block1->present) continue;

        block_count++;
        total_size += block1->size;

        u8* ptr8 = (u8*)(block1->ptr);
        for (usize j = 0; j < block1->size; j++)
            ptr8[j] = 0x3A;

        if (block1->ptr < (void*)HEAP_TEST_BASE || ((usize)block1->ptr + (usize)block1->size) >= HEAP_TEST_LIMIT) {
            htest_set_error("Memory block out of bounds");
            return;
        }

        if ((usize)block1->ptr & align_bits_mask(block1->align)) {
            htest_set_error("Unaligned memory block");
            return;
        }

        for (usize j = (i + 1); j < HEAP_TEST_BLOCK_COUNT; j++) {
            MemBlock* block2 = &htest_blocks[j];
            if (!block2->present) continue;

            usize b1 = (usize)block1->ptr;
            usize l1 = (usize)block1->ptr + block1->size;
            usize b2 = (usize)block2->ptr;
            usize l2 = (usize)block2->ptr + block2->size;

            if (l1 > b2 && b1 < l2) {
                htest_set_error("Overlapping memory blocks");
                return;
            }
        }
    }

    if (htest_heap->get_total_size() != total_size)
        htest_set_error("Incorrect reporting of total size");

    if (htest_heap->count_blocks() != block_count)
        htest_set_error("Incorrect counting of blocks");

    if (htest_heap->count_total_size() != total_size)
        htest_set_error("Incorrect counting of total size");
}

static void* htest_malloc(usize size, u8 align = HEAP_BYTE_ALIGN_BITS) {
    if (!htest_heap)
        return nullptr;

    void* ret = htest_heap->malloc(size, align);

    for (usize i = 0; i < HEAP_TEST_BLOCK_COUNT; i++) {
        MemBlock* block = &htest_blocks[i];

        if (!block->present) {
            block->present = true;
            block->align = align;
            block->ptr   = ret;
            block->size  = size;
            break;
        }
    }

    htest_validate();
    return ret;
}

static void htest_free(void* ptr) {
    if (!htest_heap)
        return;

    htest_heap->free(ptr);

    for (usize i = 0; i < HEAP_TEST_BLOCK_COUNT; i++) {
        MemBlock* block = &htest_blocks[i];

        if (block->present && block->ptr == ptr) {
            block->present = false;
            break;
        }
    }
    
    htest_validate();
}

static void* htest_realloc(void* ptr, usize size) {
    if (!htest_heap)
        return nullptr;

    void* new_ptr = htest_heap->realloc(ptr, size);

    for (usize i = 0; i < HEAP_TEST_BLOCK_COUNT; i++) {
        MemBlock* block = &htest_blocks[i];

        if (block->present && block->ptr == ptr) {
            block->ptr  = new_ptr;
            block->size = size;
            break;
        }
    }
    
    htest_validate();
    return new_ptr;
}

static const char* htest_run(void* param) {
    htest_clear();
    RetrieveHeapObject func = (RetrieveHeapObject)param;
    
    if (func)
        htest_heap = func((void*)HEAP_TEST_BASE, (void*)HEAP_TEST_LIMIT);

    if (!htest_heap)
        return "No heap provided";

    void* m0 = htest_malloc(50);
    void* m1 = htest_malloc(30);
    void* m2 = htest_malloc(426);
    void* m3 = htest_malloc(180);
    void* m4 = htest_malloc(800);

    htest_free(m0);
    htest_free(m3);

    m0 = htest_malloc(920);
    m3 = htest_malloc(30);

    htest_free(m2);
    htest_free(m4);

    m2 = htest_malloc(10);
    m4 = htest_malloc(690);

    void* a0 = htest_malloc(200, 8);
    void* a1 = htest_malloc(858, 4);
    void* a2 = htest_malloc(104, 7);

    htest_free(a1);

    a0 = htest_realloc(a0, 800);
    
    m1 = htest_realloc(m1, 890);
    m2 = htest_realloc(m2, 400);

    htest_free(m0);
    htest_free(m1);
    htest_free(m2);
    htest_free(m3);

    htest_free(m4);
    htest_free(a0);
    htest_free(a2);

    return htest_error;
}