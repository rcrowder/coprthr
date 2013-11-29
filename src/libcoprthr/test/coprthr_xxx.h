
#ifdef _coprthr_xxx_h
#define _coprthr_xxx_h

//#if defined(__coprthr_device__)


#if defined(__x86_64__) || defined(__i386__) || defined(__arm__) 

#include <pthread.h>

void coprthr_mutex_lock( void* p_mtx )
{ pthread_mutex_lock((pthread_mutex_t*)p_mtx); }

void coprthr_mutex_unlock( void* p_mtx)
{ pthread_mutex_unlock((pthread_mutex_t*)p_mtx); }

#elif defined(__epiphany__)

#include "e_mutex.h"

int __attribute__((noinline)) read_h( int* p) { return p[1]; }

void coprthr_mutex_lock( void* p_mtx ) 
{ while(read_h(p_mtx)); e_mutex_lock( p_mtx); }

void coprthr_mutex_unlock( void* p_mtx) 
{ e_mutex_unlock(p_mtx); }

#else

#error no supported architecture 

#endif


//#endif

#endif

