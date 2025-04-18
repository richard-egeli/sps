#include "sps.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define sps_error(msg)                                            \
    do {                                                          \
        fprintf(stderr,                                           \
                "sparse set assertion error in %s (%s:%d): %s\n", \
                __func__,                                         \
                __FILE_NAME__,                                    \
                __LINE__,                                         \
                (msg));                                           \
        abort();                                                  \
    } while (0)
#else
#define sps_error(msg) (void)msg
#endif

void *sps_iter_next(sparse_set_iter_t *iter, uint16_t *index) {
    if (index == NULL || iter == NULL) {
        sps_error("invalid function paramaters");
        return NULL;
    }

    if (iter->index >= iter->set->count) {
        return NULL;
    }

    *index          = iter->set->dense[iter->index];
    void *component = (char *)iter->set->components + (iter->index * iter->set->component_size);
    iter->index++;
    return component;
}

sparse_set_iter_t sps_iter_new(sparse_set_t *set) {
    if (set == NULL) {
        sps_error("sparse set is invalid");
    }

    return (sparse_set_iter_t){
        .set   = set,
        .index = 0,
    };
}

size_t sps_count(const sparse_set_t *set) {
    if (set == NULL) {
        sps_error("set cannot be NULL");
        return 0;
    }

    return set->count;
}

void sps_sort(sparse_set_t *set, sps_sort_func_t compare, void *context) {
    if (set == NULL || compare == NULL) {
        sps_error("invalid arguments");
        return;
    }

    if (set->count <= 1) {
        return;  // Already sorted or empty
    }

    // Create a temporary array to store the original order
    uint16_t *order = malloc(set->count * sizeof(uint16_t));
    if (order == NULL) {
        sps_error("failed to allocate temporary order array");
        return;
    }

    // Initialize with current order
    for (uint16_t i = 0; i < set->count; i++) {
        order[i] = i;
    }

    // Perform insertion sort (stable) with the custom comparator
    for (uint16_t i = 1; i < set->count; i++) {
        uint16_t key   = order[i];
        void *key_comp = (char *)set->components + (key * set->component_size);
        int j          = i - 1;

        // Move elements that are greater than key to one position ahead
        while (j >= 0) {
            void *comp_j = (char *)set->components + (order[j] * set->component_size);
            if (compare(comp_j, key_comp, context) <= 0) {
                break;
            }
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    // Allocate temporary buffers
    void *temp_components = malloc(set->count * set->component_size);
    uint16_t *temp_dense  = malloc(set->count * sizeof(uint16_t));

    if (temp_components == NULL || temp_dense == NULL) {
        sps_error("failed to allocate temporary buffers");
        free(order);
        if (temp_components) free(temp_components);
        if (temp_dense) free(temp_dense);
        return;
    }

    // Rebuild arrays based on the sorted order
    for (uint16_t i = 0; i < set->count; i++) {
        uint16_t old_pos = order[i];

        // Copy component to temp buffer
        memcpy((char *)temp_components + (i * set->component_size),
               (char *)set->components + (old_pos * set->component_size),
               set->component_size);

        // Copy dense array entry
        temp_dense[i] = set->dense[old_pos];

        // Update sparse array to point to new position
        set->sparse[set->dense[old_pos]] = i;
    }

    // Copy temp buffers back to original arrays
    memcpy(set->components, temp_components, set->count * set->component_size);
    memcpy(set->dense, temp_dense, set->count * sizeof(uint16_t));

    // Clean up
    free(order);
    free(temp_components);
    free(temp_dense);
}

bool sps_has(sparse_set_t *set, uint16_t index) {
    if (set == NULL || index == SPARSE_SET_MAX) {
        sps_error("invalid arguments");
        return false;
    }

    return set->sparse[index] != SPARSE_SET_MAX;
}

void *sps_add_or_replace(sparse_set_t *set, uint16_t index, void *component) {
    if (set == NULL || component == NULL || index == SPARSE_SET_MAX) {
        sps_error("invalid arguments");
        return NULL;
    }

    set->sparse[index]     = set->count;
    set->dense[set->count] = index;
    void *target           = (char *)set->components + (set->count * set->component_size);
    memcpy(target, component, set->component_size);
    set->count++;
    return target;
}

void *sps_add(sparse_set_t *set, uint16_t index, void *component) {
    if (set == NULL || component == NULL || index == SPARSE_SET_MAX) {
        sps_error("invalid arguments");
        return NULL;
    }

    if (set->count == SPARSE_SET_MAX) {
        sps_error("sparse set is full");
        return NULL;
    }

    if (set->sparse[index] != UINT16_MAX) {
        sps_error("sparse set is already set at index");
        return NULL;
    }

    set->sparse[index]     = set->count;
    set->dense[set->count] = index;
    void *target           = (char *)set->components + (set->count * set->component_size);
    memcpy(target, component, set->component_size);
    set->count++;
    return target;
}

void sps_remove(sparse_set_t *set, uint16_t index) {
    if (set == NULL || index == SPARSE_SET_MAX) {
        sps_error("invalid arguments");
        return;
    }

    if (set->sparse[index] == SPARSE_SET_MAX) {
        sps_error("sparse index is not in use");
        return;
    }

    void *last_comp = (char *)set->components + ((set->count - 1) * set->component_size);
    void *target    = (char *)set->components + (set->sparse[index] * set->component_size);
    memcpy(target, last_comp, set->component_size);

    // cache the indexes
    uint16_t sparse_idx = set->dense[set->count - 1];
    uint16_t dense_idx  = set->sparse[index];

    // update indexes
    set->dense[dense_idx]   = sparse_idx;
    set->sparse[sparse_idx] = dense_idx;

    // invalidate old indexes
    set->dense[set->count - 1] = SPARSE_SET_MAX;
    set->sparse[index]         = SPARSE_SET_MAX;
    set->count--;
}

void *sps_get(sparse_set_t *set, uint16_t index) {
    if (set == NULL || index == SPARSE_SET_MAX) {
        sps_error("invalid arguments");
        return NULL;
    }

    if (set->sparse[index] == SPARSE_SET_MAX) {
        return NULL;
    }

    return (char *)set->components + (set->sparse[index] * set->component_size);
}

sparse_set_t *sps_new(size_t component_size) {
    if (component_size == 0) {
        return NULL;
    }

    sparse_set_t *sps = malloc(sizeof(*sps) + SPARSE_SET_MAX * component_size);
    if (sps == NULL) {
        sps_error("failed to allocate sparse set");
        return NULL;
    }

    memset(sps->dense, 0xFFU, sizeof(sps->dense));
    memset(sps->sparse, 0xFFU, sizeof(sps->sparse));
    sps->component_size = component_size;
    sps->count          = 0;
    return sps;
}

void sps_free(sparse_set_t *set) {
    free(set);
}
