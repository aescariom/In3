#ifndef _O_NEW_H_
#define _O_NEW_H_

#include <stdlib.h>

void * operator new(size_t size);
void operator delete(void * ptr);

#endif