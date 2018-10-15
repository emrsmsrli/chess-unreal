#pragma once

//#define DEBUG

#ifdef DEBUG
#define MAKE_SURE(inexpr) ensure(inexpr)
#else
#define MAKE_SURE(inexpr)
#endif
