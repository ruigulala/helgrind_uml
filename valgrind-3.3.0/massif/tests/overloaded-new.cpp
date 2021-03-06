// operator new(unsigned)
// operator new[](unsigned)
// operator new(unsigned, std::nothrow_t const&)
// operator new[](unsigned, std::nothrow_t const&)

#include <stdlib.h>

#include <new>

using std::nothrow_t;

// A big structure.  Its details don't matter.
typedef struct {
           int array[1000];
        } s;

void* operator new (std::size_t n) throw (std::bad_alloc)
{
    return malloc(n);
}

void* operator new (std::size_t n, std::nothrow_t const &) throw ()
{
    return malloc(n);
}

void* operator new[] (std::size_t n) throw (std::bad_alloc)
{
    return malloc(n);
}

void* operator new[] (std::size_t n, std::nothrow_t const &) throw ()
{
    return malloc(n);
}

void operator delete (void* p)
{
    return free(p);
}

void operator delete[] (void* p)
{
    return free(p);
}

int main(void)
{
    s*        p1 = new                s;
    s*        p2 = new (std::nothrow) s;
    char*     c1 = new                char[2000];
    char*     c2 = new (std::nothrow) char[2000];
    delete p1;
    delete p2;
    delete [] c1;
    delete [] c2;
    return 0;
}


