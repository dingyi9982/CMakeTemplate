#include "demo.h"
#include "RRLogger.h"

int GetData() {
  int ret = 1;
  RRLOG_DEBUG("GetData return: {}", ret);
  return ret;
}
