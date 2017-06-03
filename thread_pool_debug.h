//
//  thread_pool_debug.h
//  
//
//  Created by COLIN TERNDRUP on 6/3/17.
//
//

#ifndef _THREAD_POOL_DEBUG_H_
#define _THREAD_POOL_DEBUG_H_

#ifdef DEBUG
#include <stdio.h>
#define DPRINT(s) printf(s)
#else
#define DPRINT(s)
#endif

#endif // _THREAD_POOL_DEBUG_H_
