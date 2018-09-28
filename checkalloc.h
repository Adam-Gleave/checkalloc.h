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

struct Allocated
{
	void* ptr;
	std::string file;
	int line;

	Allocated(void* in_ptr, std::string f, int ln) : ptr(in_ptr), file(std::string(f)), line(ln) {}
};

const int ENTRIES = 3;
const int INDEX_SIZE = 10;

static std::vector<Allocated> allocator;
static std::string delete_file;
static int delete_line;

const size_t alloc_count()
{
	return allocator.size();
}

void* operator new(std::size_t size, const char* file, int line)
{
    void* memory = std::malloc(size);

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
			found = true;
            std::free(ptr);
			int* i_ptr = reinterpret_cast<int*>(ptr);
			*i_ptr = 00000000;
			allocator.erase(iter);
			return;
        }
    }

	if (!found)
	{
		int* i_ptr = reinterpret_cast<int*>(ptr);
		
		if (*i_ptr == 00000000)
		{
			std::stringstream ss("");
			ss << "FREEING MEMORY ALREADY FREED (OR NOT ALLOCATED) AT " << delete_file << ": " << delete_line << "!\n";
			throw ss.str();
		}
	}
}

void delete_loc(const char* file, int line)
{
	delete_file = file;
	delete_line = line;
}

typedef std::pair<std::string, int> alloc_info;

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