#pragma once

#include <string>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

class ReadOnlyFileMMap
{
public:
    typedef const uint8_t* const_iterator;
    ReadOnlyFileMMap(std::string filename="")
    {
        this->filename = filename;
    }
    
    ReadOnlyFileMMap(const char* filename_p=nullptr)
    {
        if( filename_p ) this->filename = filename_p;
    }

    ReadOnlyFileMMap(ReadOnlyFileMMap&& other) : p{other.p},fd{other.fd},filesize{other.filesize},filename{other.filename}
    {
        other.p = nullptr;
        other.fd = -1;
        other.filesize=-1;
        other.filename.clear();
    }
    
    bool open()
    {
        if( this->filesize == -1 )
        {
            fd = STDIN_FILENO;
            if( this->filename.length())
            {
                fd = ::open( this->filename.c_str(), O_RDONLY, S_IRUSR | S_IWUSR );
                
                if( fd < 0 ) return false;
            }
            struct stat s;
            if( fstat( fd, &s ) == -1 )
            {
                if( this->fd > STDIN_FILENO ) ::close( this->fd );
                return false;
            }
            this->p = mmap( NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd ,0 );
            
            if( this->p == MAP_FAILED )
            {
                if( this->fd > STDIN_FILENO ) ::close( this->fd );
                return false;
            }
            this->filesize = s.st_size;
        }
        
        return true;
        
    }
    int64_t size() const { return this->filesize; }
    const uint8_t* const_data() const { return (const uint8_t*)this->p; }
    const_iterator cbegin() const { return const_data(); }
    const_iterator cend() const { return const_data() + this->size(); }
    const_iterator begin() const { return const_data(); }
    const_iterator end() const { return const_data() + this->size(); }

    ReadOnlyFileMMap& operator=(ReadOnlyFileMMap&& other)
    {
        if (&other != this) {
            if( this->p )
            {
                munmap( this->p, this->filesize );
            }
            if( this->fd > STDIN_FILENO ) close( this->fd );
            this->p = other.p;
            this->fd = other.fd;
            this->filesize = other.filesize;
            this->filename = std::move( other.filename );
            other.p = nullptr;
            other.fd = -1;
            other.filesize = -1;
        }
        return *this;
    }
    
    ~ReadOnlyFileMMap()
    {
        if( this->p )
        {
            munmap( this->p, this->filesize );
        }
        if( this->fd > STDIN_FILENO ) close( this->fd );
    }
    
public:
    ReadOnlyFileMMap(const ReadOnlyFileMMap&) = delete;
    ReadOnlyFileMMap& operator=(const ReadOnlyFileMMap&) = delete;
    
private:
    void* p=nullptr;
    int fd=-1;
    int64_t filesize=-1;
    std::string filename;
};

