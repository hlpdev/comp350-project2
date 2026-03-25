#include "history.h"

#include <stdlib.h>
#include <string.h>

struct history* history_create(size_t history_len) {
  struct history* h = malloc(sizeof(struct history));
  h->data = calloc(history_len, sizeof(char*));
  h->capacity = history_len;
  h->head = 0;
  h->count = 0;

  return h;
}

void history_free(struct history* h) {
  for (size_t i = 0; i < h->capacity; i++) {
    free(h->data[i]);
  }

  free(h->data);
  free(h);
}

void history_push(struct history* h, char* command, size_t len) {
  free(h->data[h->head]);
  h->data[h->head] = strndup(command, len);
  h->head = (h->head + 1) % h->capacity;
  if (h->count < h->capacity) {
    h->count++;
  }
}

void history_get(struct history* h, char* out, size_t len, size_t index) {
  if (index >= h->count) {
    out[0] = '\0';
    return;
  }

  size_t real_index = (h->head - 1 - index + h->capacity) % h->capacity;
  strncpy(out, h->data[real_index], len - 1);
  out[len - 1] = '\0';
}
