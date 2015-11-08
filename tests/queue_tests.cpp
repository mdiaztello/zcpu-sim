

#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdio.h>
#include "queue.h"
}

const uint8_t TEST_QUEUE_SIZE = 100;
queue_t* queue;

TEST_GROUP(QUEUE_TESTS)
{

    void setup(void)
    {
        queue = queue_create(TEST_QUEUE_SIZE);
    }

    void teardown(void)
    {
        queue_destroy(queue);
    }


    void fill_queue(uint8_t num_elements_to_add)
    {
        for(int i = 0; i < num_elements_to_add; i++)
        {
            queue_put(queue, i);
        }
    }

    void fill_queue_halfway(void)
    {
        fill_queue(TEST_QUEUE_SIZE/2);
    }

    void fill_queue_almost_completely(void)
    {
        fill_queue(TEST_QUEUE_SIZE-1);
    }

    void fill_queue_completely(void)
    {
        fill_queue(TEST_QUEUE_SIZE);
    }
};


TEST(QUEUE_TESTS, queue_has_correct_size_on_creation)
{
    LONGS_EQUAL(TEST_QUEUE_SIZE, queue_get_size(queue));
}

TEST(QUEUE_TESTS, queue_can_not_have_size_zero)
{
    queue_t* q = queue_create(0);
    CHECK(NULL == q);
}


TEST(QUEUE_TESTS, queue_empty_on_creation)
{
    CHECK(queue_is_empty(queue) == true);
}

TEST(QUEUE_TESTS, queue_not_full_on_creation)
{
    CHECK(queue_is_full(queue) == false);
}

TEST(QUEUE_TESTS, queue_no_longer_empty_after_adding_something)
{
    uint8_t TEST_VALUE = 47;
    queue_put(queue, TEST_VALUE);

    CHECK(queue_is_empty(queue) == false);
}

TEST(QUEUE_TESTS, queue_becomes_empty_again_when_removing_from_a_1_element_queue)
{
    const uint8_t TEST_VALUE = 44;
    (void)queue_put(queue, TEST_VALUE);
    CHECK(queue_is_empty(queue) == false);
    (void)queue_get(queue);
    CHECK(queue_is_empty(queue) == true);
}

TEST(QUEUE_TESTS, partially_filled_queues_are_neither_empty_nor_full)
{
    fill_queue_halfway();
    CHECK(queue_is_empty(queue) == false);
    CHECK(queue_is_full(queue) == false);
}

TEST(QUEUE_TESTS, queues_can_never_be_simultaneously_full_and_empty)
{
    CHECK(queue_is_empty(queue) == true);
    CHECK(queue_is_full(queue) == false);
    fill_queue_completely();
    CHECK(queue_is_empty(queue) == false);
    CHECK(queue_is_full(queue) == true);
}

TEST(QUEUE_TESTS, queue_reports_full_when_completely_filled)
{
    fill_queue_completely();
    CHECK(queue_is_full(queue) == true);
}

TEST(QUEUE_TESTS, queue_will_not_report_full_until_completely_filled)
{
    const uint8_t TEST_VALUE = 77;
    fill_queue_almost_completely();
    CHECK(queue_is_full(queue) == false);
    queue_put(queue, TEST_VALUE);
    CHECK(queue_is_full(queue) == true);
}

TEST(QUEUE_TESTS, removing_from_a_full_queue_means_the_queue_is_no_longer_full)
{
    fill_queue_completely();
    CHECK(queue_is_full(queue) == true);
    (void)queue_get(queue);
    CHECK(queue_is_full(queue) == false);
}

TEST(QUEUE_TESTS, removing_from_an_empty_queue_will_fail)
{
    CHECK(queue_is_empty(queue) == true);
    queue_return_data_t queue_return_data = queue_get(queue);
    CHECK(QUEUE_IS_EMPTY_ERROR == queue_return_data.return_data_status);
}

TEST(QUEUE_TESTS, removing_from_an_empty_queue_has_no_effect_on_emptiness_status)
{
    CHECK(queue_is_empty(queue) == true);
    (void)queue_get(queue);
    CHECK(queue_is_empty(queue) == true);
}

TEST(QUEUE_TESTS, removing_from_a_non_empty_queue_will_always_succeeed)
{
    fill_queue_halfway();
    CHECK(queue_is_full(queue) == false);
    CHECK(queue_is_empty(queue) == false);
    queue_return_data_t queue_return_data = queue_get(queue);
    CHECK(QUEUE_DATA_RETRIEVED_SUCCESSFULLY == queue_return_data.return_data_status);
}

TEST(QUEUE_TESTS, adding_to_a_non_full_queue_will_always_succeeed)
{
    const uint8_t TEST_VALUE = 47;
    CHECK(queue_is_full(queue) == false);
    CHECK(queue_put(queue, TEST_VALUE) == true);
}

TEST(QUEUE_TESTS, adding_to_a_full_queue_will_fail)
{
    const uint8_t TEST_VALUE = 47;
    fill_queue_completely();
    CHECK(queue_is_full(queue) == true);
    CHECK(queue_put(queue, TEST_VALUE) == false);
}

TEST(QUEUE_TESTS, adding_to_a_full_queue_has_no_effect_on_fullness_status)
{
    const uint8_t TEST_VALUE = 47;
    fill_queue_completely();
    CHECK(queue_is_full(queue) == true);
    queue_put(queue, TEST_VALUE);
    CHECK(queue_is_full(queue) == true);
}

TEST(QUEUE_TESTS, data_is_retrievable_from_queue)
{
    const uint8_t TEST_VALUE = 47;
    queue_put(queue, TEST_VALUE);
    queue_return_data_t queue_return_data = queue_get(queue);
    LONGS_EQUAL(TEST_VALUE, queue_return_data.value);
}

TEST(QUEUE_TESTS, queue_is_a_FIFO_structure)
{
    queue_put(queue, 1);
    queue_put(queue, 2);
    queue_put(queue, 3);
    queue_put(queue, 4);

    queue_return_data_t queue_return_data = queue_get(queue);
    LONGS_EQUAL(1, queue_return_data.value);
    queue_return_data = queue_get(queue);
    LONGS_EQUAL(2, queue_return_data.value);
    queue_return_data = queue_get(queue);
    LONGS_EQUAL(3, queue_return_data.value);
    queue_return_data = queue_get(queue);
    LONGS_EQUAL(4, queue_return_data.value);
}

TEST(QUEUE_TESTS, queue_works_properly_even_after_heavy_use)
{
    //checks to see that the queue will work properly after it has been totally
    //filled and then totally emptied

    for(int i = 0; i < TEST_QUEUE_SIZE; i++)
    {
        queue_put(queue, i+100);
    }
    CHECK(queue_is_full(queue) == true);

    for(int i = 0; i < TEST_QUEUE_SIZE; i++)
    {
        queue_return_data_t queue_return_data = queue_get(queue);
        LONGS_EQUAL(i+100, queue_return_data.value);
    }
    CHECK(queue_is_empty(queue) == true);

    for(int i = 0; i < TEST_QUEUE_SIZE; i++)
    {
        queue_put(queue, i+20);
    }
    CHECK(queue_is_full(queue) == true);

    for(int i = 0; i < TEST_QUEUE_SIZE; i++)
    {
        queue_return_data_t queue_return_data = queue_get(queue);
        LONGS_EQUAL(i+20, queue_return_data.value);
    }
    CHECK(queue_is_empty(queue) == true);
}


