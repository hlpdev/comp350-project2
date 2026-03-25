#pragma once

#include <stddef.h>

struct history {
  char** data;
  size_t capacity;
  size_t head;
  size_t count;
};

struct history* history_create(size_t history_len);
void history_free(struct history* history);

void history_push(struct history* history, char* command, size_t len);
void history_get(struct history* history, char* out, size_t len, size_t index);
