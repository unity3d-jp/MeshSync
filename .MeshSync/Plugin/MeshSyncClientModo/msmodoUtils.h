#pragma once

inline bool LXTypeMatch(const char *t1, const char *t2)
{
    return t1 && t2 && strcmp(t1, t2) == 0;
}
