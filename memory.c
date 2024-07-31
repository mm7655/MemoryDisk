#include <stdio.h>
#include <stdlib.h>
#include "oslabs.h"

struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK best_block = {-1, -1, -1, -1}; // NULLBLOCK initialization
    int best_diff = MAPMAX + 1; // Initialize with a large value
    int best_fit_index = -1;    // Initialize with an invalid index

    // Find the best fit block
    for (int i = 0; i < *map_cnt; i++) {
        // Check if the block is free and large enough
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            // Calculate the difference in size between the block and the request
            int diff = memory_map[i].segment_size - request_size;

            // Update best_block and best_fit_index if this is a better fit
            if (diff < best_diff) {
                best_diff = diff;
                best_block = memory_map[i]; // Found a potential fit, copy the block
                best_fit_index = i;        // Keep track of the index for splitting
            }
        }
    }

    // Allocate and split if necessary
    if (best_fit_index != -1) { // If we found a suitable block
        if (best_diff > 0) { // If the block is larger than the request, split it
            // Create a new memory block for the remaining space after allocation
            struct MEMORY_BLOCK new_block = {
                best_block.start_address + request_size, // Start after the allocated block
                best_block.end_address,                  // End address remains the same
                best_diff,                               // Size is the remaining space
                0                                       // New block is free
            };

            // Update the allocated block's end address and size
            best_block.end_address = best_block.start_address + request_size - 1;
            best_block.segment_size = request_size;

            // Shift memory map and insert the new block
            for (int j = *map_cnt; j > best_fit_index + 1; j--) {
                memory_map[j] = memory_map[j - 1];
            }
            memory_map[best_fit_index + 1] = new_block; // Insert the new block right after the allocated one
            (*map_cnt)++;                               // Increment the number of blocks
        }

        // Mark the allocated block with the process_id
        best_block.process_id = process_id;
    }
    return best_block;
}

// First Fit allocation with memory release
struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            struct MEMORY_BLOCK allocated_block = memory_map[i];

            if (allocated_block.segment_size > request_size) { // Split if needed
                struct MEMORY_BLOCK new_block = {allocated_block.start_address + request_size,
                                                 allocated_block.end_address,
                                                 allocated_block.segment_size - request_size,
                                                 0};
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
    
    // No suitable block found, try to release memory from this process
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == process_id) {
            release_memory(memory_map[i], memory_map, map_cnt);
        }
    }

    // Retry allocation after releasing memory
    return first_fit_allocate(request_size, memory_map, map_cnt, process_id); 
}

// Worst Fit allocation with memory release
struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK worst_block = {-1, -1, -1, -1};
    int worst_diff = -1;
    int worst_fit_index = -1;

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
    } else {
        // No suitable block found, try to release memory from this process
        for (int i = 0; i < *map_cnt; i++) {
            if (memory_map[i].process_id == process_id) {
                release_memory(memory_map[i], memory_map, map_cnt);
            }
        }

        // Retry allocation after releasing memory
        return worst_fit_allocate(request_size, memory_map, map_cnt, process_id);
    }
    return worst_block;
}

// Next Fit allocation (with memory release)
struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    int start_index = 0;
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address == last_address) {
            start_index = i + 1;
            break;
        }
    }

    // Search from the start index to the end of the memory map
    for (int i = start_index; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            struct MEMORY_BLOCK allocated_block = memory_map[i];
            if (allocated_block.segment_size > request_size) { // Split if needed
                struct MEMORY_BLOCK new_block = {allocated_block.start_address + request_size,
                                                 allocated_block.end_address,
                                                 allocated_block.segment_size - request_size,
                                                 0};
                allocated_block.end_address = allocated_block.start_address + request_size - 1;
                allocated_block.segment_size = request_size;

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
    
    // Wrap around to the beginning and search from there until the start_index
    for (int i = 0; i < start_index; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            struct MEMORY_BLOCK allocated_block = memory_map[i];
            if (allocated_block.segment_size > request_size) {
                struct MEMORY_BLOCK new_block = {allocated_block.start_address + request_size,
                                                 allocated_block.end_address,
                                                 allocated_block.segment_size - request_size,
                                                 0};
                allocated_block.end_address = allocated_block.start_address + request_size - 1;
                allocated_block.segment_size = request_size;
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


    // No suitable block found, try to release memory from this process
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == process_id) {
            release_memory(memory_map[i], memory_map, map_cnt);
        }
    }

    // Retry allocation after releasing memory
    return next_fit_allocate(request_size, memory_map, map_cnt, process_id, last_address);
}



// Release memory and merge adjacent free blocks
void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt) {
    int index = -1;
    // Find the index of the freed block in the memory map
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].start_address == freed_block.start_address) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        // Mark the block as free
        memory_map[index].process_id = 0; 

        // Merge with the previous block if it's free
        if (index > 0 && memory_map[index - 1].process_id == 0) {
            memory_map[index - 1].end_address = memory_map[index].end_address;
            memory_map[index - 1].segment_size += memory_map[index].segment_size;
            // Shift the memory map to remove the merged block
            for (int i = index; i < *map_cnt - 1; i++) {
                memory_map[i] = memory_map[i + 1];
            }
            (*map_cnt)--; 
            index--; // Update index after shifting
        }

        // Merge with the next block if it's free
        if (index < *map_cnt - 1 && memory_map[index + 1].process_id == 0) {
            memory_map[index].end_address = memory_map[index + 1].end_address;
            memory_map[index].segment_size += memory_map[index + 1].segment_size;
            // Shift the memory map to remove the merged block
            for (int i = index + 1; i < *map_cnt - 1; i++) {
                memory_map[i] = memory_map[i + 1];
            }
            (*map_cnt)--;
        }
    }
}
