#include <stddef.h>
#include <stdlib.h>

void* operator new(size_t len)
{
	return malloc(len);
}

void operator delete(void* ptr)
{
	free(ptr);
}

void* operator new[](size_t len)
{
	return ::operator new(len);
}

void operator delete[](void* ptr)
{
	return ::operator delete(ptr);
}
