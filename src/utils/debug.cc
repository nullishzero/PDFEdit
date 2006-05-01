#include "debug.h"

// if default debug level doesn't come from gcc command line, we use
// PANIC
#ifndef DEFAULT_DEBUG_LEVEL
#define DEFAULT_DEBUG_LEVEL debug::DBG_PANIC
#endif

namespace debug
{

unsigned int debugLevel = DEFAULT_DEBUG_LEVEL;


}
