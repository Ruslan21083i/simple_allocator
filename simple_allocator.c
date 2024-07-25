/*
 * Copyright (c) 2024 Ruslan Kyargin

 Простой аллокатор.
 Размер блока может быть любой, после malloc пул будет делится на блоки нужного размера
 Дескрипторы блоков содержатся в односвязном списке.
 При освобождении блока, если следующий за ним блок свободен, делается склейка блоков.
 
 Поиск свободного куска всегда идет от начала пула.
 Настройка полного размера пула через переопределения BLOCK_SIZE и BLOCKS_TOTAL
*/

#include <stdlib.h>
#include "simple_allocator.h"

#define BLOCK_SIZE          16
#define BLOCKS_TOTAL    1024


#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#define IS_POW2(val) (val && !(val & (val - 1)))

// выравнивание только для варианта когда size является степенью двойки
#define ALIGN(val, size) (((val) + (size - 1)) & ~(size - 1))

#define FLAG_CHUNK_FREE             0
#define FLAG_CHUNK_OCCUPIED     1

typedef struct chunk_desc chunk_desc_t;

// можно добавить __attribute__((packed)), но тогда надо проверять выравнивания
typedef struct chunk_desc {
    chunk_desc_t *next_chunk;
    char flags;
} chunk_desc_t;

// расчитываем размер чтобы точно хватило на BLOCK_SIZE * BLOCKS_TOTAL байт
#define TOTAL_CHUNKS  (BLOCKS_TOTAL + ALIGN(BLOCK_SIZE * BLOCKS_TOTAL, sizeof(chunk_desc_t)) / sizeof(chunk_desc_t) + 1)

// возможно надо определить специальную секцию памяти для этого
static chunk_desc_t chunks[TOTAL_CHUNKS];

static volatile int g_lock;
// критическая секция для многозадачности
static void enter_cs(void) {
    while(!__sync_bool_compare_and_swap(&g_lock, 0, 1));
}

static void exit_cs(void) {
    while(!__sync_bool_compare_and_swap(&g_lock, 1, 0));
}

void simple_allocator_init(void) {
    chunks[0].flags = FLAG_CHUNK_FREE;
    chunks[0].next_chunk = &chunks[TOTAL_CHUNKS - 1];
    chunks[TOTAL_CHUNKS - 1].flags = FLAG_CHUNK_OCCUPIED;
    chunks[TOTAL_CHUNKS - 1].next_chunk = NULL;
}

// возвращает размер данных (без размера дескриптора чанка) чанка в байтах
static unsigned int sizeof_chunk(chunk_desc_t *chunk) {
    int ret;
    ret = ((chunk->next_chunk - chunk) - 1 ) * sizeof(chunks[0]);
    return ret;
}

// req_size - выровненный размер
static chunk_desc_t* bytes_to_next_chunk(chunk_desc_t *chunk, unsigned int req_size) {
    chunk_desc_t *next_chunk;;

    next_chunk = chunk + 1 + (req_size / sizeof(chunks[0]));
    if (chunk->next_chunk != next_chunk) {
        next_chunk->next_chunk = chunk->next_chunk;
        next_chunk->flags = FLAG_CHUNK_FREE;
    }
    return next_chunk;
}

void* simple_allocator_malloc(unsigned int size) {
    chunk_desc_t *chunk = &chunks[0];

    // проверка что размер дескриптора является степенью двойки - иначе будут проблемы с выравниванием
    BUILD_BUG_ON(!IS_POW2(sizeof(chunks[0])));

    size = ALIGN(size, sizeof(chunks[0]));
    if (!size) {
        return NULL;
    }

    enter_cs();

    while (chunk) {
        if (chunk->flags == FLAG_CHUNK_FREE && sizeof_chunk(chunk) >= size) {
            chunk->flags = FLAG_CHUNK_OCCUPIED;
            chunk->next_chunk = bytes_to_next_chunk(chunk, size);
            chunk = chunk + 1;
            break;
        }
        chunk = chunk->next_chunk;
    }

    exit_cs();
    
    return chunk;
}

void simple_allocator_free(void *ptr) {
    if (ptr) {
        enter_cs();

        chunk_desc_t *chunk = (chunk_desc_t*)ptr - 1;
        chunk->flags = FLAG_CHUNK_FREE;
        if (chunk->next_chunk && chunk->next_chunk->flags == FLAG_CHUNK_FREE) {
            // объединяем сбодные чанки
            chunk->next_chunk = chunk->next_chunk->next_chunk;
        }
      exit_cs();
  }
}
