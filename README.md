# sps — Sparse Set Library for C

A high-performance sparse set implementation in C17, designed for real-time applications such as ECS (Entity-Component Systems) in game engines. Provides O(1) add, remove, lookup, and iteration over components.

## Features

- Fast O(1) operations
- Stable insertion-sort for deterministic iteration order
- Iterator support for easy traversal
- Custom comparator-based sorting
- Fully tested with Unity test framework
- Zero dependencies (except for optional test framework)

## Example

```c
sparse_set_t *set = sps_new(sizeof(int));
int value = 42;
sps_add(set, 3, &value);

if (sps_has(set, 3)) {
    int *val = sps_get(set, 3);
    printf("Value: %d\n", *val);
}

sps_remove(set, 3);
sps_free(set);
```

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run Tests

```sh
cmake -B build -DBUILD_SPS_TESTS=ON
cmake --build build
./build/bin/test_sps
```

## API

See [`sps.h`](include/sps.h) for full documentation.

Main functions:

- `sps_new(size_t component_size)`
- `sps_add(sparse_set_t *set, uint16_t index, void *component)`
- `sps_get(sparse_set_t *set, uint16_t index)`
- `sps_remove(sparse_set_t *set, uint16_t index)`
- `sps_has(sparse_set_t *set, uint16_t index)`
- `sps_sort(sparse_set_t *set, sps_sort_func_t, void *ctx)`
- `sps_iter_new`, `sps_iter_next`
