/*
 * iojournal -- bounded ring helpers
 *
 * SPDX-License-Identifier: MIT
 */

#include "ij_internal.h"

#include <stdlib.h>
#include <string.h>

ij_status_t ij_ring_init(ij_ring_t *ring, size_t capacity)
{
    if (ring == NULL || capacity < IJ_QUEUE_CAPACITY_MIN || capacity > IJ_QUEUE_CAPACITY_MAX ||
        (capacity & (capacity - 1U)) != 0U) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }

    memset(ring, 0, sizeof(*ring));
    ring->slots = calloc(capacity, sizeof(*ring->slots));
    ring->occupied = calloc(capacity, sizeof(*ring->occupied));
    if (ring->slots == NULL || ring->occupied == NULL) {
        free(ring->slots);
        free(ring->occupied);
        memset(ring, 0, sizeof(*ring));
        return IJ_STATUS_INTERNAL_ERROR;
    }
    if (mtx_init(&ring->lock, mtx_plain) != thrd_success) {
        free(ring->slots);
        free(ring->occupied);
        memset(ring, 0, sizeof(*ring));
        return IJ_STATUS_INTERNAL_ERROR;
    }

    ring->capacity = capacity;
    ring->mask = capacity - 1U;
    atomic_init(&ring->head, 0U);
    atomic_init(&ring->tail, 0U);
    atomic_init(&ring->count, 0U);
    atomic_init(&ring->dropped_events, 0U);
    atomic_init(&ring->enqueued_events, 0U);
    atomic_init(&ring->high_water_mark, 0U);
    return IJ_STATUS_OK;
}

void ij_ring_destroy(ij_ring_t *ring)
{
    if (ring == NULL || ring->slots == NULL) {
        return;
    }

    if (mtx_lock(&ring->lock) == thrd_success) {
        for (size_t i = 0U; i < ring->capacity; ++i) {
            if (ring->occupied[i]) {
                ij_event_copy_dispose(&ring->slots[i]);
            }
        }
        if (mtx_unlock(&ring->lock) != thrd_success) {
            memset(ring, 0, sizeof(*ring));
            return;
        }
    }

    mtx_destroy(&ring->lock);
    free(ring->slots);
    free(ring->occupied);
    memset(ring, 0, sizeof(*ring));
}

ij_status_t ij_ring_enqueue(ij_ring_t *ring, ij_event_copy_t *event)
{
    size_t count;
    size_t tail_index;

    if (ring == NULL || event == NULL || ring->slots == NULL) {
        return IJ_STATUS_INVALID_ARGUMENT;
    }
    if (mtx_lock(&ring->lock) != thrd_success) {
        return IJ_STATUS_INTERNAL_ERROR;
    }

    count = atomic_load_explicit(&ring->count, memory_order_relaxed);
    if (count >= ring->capacity) {
        atomic_fetch_add_explicit(&ring->dropped_events, 1U, memory_order_relaxed);
        if (mtx_unlock(&ring->lock) != thrd_success) {
            return IJ_STATUS_INTERNAL_ERROR;
        }
        return IJ_STATUS_QUEUE_FULL;
    }

    tail_index = atomic_load_explicit(&ring->tail, memory_order_relaxed) & ring->mask;
    ring->slots[tail_index] = *event;
    ring->occupied[tail_index] = true;
    memset(event, 0, sizeof(*event));

    atomic_store_explicit(&ring->tail, atomic_load_explicit(&ring->tail, memory_order_relaxed) + 1U,
                          memory_order_release);
    count += 1U;
    atomic_store_explicit(&ring->count, count, memory_order_release);
    atomic_fetch_add_explicit(&ring->enqueued_events, 1U, memory_order_relaxed);
    if (count > atomic_load_explicit(&ring->high_water_mark, memory_order_relaxed)) {
        atomic_store_explicit(&ring->high_water_mark, count, memory_order_relaxed);
    }

    if (mtx_unlock(&ring->lock) != thrd_success) {
        return IJ_STATUS_INTERNAL_ERROR;
    }
    return IJ_STATUS_OK;
}

bool ij_ring_try_dequeue(ij_ring_t *ring, ij_event_copy_t *out_event)
{
    size_t count;
    size_t head_index;

    if (ring == NULL || out_event == NULL || ring->slots == NULL) {
        return false;
    }
    if (mtx_lock(&ring->lock) != thrd_success) {
        return false;
    }

    count = atomic_load_explicit(&ring->count, memory_order_relaxed);
    if (count == 0U) {
        if (mtx_unlock(&ring->lock) != thrd_success) {
            return false;
        }
        return false;
    }

    head_index = atomic_load_explicit(&ring->head, memory_order_relaxed) & ring->mask;
    *out_event = ring->slots[head_index];
    memset(&ring->slots[head_index], 0, sizeof(ring->slots[head_index]));
    ring->occupied[head_index] = false;

    atomic_store_explicit(&ring->head, atomic_load_explicit(&ring->head, memory_order_relaxed) + 1U,
                          memory_order_release);
    atomic_store_explicit(&ring->count, count - 1U, memory_order_release);

    if (mtx_unlock(&ring->lock) != thrd_success) {
        return false;
    }
    return true;
}
