#include "memory_streambuf.h"

#if 0
memory_streambuf::memory_streambuf(const void * const buffer, size_t size)
: memory_streambuf(reinterpret_cast<const uint8_t* const>(buffer), size)
{
}

memory_streambuf::memory_streambuf(const uint8_t * const buffer, size_t size)
: _begin(buffer)
, _end(buffer+size)
, _current(buffer)
{
}

memory_streambuf::~memory_streambuf()
{
}
#endif