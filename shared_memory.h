//
// Created by root on 10/24/19.
//

#ifndef LAB3_SHARED_MEMORY_H
#define LAB3_SHARED_MEMORY_H

#include <stddef.h>
#include <stdbool.h>

void* create_shared_memory(size_t size);

bool clear_shared_memory(void* ptr);

#endif //LAB3_SHARED_MEMORY_H
