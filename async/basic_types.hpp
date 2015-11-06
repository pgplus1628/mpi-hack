#pragma once

#include <utility>
#include <time.h>
#include <sys/time.h>

typedef std::pair<size_t, size_t> Range;

enum Role {
  ROLE_ACT; /* active side of a channel */
  ROLE_PAS; /* passive side of a channle */
};

#define TIMER(val) do { \
  struct timeval tm; \
  gettimeofday(&tm, NULL); \
  val = tm.tv_sec * 1000 + tm.tv_usec/1000; \
} while(0)
