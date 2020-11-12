/*-----------------------------------------------------------------------------
 *
 * memory_instrumentation.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef MEMORY_INSTRUMENTATION_H_
#define MEMORY_INSTRUMENTATION_H_

#include "common.h"

#define MEMDEBUG_CONTEXT_NAME "MemoryInstrumentationContext"

extern void setupMemInstrumentation(void);
extern void shutdownMemInstrumentation(void);
extern void addContext(char *name, unsigned int allocationSize, boolean acquired);
extern void addContextUnused(char *name, unsigned long unusedSize);
extern void addContextChunkInfo(char *name, unsigned long newChunkSize);
extern void outputMemstats(boolean showDetails);

#endif /* MEMORY_INSTRUMENTATION_H_ */
