#ifndef CHECKALLOC
#define CHECKALLOC

#include <cstdlib>
#include <string>
#include <array>
#include <new>
#include <vector>
#include <sstream>
#include <iostream>

#define CHECKALLOC_NEW new(__FILE__, __LINE__)
#define CHECKALLOC_DELETE delete_loc(__FILE__, __LINE__), delete

// Contains pointer to memory, file and line that called for allocation
struct Allocated
{
	void* ptr;
	std::string file;
	int line;

	Allocated(void* in_ptr, std::string f, int ln) 
        : ptr(in_ptr), file(std::string(f)), line(ln) {}
};

const int ENTRIES = 3;

static std::vector<Allocated> allocator;
static std::string delete_file;
static int delete_line;

// Get number of objects allocated, call at end of program for memory leak checks
const size_t alloc_count()
{
	return allocator.size();
}

void* operator new(std::size_t size, const char* file, int line)
{
    void* memory = std::malloc(size);

    // Throw if bad alloc
    if (!memory)
    {
        std::stringstream ss("");
        ss << "BAD ALLOCATION AT " << file << ": " << line << "!\n";
        throw ss.str();
    }

    allocator.push_back(Allocated(memory, file, line));
    return memory;
}

void operator delete(void* ptr) noexcept(false)
{
    bool found = false;

    for (auto iter = allocator.begin(); iter < allocator.end(); ++iter)
    {
        if ((*iter).ptr == ptr)
        {
            // Free-fill (with zeroes for now)
            found = true;
            std::free(ptr);
            int* i_ptr = reinterpret_cast<int*>(ptr);
            *i_ptr = 00000000;
            allocator.erase(iter);
            return;
        }
    }

    // If not found in allocator, check address for free-fill value 
    // (detect likely double frees)
    if (!found)
    {
        int* i_ptr = reinterpret_cast<int*>(ptr);

        if (*i_ptr == 00000000)
        {
            std::stringstream ss("");
            ss << "LIKELY DOUBLE FREE AT "
                << delete_file << ": " << delete_line << "!\n";
            throw ss.str();
        }
    }
}

// Get information on delete call, for overloaded delete operator
void delete_loc(const char* file, int line)
{
    delete_file = file;
    delete_line = line;
}

typedef std::pair<std::string, int> alloc_info;

// Get file and line where memory was allocated
alloc_info getinfo(void* ptr)
{
    for (auto iter = allocator.begin(); iter < allocator.end(); ++iter)
    {
        if ((*iter).ptr == ptr)
        {
            return alloc_info((*iter).file, (*iter).line);
        }
    }

    std::stringstream ss("");
    ss << "CANNOT GET ALLOC INFO, POINTER TO FREED (OR NEVER ALLOCATED) MEMORY\n";
    throw ss.str();
}

#endif