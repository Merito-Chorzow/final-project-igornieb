#pragma once
#include <stdint.h>
#include <stddef.h>

#ifndef RINGBUF_SIZE
#define RINGBUF_SIZE 256
#endif

typedef struct {
    uint8_t q[RINGBUF_SIZE];
    size_t head;    // wskazuje na miejsce NASTĘPNEGO WPISANIA
    size_t tail;    // wskazuje na miejsce NASTĘPNEGO ODCZYTANIA
    size_t dropped; // licznik bajtów utraconych z powodu przepełnienia
} ringbuf_t;

void ringbuf_init(ringbuf_t* rb);
size_t ringbuf_free(const ringbuf_t* rb);
size_t ringbuf_count(const ringbuf_t* rb);

// zwraca 1 jeśli się udało, 0 jeśli bufor pełny (i bajt został odrzucony)
int ringbuf_put(ringbuf_t* rb, uint8_t byte);

// zwraca 1 jeśli się udało, 0 jeśli bufor pusty
int ringbuf_get(ringbuf_t* rb, uint8_t* out);
