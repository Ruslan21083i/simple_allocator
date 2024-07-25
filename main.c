#include <stdio.h>
#include "simple_allocator.h"

#define BLOCK_SIZE  11

int main(void) {
    void *ptr1;
    void *ptr2;
    int block_size = BLOCK_SIZE;
    int blocks_allocated = 0;

    simple_allocator_init();
    
    if (simple_allocator_malloc(-1)) {
        fprintf(stderr, "error0\n");
    }
    simple_allocator_free(NULL);
    
    ptr1 = simple_allocator_malloc(block_size);
    blocks_allocated++;
    ptr2 = simple_allocator_malloc(block_size);
    blocks_allocated++;

    while (simple_allocator_malloc(block_size)) {
        blocks_allocated++;
    }
    fprintf(stderr, "blocks_allocated=%d\n", blocks_allocated);

    simple_allocator_free(ptr1);

    if (simple_allocator_malloc(block_size) != ptr1) {
        fprintf(stderr, "error1\n");
    }
    if (simple_allocator_malloc(block_size) != NULL) {
        fprintf(stderr, "error2\n");
    }
    
    block_size = 16;

    simple_allocator_free(ptr2);
    simple_allocator_free(ptr1);
    if (simple_allocator_malloc(block_size * 2) != ptr1) {
        // два блока подряд должны были склеится
        fprintf(stderr, "error3\n");
    }
    simple_allocator_free(ptr1);
    if (simple_allocator_malloc(block_size) != ptr1) {
        fprintf(stderr, "error4\n");
    }
    if (simple_allocator_malloc(block_size) != ptr2) {
        fprintf(stderr, "error5\n");
    }

    simple_allocator_init();
    
    {
        int blocks_allocated2 = 0;
        int i;
        
        for (i = 0; i < blocks_allocated / 2; i++) {
            if (simple_allocator_malloc(block_size) == NULL) {
                fprintf(stderr, "error6\n");
            }
            blocks_allocated2++;
        }
        ptr1 = simple_allocator_malloc(block_size);
        ptr2 = simple_allocator_malloc(block_size);
        if (ptr1 == NULL || ptr2 == NULL) {
                fprintf(stderr, "error7\n");
        }

        simple_allocator_free(ptr2);
        simple_allocator_free(ptr1);

        while (simple_allocator_malloc(block_size)) {
            blocks_allocated2++;
        }
        
        if (blocks_allocated != blocks_allocated2) {
            fprintf(stderr, "error8\n");
        }
    }

    return 0;
}
