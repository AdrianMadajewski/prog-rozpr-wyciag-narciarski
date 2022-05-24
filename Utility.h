#ifndef UTIL_H
#define UTIL_H

#include "Skyer.h"

extern int MAX_SKYERS;
extern int MAX_SKI_LIFT;
extern int MAX_SLEEP_TIME;
extern int MIN_SLEEP_TIME;

enum MPITag
{
    REQUEST = 0,
    REPLY, 
    RELEASE,
};

bool compareClocks(const Data &s1, const Data &s2);

#endif