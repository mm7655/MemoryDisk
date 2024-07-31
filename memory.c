#include <stdio.h>
#include <stdlib.h>
#include "oslabs.h"

// Best Fit allocation
struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK best_block = {-1, -1, -1, -1};
    int best_diff = MAPMAX + 1;
    int best_fit_index = -1;

    // Find the best fit block
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            int diff = memory_map[i].segment_size - request_size;
            if (diff < best_diff) {
                best_diff = diff;
                best_block = memory_map[i];
                best_fit_index = i;
            }
        }
    }

    // Allocate and split if necessary
    if (best_block.start_address != -1) {
        if (best_diff > 0) {
            // Create a new memory block for the remaining space after allocation
            struct MEMORY_BLOCK new_block = {best_block.start_address + request_size,
                                             best_block.end_address,
                                             best_diff,
                                             0};
            // Update the allocated block with the new ending address and size
            best_block.end_address = best_block.start_address + request_size - 1;
            best_block.segment_size = request_size;

            // Shift memory map and insert the new block
            for (int j = *map_cnt; j > best_fit_index + 1; j--) {
                memory_map[j] = memory_map[j - 1];
            }
            memory_map[best_fit_index + 1] = new_block;
            (*map_cnt)++; 
        }
        best_block.process_id = process_id;
    }
    return best_block;
}



// First Fit allocation (with simplified memory release)
struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            struct MEMORY_BLOCK allocated_block = memory_map[i];

            if (allocated_block.segment_size > request_size) { // Split if needed
                // Create a new memory block for the remaining space after allocation
                struct MEMORY_BLOCK new_block = {allocated_block.start_address + request_size, 
                                                 allocated_block.end_address, 
                                                 allocated_block.segment_size - request_size, 
                                                 0};
                // Update the allocated block with the new ending address and size
                allocated_block.end_address = allocated_block.start_address + request_size - 1;
                allocated_block.segment_size = request_size;

                // Shift memory map and insert the new block
                for (int j = *map_cnt; j > i + 1; j--) {
                    memory_map[j] = memory_map[j - 1];
                }
                memory_map[i + 1] = new_block;
                (*map_cnt)++;
            }

            allocated_block.process_id = process_id;
            return allocated_block;
        }
    }
    return (struct MEMORY_BLOCK) {-1, -1, -1, -1}; // NULLBLOCK if no fit is found
}

// Worst Fit allocation (with simplified memory release)
struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK worst_block = {-1, -1, -1, -1};
    int worst_diff = -1;
    int worst_fit_index = -1; // To track the index of the worst fit block

    // Find the worst fit block
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            int diff = memory_map[i].segment_size - request_size;
            if (diff > worst_diff) {
                worst_diff = diff;
                worst_block = memory_map[i];
                worst_fit_index = i;
            }
        }
    }

    // Allocate and split if necessary
    if (worst_block.start_address != -1) {
        if (worst_diff > 0) {
            struct MEMORY_BLOCK new_block = {worst_block.start_address + request_size,
                                             worst_block.end_address,
                                             worst_diff,
                                             0};
            worst_block.end_address = worst_block.start_address + request_size - 1;
            worst_block.segment_size = request_size;
            for (int j = *map_cnt; j > worst_fit_index + 1; j--) {
                memory_map[j] = memory_map[j - 1];
            }
            memory_map[worst_fit_index + 1] = new_block;
            (*map_cnt)++; 
        }
        worst_block.process_id = process_id;
    }
    return worst_block;
}
