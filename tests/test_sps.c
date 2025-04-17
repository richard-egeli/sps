#include <stdlib.h>

#include <unity.h>

#include "sps.h"
#include "unity_internals.h"

static sparse_set_t *set = NULL;

void setUp(void) { set = sps_new(sizeof(int)); }

void tearDown(void) {
  sps_free(set);
  set = NULL;
}

static void test_sps_add(void) {
  int comp = 10;
  int *placed = sps_add(set, 5, &comp);
  TEST_ASSERT_NOT_NULL(placed);
  TEST_ASSERT_EQUAL(comp, *placed);
}

static void test_sps_get(void) {
  int comp = 42;
  sps_add(set, 7, &comp);

  int *retrieved = sps_get(set, 7);
  TEST_ASSERT_NOT_NULL(retrieved);
  TEST_ASSERT_EQUAL(comp, *retrieved);

  // Test getting non-existent component
  int *null_comp = sps_get(set, 8);
  TEST_ASSERT_NULL(null_comp);
}

static void test_sps_remove(void) {
  int comp1 = 10;
  int comp2 = 20;
  sps_add(set, 5, &comp1);
  sps_add(set, 8, &comp2);

  // Remove the first component
  sps_remove(set, 5);

  // Check it was removed
  TEST_ASSERT_NULL(sps_get(set, 5));

  // Check the other one still exists
  int *comp = sps_get(set, 8);
  TEST_ASSERT_NOT_NULL(comp);
  TEST_ASSERT_EQUAL(20, *comp);
}

static void test_sps_has(void) {
  int comp = 30;
  sps_add(set, 3, &comp);

  TEST_ASSERT_TRUE(sps_has(set, 3));
  TEST_ASSERT_FALSE(sps_has(set, 4));
}

static void test_sps_iter(void) {
  int values[] = {100, 200, 300};
  uint16_t indices[] = {10, 20, 30};

  // Add components
  for (int i = 0; i < 3; i++) {
    sps_add(set, indices[i], &values[i]);
  }

  // Test iteration
  sparse_set_iter_t iter = sps_iter_new(set);
  uint16_t idx;
  int count = 0;
  int found[3] = {0, 0, 0};

  int *comp;
  while ((comp = sps_iter_next(&iter, &idx)) != NULL) {
    // Find which component this is
    for (int i = 0; i < 3; i++) {
      if (idx == indices[i]) {
        TEST_ASSERT_EQUAL(values[i], *comp);
        found[i] = 1;
        break;
      }
    }
    count++;
  }

  TEST_ASSERT_EQUAL(3, count);
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL(1, found[i]);
  }
}

static void test_sps_full(void) {
  // Test adding up to capacity
  sparse_set_t *small_set = sps_new(sizeof(int));

  // Fill the set to capacity (using a smaller loop for testing)
  for (uint16_t i = 0; i < 100; i++) {
    int value = i;
    sps_add(small_set, i, &value);
  }

  // Verify all components are there
  for (uint16_t i = 0; i < 100; i++) {
    int *comp = sps_get(small_set, i);
    TEST_ASSERT_NOT_NULL(comp);
    TEST_ASSERT_EQUAL(i, *comp);
  }

  sps_free(small_set);
}

// Comparison function for sorting test
int compare_ints(const void *a, const void *b, void *context) {
  const int *int_a = (const int *)a;
  const int *int_b = (const int *)b;
  return *int_a - *int_b;
}

static void test_sps_sort(void) {
  int values[] = {30, 10, 20, 15, 25};
  uint16_t indices[] = {5, 6, 7, 8, 9};

  // Add components in unsorted order
  for (int i = 0; i < 5; i++) {
    sps_add(set, indices[i], &values[i]);
  }

  // Sort the components
  sps_sort(set, compare_ints, NULL);

  // Verify components are sorted but indices are maintained
  sparse_set_iter_t iter = sps_iter_new(set);
  uint16_t idx;
  int prev_value = 0;
  int count = 0;

  int *comp;
  while ((comp = sps_iter_next(&iter, &idx)) != NULL) {
    if (count > 0) {
      TEST_ASSERT_TRUE(*comp >= prev_value); // Check sorting
    }
    prev_value = *comp;

    // Verify this index still has the right value
    for (int i = 0; i < 5; i++) {
      if (idx == indices[i]) {
        TEST_ASSERT_EQUAL(values[i], *comp);
        break;
      }
    }
    count++;
  }

  TEST_ASSERT_EQUAL(5, count);
}

// Test behavior with modifications during iteration
static void test_sps_iter_with_modifications(void) {
  int values[] = {100, 200, 300, 400};

  // Add components
  for (int i = 0; i < 4; i++) {
    sps_add(set, i + 1, &values[i]);
  }

  // Start iteration
  sparse_set_iter_t iter = sps_iter_new(set);
  uint16_t idx;
  int count = 0;

  // Remove an element during iteration
  sps_remove(set, 2);

  int *comp;
  while ((comp = sps_iter_next(&iter, &idx)) != NULL) {
    // We've removed one, so we should have only 3 elements
    count++;
  }

  TEST_ASSERT_EQUAL(3, count);
}

// Test edge cases
static void test_sps_edge_cases(void) {
  // Test with NULL set
  TEST_ASSERT_NULL(sps_add(NULL, 1, &(int){10}));

  // Test with NULL component
  TEST_ASSERT_NULL(sps_add(set, 1, NULL));

  // Test with invalid index
  TEST_ASSERT_NULL(sps_add(set, SPARSE_SET_MAX, &(int){10}));

  // Test adding twice to same index
  int comp = 10;
  sps_add(set, 1, &comp);
  TEST_ASSERT_NULL(sps_add(set, 1, &comp));

  // Test removing non-existent index
  sps_remove(set, 2); // Should not crash

  // Test get on non-existent index
  TEST_ASSERT_NULL(sps_get(set, 2));
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();

  RUN_TEST(test_sps_add);
  RUN_TEST(test_sps_get);
  RUN_TEST(test_sps_remove);
  RUN_TEST(test_sps_has);
  RUN_TEST(test_sps_iter);
  RUN_TEST(test_sps_full);
  RUN_TEST(test_sps_sort);
  RUN_TEST(test_sps_iter_with_modifications);
  RUN_TEST(test_sps_edge_cases);

  return UNITY_END();
}
