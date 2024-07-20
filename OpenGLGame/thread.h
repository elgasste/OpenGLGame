#if !defined( THREAD_H )
#define THREAD_H

#include "common.h"

typedef struct
{
   void (*workerFnc)();
   void* data;
}
ThreadQueueEntry_t;

typedef struct
{
   uint32_t numThreads;

   uint32_t volatile completionGoal;
   uint32_t volatile completionCount;
   uint32_t volatile nextEntryToRead;
   uint32_t volatile nextEntryToWrite;

   ThreadQueueEntry_t entries[MAX_THREADQUEUE_SIZE];
}
ThreadQueue_t;

#endif
