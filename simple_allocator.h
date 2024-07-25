/*
 * Copyright (c) 2024 Ruslan Kyargin

 */
#ifndef __SIMPLE_ALLOCATOR__H__
#define __SIMPLE_ALLOCATOR__H__

void simple_allocator_init(void);

void* simple_allocator_malloc(unsigned int size);
void simple_allocator_free(void *ptr);


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif //__SIMPLE_ALLOCATOR__H__
