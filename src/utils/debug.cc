#include "debug.h"

// if default debug level doesn't come from gcc command line, we use
// CRIT
#ifndef DEFAULT_DEBUG_LEVEL
#define DEFAULT_DEBUG_LEVEL debug::DBG_CRIT
#endif

namespace debug
{

unsigned int debugLevel = DEFAULT_DEBUG_LEVEL;


}
