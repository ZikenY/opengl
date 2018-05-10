#if !defined(ZIKEN_COMMONINCLUDE_INCLUDE_)
#define ZIKEN_COMMONINCLUDE_INCLUDE_

#pragma warning(disable: 4786)
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//#include <malloc.h>
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <math.h>
#include <signal.h>

#ifdef __GNUC__
#include <sys/time.h>
#include <dirent.h>
#endif
#ifdef _MSC_FULL_VER
#include <time.h>
#endif


#include "basictype.h"

using namespace std;

namespace ziken
{
    #ifdef _MSC_FULL_VER
    #ifndef max
    template <typename T>
    inline T max(const T& a, const T& b)
    {
        return a < b ? b : a;
    }
    template short max(const short &a, const short &b);
    template int max(const int &a, const int &b);
    template float max(const float &a, const float &b);
    template double max(const double &a, const double &b);
    #endif

    #ifndef min
    template <typename T>
    inline T min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }
    template short min(const short &a, const short &b);
    template int min(const int &a, const int &b);
    template float min(const float &a, const float &b);
    template double min(const double &a, const double &b);
    #endif
    #endif
}
#endif
