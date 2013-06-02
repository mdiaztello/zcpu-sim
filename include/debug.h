#ifndef INCLUDE_GUARD__DEBUG_H_
#define INCLUDE_GUARD__DEBUG_H_

#include <stdio.h>

#define DEBUG 1

#ifdef DEBUG
#define dprint(...); printf(__VA_ARGS__);
#define crashprint(); dprint("\nDid we crash here? LINE: %d FILE: %s\n\n", __LINE__, __FILE__);
#define beacon();   dprint("\n\nNOW EXECUTING LINE %d OF FUNCTION \"%s\" IN FILE %s\n\n", __LINE__, __func__, __FILE__);
#else
#define dprint(...);
#endif


#endif //INCLUDE_GUARD__DEBUG_H_
