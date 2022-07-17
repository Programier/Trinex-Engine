#pragma once

#define DEBUG

#if defined(DEBUG)
#define DEBUG_CODE(code) code
#else
#define DEBUG_CODE(code)                                                                                                    \
    {}
#endif
