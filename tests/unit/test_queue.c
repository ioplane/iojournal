#include "ij_internal.h"

#include <unity/unity.h>

#include <threads.h>

static ij_event_t ij_test_queue_event(const char *event_name, const char *message)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-1",
                         .len = 5U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = event_name,
        .message = message,
        .logger = "iojournal.test",
        .attributes = attributes,
        .attribute_count = 1U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = NULL,
        .source_line = 0U,
        .source_function = NULL,
    };

    return event;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_ring_enqueue_then_dequeue_preserves_event_order(void)
{
    ij_ring_t ring = {0};
    ij_event_copy_t first = {0};
    ij_event_copy_t second = {0};
    ij_event_copy_t dequeued = {0};
    ij_event_t first_event = ij_test_queue_event("first.event", "first");
    ij_event_t second_event = ij_test_queue_event("second.event", "second");

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, IJ_QUEUE_CAPACITY_MIN));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&first, &first_event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&second, &second_event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_enqueue(&ring, &first));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_enqueue(&ring, &second));

    TEST_ASSERT_TRUE(ij_ring_try_dequeue(&ring, &dequeued));
    TEST_ASSERT_EQUAL_STRING("first.event", dequeued.event_name.data);
    ij_event_copy_dispose(&dequeued);

    TEST_ASSERT_TRUE(ij_ring_try_dequeue(&ring, &dequeued));
    TEST_ASSERT_EQUAL_STRING("second.event", dequeued.event_name.data);
    ij_event_copy_dispose(&dequeued);

    ij_ring_destroy(&ring);
}

void test_ring_reports_queue_full_when_capacity_is_exhausted(void)
{
    ij_ring_t ring = {0};
    ij_event_t event = ij_test_queue_event("pressure.event", "full");

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, IJ_QUEUE_CAPACITY_MIN));

    for (size_t i = 0; i < IJ_QUEUE_CAPACITY_MIN; ++i) {
        ij_event_copy_t copied_event = {0};

        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                     IJ_REDACTION_MODE_ENABLED));
        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_enqueue(&ring, &copied_event));
    }

    {
        ij_event_copy_t overflow_event = {0};

        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&overflow_event, &event,
                                                                     IJ_REDACTION_MODE_ENABLED));
        TEST_ASSERT_EQUAL_INT(IJ_STATUS_QUEUE_FULL, ij_ring_enqueue(&ring, &overflow_event));
        ij_event_copy_dispose(&overflow_event);
    }

    ij_ring_destroy(&ring);
}

void test_ring_dequeue_on_empty_ring_returns_false(void)
{
    ij_ring_t ring = {0};
    ij_event_copy_t dequeued = {0};

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, IJ_QUEUE_CAPACITY_MIN));
    TEST_ASSERT_FALSE(ij_ring_try_dequeue(&ring, &dequeued));
    ij_ring_destroy(&ring);
}

typedef struct {
    ij_ring_t *ring;
    size_t count;
    _Atomic size_t *enqueued;
} ij_producer_args_t;

static int ij_producer_thread(void *arg)
{
    ij_producer_args_t *args = (ij_producer_args_t *)arg;

    for (size_t i = 0; i < args->count; ++i) {
        ij_event_t event = ij_test_queue_event("stress.event", "test");
        ij_event_copy_t copied = {0};

        if (ij_event_copy_from_input(&copied, &event, IJ_REDACTION_MODE_ENABLED) != IJ_STATUS_OK) {
            continue;
        }
        if (ij_ring_enqueue(args->ring, &copied) == IJ_STATUS_OK) {
            atomic_fetch_add(args->enqueued, 1);
        } else {
            ij_event_copy_dispose(&copied);
        }
    }
    return 0;
}

void test_ring_mpsc_stress_four_producers(void)
{
    ij_ring_t ring = {0};
    _Atomic size_t enqueued = 0;
    thrd_t threads[4];
    ij_producer_args_t args[4];

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, 256));

    for (size_t i = 0; i < 4; ++i) {
        args[i] = (ij_producer_args_t){.ring = &ring, .count = 250, .enqueued = &enqueued};
        TEST_ASSERT_EQUAL_INT(thrd_success, thrd_create(&threads[i], ij_producer_thread, &args[i]));
    }

    for (size_t i = 0; i < 4; ++i) {
        int res = 0;
        TEST_ASSERT_EQUAL_INT(thrd_success, thrd_join(threads[i], &res));
    }

    size_t total_enqueued = atomic_load(&enqueued);
    size_t dropped = atomic_load(&ring.dropped_events);
    TEST_ASSERT_EQUAL_size_t(1000, total_enqueued + dropped);

    size_t dequeued_count = 0;
    ij_event_copy_t dequeued = {0};

    while (ij_ring_try_dequeue(&ring, &dequeued)) {
        ij_event_copy_dispose(&dequeued);
        ++dequeued_count;
    }

    TEST_ASSERT_EQUAL_size_t(1000, dequeued_count + dropped);

    ij_ring_destroy(&ring);
}

typedef struct {
    ij_ring_t *ring;
    _Atomic bool *done;
    _Atomic size_t *total_dequeued;
} ij_consumer_args_t;

static int ij_consumer_thread(void *arg)
{
    ij_consumer_args_t *ctx = (ij_consumer_args_t *)arg;
    ij_event_copy_t dequeued = {0};

    for (;;) {
        if (ij_ring_try_dequeue(ctx->ring, &dequeued)) {
            ij_event_copy_dispose(&dequeued);
            atomic_fetch_add(ctx->total_dequeued, 1);
        } else if (atomic_load(ctx->done)) {
            /* Drain any remaining items after producers finished. */
            while (ij_ring_try_dequeue(ctx->ring, &dequeued)) {
                ij_event_copy_dispose(&dequeued);
                atomic_fetch_add(ctx->total_dequeued, 1);
            }
            break;
        } else {
            thrd_yield();
        }
    }
    return 0;
}

void test_ring_concurrent_enqueue_dequeue(void)
{
    ij_ring_t ring = {0};
    _Atomic size_t enqueued = 0;
    _Atomic size_t total_dequeued = 0;
    _Atomic bool done = false;
    thrd_t producers[2];
    thrd_t consumer;
    ij_producer_args_t producer_args[2];

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, 256));

    ij_consumer_args_t consumer_args = {
        .ring = &ring, .done = &done, .total_dequeued = &total_dequeued};
    TEST_ASSERT_EQUAL_INT(thrd_success, thrd_create(&consumer, ij_consumer_thread, &consumer_args));

    for (size_t i = 0; i < 2; ++i) {
        producer_args[i] = (ij_producer_args_t){.ring = &ring, .count = 200, .enqueued = &enqueued};
        TEST_ASSERT_EQUAL_INT(thrd_success,
                              thrd_create(&producers[i], ij_producer_thread, &producer_args[i]));
    }

    for (size_t i = 0; i < 2; ++i) {
        int res = 0;
        TEST_ASSERT_EQUAL_INT(thrd_success, thrd_join(producers[i], &res));
    }

    atomic_store(&done, true);
    {
        int res = 0;
        TEST_ASSERT_EQUAL_INT(thrd_success, thrd_join(consumer, &res));
    }

    size_t dropped = atomic_load(&ring.dropped_events);
    size_t consumed = atomic_load(&total_dequeued);
    TEST_ASSERT_EQUAL_size_t(400, consumed + dropped);

    ij_ring_destroy(&ring);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ring_enqueue_then_dequeue_preserves_event_order);
    RUN_TEST(test_ring_reports_queue_full_when_capacity_is_exhausted);
    RUN_TEST(test_ring_dequeue_on_empty_ring_returns_false);
    RUN_TEST(test_ring_mpsc_stress_four_producers);
    RUN_TEST(test_ring_concurrent_enqueue_dequeue);
    return UNITY_END();
}
