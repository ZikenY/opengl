#if !defined(ZIKEN_BASICTYPE_INCLUDED_)
#define ZIKEN_BASICTYPE_INCLUDED_

//#include <inttypes.h>

namespace ziken
{

#if !defined(NULL)
    #define NULL 0
#endif

#if !defined(byte)
    typedef unsigned char byte;
#endif

#if !defined(word)
    typedef unsigned short word;
#endif

#if !defined(dword)
    typedef unsigned long dword;
#endif

}

#endif
