#pragma once

#include <iostream>

#define ANALYZE 1

#define dbgs                                               \
  if (!ANALYZE) {                                          \
  } else                                                   \
    std::cout

