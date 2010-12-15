/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2008 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2010      Cisco Systems, Inc. All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */
/** @file
 *
 */

#ifndef OPAL_RING_BUFFER_H
#define OPAL_RING_BUFFER_H

#include "opal_config.h"

#include "opal/threads/threads.h"
#include "opal/class/opal_object.h"
#include "opal/util/output.h"

BEGIN_C_DECLS

/**
 * dynamic pointer ring
 */
struct opal_ring_buffer_t {
    /** base class */
    opal_object_t super;
    /** synchronization object */
    opal_mutex_t lock;
    opal_condition_t cond;
    bool in_use;
    /* head/tail indices */
    char **head;
    char **tail;
    /** size of list, i.e. number of elements in addr */
    int size;
    /** pointer to ring */
    char **addr;
};
/**
 * Convenience typedef
 */
typedef struct opal_ring_buffer_t opal_ring_buffer_t;
/**
 * Class declaration
 */
OPAL_DECLSPEC OBJ_CLASS_DECLARATION(opal_ring_buffer_t);

/**
 * Initialize the ring buffer, defining its size.
 *
 * @param ring Pointer to a ring buffer (IN/OUT)
 * @param size The number of elements in the ring (IN)
 *
 * @return OPAL_SUCCESS if all initializations were succesful. Otherwise,
 *  the error indicate what went wrong in the function.
 */
OPAL_DECLSPEC int opal_ring_buffer_init(opal_ring_buffer_t* ring, int size);

/**
 * Push an item onto the ring buffer
 *
 * @param ring Pointer to ring (IN)
 * @param ptr Pointer value (IN)
 *
 * @return OPAL_SUCCESS. Returns error if ring cannot take
 *  another entry
 */
static inline void* opal_ring_buffer_push(opal_ring_buffer_t *ring, void *ptr)
{
    char *p=NULL;
    
    OPAL_ACQUIRE_THREAD(&(ring->lock), &(ring->cond), &(ring->in_use));
    if (NULL != *ring->head) {
        p = *ring->head;
        if (ring->head == ring->tail) {
            /* push the tail ahead of us */
            if (ring->tail == &ring->addr[ring->size-1]) {
                ring->tail = &ring->addr[0];
            } else {
                ring->tail++;
            }
        }
    }
    *ring->head = (char *) ptr;
    if (ring->head == &ring->addr[ring->size-1]) {
        ring->head = &ring->addr[0];
    } else {
        ring->head++;
    }
    OPAL_RELEASE_THREAD(&(ring->lock), &(ring->cond), &(ring->in_use));
    return (void*)p;
}


/**
 * Pop an item off of the ring. The head of the ring will be
 * returned. If nothing on the ring, NULL is returned.
 *
 * @param ring          Pointer to ring (IN)
 *
 * @return Error code.  NULL indicates an error.
 */

static inline void* opal_ring_buffer_pop(opal_ring_buffer_t *ring)
{
    char *p;

    OPAL_ACQUIRE_THREAD(&(ring->lock), &(ring->cond), &(ring->in_use));
    if (NULL == ring->tail || ring->head == ring->tail) {
        p = NULL;
    } else {
        p = *ring->tail;
        *ring->tail = NULL;
        if (ring->tail == &ring->addr[ring->size-1]) {
            ring->tail = &ring->addr[0];
        } else {
            ring->tail++;
        }
    }
    OPAL_RELEASE_THREAD(&(ring->lock), &(ring->cond), &(ring->in_use));
    return (void*)p;
}


END_C_DECLS

#endif /* OPAL_RING_BUFFER_H */