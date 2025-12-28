#include "ringbuf.h"

void ringbuf_init(ringbuf_t* rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->dropped = 0;
}

size_t ringbuf_free(const ringbuf_t* rb) {
    // (tail - head - 1) mod SIZE daje nam wolne miejsce
    // -1 bo rezervujemy jedno miejsce na koniec (żeby head != tail gdy pełny)
    // Inaczej nie moglibyśmy rozróżnić pełny bufor od pustego
    if (rb->head >= rb->tail) {
        return RINGBUF_SIZE - (rb->head - rb->tail) - 1;
    } else {
        return rb->tail - rb->head - 1;
    }
}

size_t ringbuf_count(const ringbuf_t* rb) {
    // ile bajtów jest w buforze (ready to read)
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    } else {
        return RINGBUF_SIZE - (rb->tail - rb->head);
    }
}

int ringbuf_put(ringbuf_t* rb, uint8_t byte) {
    size_t next_head = (rb->head + 1) % RINGBUF_SIZE;
    
    // jeśli next_head == tail, to bufor jest pełny
    // Polityka: odrzucamy nowy bajt i zwiększamy dropped
    if (next_head == rb->tail) {
        rb->dropped++;
        return 0;
    }
    
    rb->q[rb->head] = byte;
    rb->head = next_head;
    return 1;
}

int ringbuf_get(ringbuf_t* rb, uint8_t* out) {
    // jeśli head == tail, to bufor jest pusty (nic do czytania)
    if (rb->head == rb->tail) {
        return 0;
    }
    
    *out = rb->q[rb->tail];
    rb->tail = (rb->tail + 1) % RINGBUF_SIZE;
    return 1;
}
