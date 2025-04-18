/**
 * @file sparse_h
 * @brief A sparse set implementation for efficient entity-component systems
 *
 * This sparse set provides O(1) lookups, insertions, and removals
 * using a sparse-dense array mapping technique. It's particularly
 * useful for game engine entity-component systems.
 */

#ifndef SPARSE_H_
#define SPARSE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Maximum capacity of the sparse set */
#define SPARSE_SET_MAX (UINT16_MAX)

/**
 * @brief Sparse set data structure
 *
 * Implements a sparse set with associated component data. The set maintains
 * a sparse array mapping entity IDs to dense array indices, and a dense array
 * for fast iteration of active entities.
 */
typedef struct sparse_set {
    uint16_t count;                  /**< Number of active entities in the set */
    size_t component_size;           /**< Size of individual component in bytes */
    uint16_t sparse[SPARSE_SET_MAX]; /**< Maps entity index to position in dense array */
    uint16_t dense[SPARSE_SET_MAX];  /**< Stores active entity indices in packed format */
    uint8_t components[];            /**< Component data associated with entities */
} sparse_set_t;

/**
 * @brief Iterator for sparse set traversal
 *
 * Provides sequential access to components and their associated indices.
 */
typedef struct sparse_set_iter {
    sparse_set_t* set; /**< Set being iterated */
    uint16_t index;    /**< Current iteration index */
} sparse_set_iter_t;

/**
 * @brief Function type for custom component sorting
 *
 * @param c1 Pointer to first component to compare
 * @param c2 Pointer to second component to compare
 * @param ctx Context pointer for additional sorting data
 * @return Negative if c1 < c2, 0 if equal, positive if c1 > c2
 */
typedef int (*sps_sort_func_t)(const void* c1, const void* c2, void* ctx);

/**
 * @brief Get next component from iterator
 *
 * @param iter Pointer to iterator instance
 * @param index Pointer to receive the entity index (can be NULL if not needed)
 * @return Pointer to the next component data, or NULL if iteration is complete
 */
void* sps_iter_next(sparse_set_iter_t* iter, uint16_t* index);

/**
 * @brief Create new iterator for the given set
 *
 * @param set Sparse set to iterate
 * @return Initialized iterator positioned at the start of the set
 */
sparse_set_iter_t sps_iter_new(sparse_set_t* set);

/**
 * @brief Get the number of entities in the sparse set
 *
 * Returns the current count of active entities stored in the set.
 *
 * @param set Pointer to the sparse set (must not be NULL)
 * @return Number of active entities in the set
 */
size_t sps_count(const sparse_set_t* set);

/**
 * @brief Sort components using insertion sort algorithm
 *
 * Rearranges components and maintains entity-component associations.
 *
 * @param set Sparse set to sort
 * @param compare Comparison function for ordering components
 * @param context User context passed to comparison function
 */
void sps_sort(sparse_set_t* set, sps_sort_func_t compare, void* context);

/**
 * @brief Check if an entity exists in the set
 *
 * @param set Sparse set to query
 * @param index Entity index to check
 * @return true if entity exists in set, false otherwise
 */
bool sps_has(sparse_set_t* set, uint16_t index);

/**
 * @brief Add or replace an entity component in the sparse set
 *
 * @param set Sparse set to modify
 * @param index Entity index to add or update
 * @param component Pointer to component data to copy (must be non-NULL)
 * @return Pointer to the component data in the set, or NULL on failure
 *         (failure can occur if the set is full when trying to add a new component)
 *
 * If the entity already exists in the set, its component data will be replaced.
 * If the entity doesn't exist, it will be added to the set.
 */
void* sps_add_or_replace(sparse_set_t* set, uint16_t index, void* component);

/**
 * @brief Add an entity with its component to the set
 *
 * @param set Sparse set to modify
 * @param index Entity index to add
 * @param component Pointer to component data to copy (must be non-NULL)
 * @return Pointer to the newly added component data, or NULL on failure
 *         (failure occurs if index already exists or set is full)
 */
void* sps_add(sparse_set_t* set, uint16_t index, void* component);

/**
 * @brief Remove an entity and its component from the set
 *
 * Maintains packed structure by swapping with the last entity in the dense array.
 *
 * @param set Sparse set to modify
 * @param index Entity index to remove
 */
void sps_remove(sparse_set_t* set, uint16_t index);

/**
 * @brief Get component data for an entity
 *
 * @param set Sparse set to query
 * @param index Entity index to look up
 * @return Pointer to component data, or NULL if entity doesn't exist in set
 */
void* sps_get(sparse_set_t* set, uint16_t index);

/**
 * @brief Create a new sparse set
 *
 * @param component_size Size of each component in bytes
 * @return Pointer to newly allocated sparse set, or NULL on allocation failure
 */
sparse_set_t* sps_new(size_t component_size);

/**
 * @brief Free a sparse set and its resources
 *
 * @param set Sparse set to free
 */
void sps_free(sparse_set_t* set);

#endif  // SPARSE_H_
