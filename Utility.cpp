#include "Utility.h"

int MAX_SKYERS;
int MAX_SKI_LIFT;
int MAX_SLEEP_TIME;
int MIN_SLEEP_TIME;

bool compareClocks(const Data &s1, const Data &s2)
{
    if(s1.clock < s2.clock)
        return true;

    return (s1.clock == s2.clock && s1.ID < s2.ID);
}
