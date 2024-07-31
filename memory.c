#include "oslabs.h"
#include <limits.h>

// Best-Fit Allocation
struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int best_index = -1;
    int best_size = INT_MAX;

    // Find the smallest free block that can accommodate the request
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && 
            memory_map[i].segment_size >= request_size && 
            memory_map[i].segment_size < best_size) {

            best_size = memory_map[i].segment_size;
            best_index = i;
        }
    }

    // No suitable block found
    if (best_index == -1) {
        return NULLBLOCK; 
    }

    struct MEMORY_BLOCK allocated_block = memory_map[best_index];

    // Exact fit - no need to split
    if (allocated_block.segment_size == request_size) {
        memory_map[best_index].process_id = process_id;
    } else {
        // Split the block
        memory_map[best_index].start_address += request_size;
        memory_map[best_index].segment_size -= request_size;

        // Update the allocated block
        allocated_block.end_address = allocated_block.start_address + request_size - 1;
        allocated_block.segment_size = request_size;
        allocated_block.process_id = process_id;

        // Shift memory map to insert the allocated block at the beginning
        for (int i = *map_cnt; i > best_index; i--) {
            memory_map[i] = memory_map[i - 1];
        }
        memory_map[best_index] = allocated_block;
        (*map_cnt)++; 
    }

    return allocated_block;
}

// First-Fit Allocation
struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            struct MEMORY_BLOCK allocated_block = memory_map[i];
            if (allocated_block.segment_size == request_size) {
                memory_map[i].process_id = process_id;
            } else {
                // Split the block
                memory_map[i].start_address += request_size;
                memory_map[i].segment_size -= request_size;
                allocated_block.end_address = allocated_block.start_address + request_size - 1;
                allocated_block.segment_size = request_size;
                allocated_block.process_id = process_id;

                // Shift memory map to insert the allocated block at the beginning
                for (int j = *map_cnt; j > i; j--) {
                    memory_map[j] = memory_map[j - 1];
                }
                memory_map[i] = allocated_block;
                (*map_cnt)++;
            }
            return allocated_block;
        }
    }
    return NULLBLOCK;
}

// Worst-Fit Allocation
struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int worst_index = -1;
    int worst_size = -1;

    // Find the largest free block that can accommodate the request
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && 
            memory_map[i].segment_size >= request_size && 
            memory_map[i].segment_size > worst_size) {

            worst_size = memory_map[i].segment_size;
            worst_index = i;
        }
    }

    // No suitable block found
    if (worst_index == -1) {
        return NULLBLOCK;
    }

    struct MEMORY_BLOCK allocated_block = memory_map[worst_index];

    // Exact fit - no need to split
    if (allocated_block.segment_size == request_size) {
        memory_map[worst_index].process_id = process_id;
    } else {
        // Split the block
        memory_map[worst_index].start_address += request_size;
        memory_map[worst_index].segment_size -= request_size;

        // Update the allocated block
        allocated_block.end_address = allocated_block.start_address + request_size - 1;
        allocated_block.segment_size = request_size;
        allocated_block.process_id = process_id;

        // Shift memory map to insert the allocated block at the beginning
        for (int i = *map_cnt; i > worst_index; i--) {
            memory_map[i] = memory_map[i - 1];
        }
        memory_map[worst_index] = allocated_block;
        (*map_cnt)++;
    }

    return allocated_block;
}
// Next-Fit Allocation
struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    int start_index = 0;
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address >= last_address) {
            start_index = i;
            break;
        }
    }
    for (int i = start_index; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            struct MEMORY_BLOCK allocated_block = memory_map[i];
            if (allocated_block.segment_size == request_size) {
                memory_map[i].process_id = process_id;
            } else {
                // Split the block
                memory_map[i].start_address += request_size;
                memory_map[i].segment_size -= request_size;
                allocated_block.end_address = allocated_block.start_address + request_size - 1;
                allocated_block.segment_size = request_size;
                allocated_block.process_id = process_id;

                // Shift memory map to insert the allocated block at the beginning
                for (int j = *map_cnt; j > i; j--) {
                    memory_map[j] = memory_map[j - 1];
                }
                memory_map[i] = allocated_block;
                (*map_cnt)++;
            }
            return allocated_block;
        }
    }
    return NULLBLOCK;
}

// Release Memory
void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt) {
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address == freed_block.start_address && memory_map[i].end_address == freed_block.end_address) {
            memory_map[i].process_id = 0; // Mark as free

            // Merge with previous block if free
            if (i > 0 && memory_map[i - 1].process_id == 0) {
                memory_map[i - 1].end_address = memory_map[i].end_address;
                memory_map[i - 1].segment_size += memory_map[i].segment_size;
                
                // Remove the current block
                for (int j = i; j < *map_cnt - 1; j++) {
                    memory_map[j] = memory_map[j + 1];
                }
                (*map_cnt)--;
                i--; // Adjust index after removal
            }

            // Merge with next block if free
              if (i < *map_cnt - 1 && memory_map[i + 1].process_id == 0) {
               memory_map[i].end_address = memory_map[i + 1].end_address;
               memory_map[i].segment_size += memory_map[i + 1].segment_size;
               for (int j = i + 1; j < *map_cnt - 1; j++) {
                   memory_map[j] = memory_map[j + 1];
               }
               (*map_cnt)--;
           }

           break;
       }
   }
}
