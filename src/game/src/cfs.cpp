/*

Xfrog - a frogger clone.
Copyright (C) 2000 Gustavo S. Muller

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// The original disk records are always 80 bytes, including a discarded 32-bit
// linked-list pointer. Never deserialize a native pointer or zlib length from disk.
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <zlib.h>
#include "cfs.h"
#include "error.h"
#ifdef XFROG_SDL
#include <SDL3/SDL.h>
#endif
namespace {
struct Entry { char name[64]; uint32 offset,csize,usize; };
std::vector<Entry> entries;
FILE* pack=nullptr;
uint32 LE32(const byte* p) { return uint32(p[0]) | uint32(p[1])<<8 | uint32(p[2])<<16 | uint32(p[3])<<24; }
bool Fail(const char* message) {
    CFS_Shutdown();
    Error error(const_cast<char*>("%s"),message); error.Report(); return false;
}
}
bool CFS_Init(const char* filename) {
    if(pack) return true;
#ifdef XFROG_SDL
    const char* base=SDL_GetBasePath();
    if(!base) return Fail("Cannot locate game data");
    std::string path=std::string(base)+"xfrog.cfs";
    pack=fopen(filename ? filename : path.c_str(),"rb");
#else
    pack=fopen(filename ? filename : "xfrog.cfs","rb");
#endif
    if(!pack) return Fail("Cannot open xfrog.cfs");
    if(fseek(pack,0,SEEK_END)!=0) return Fail("Cannot read asset pack");
    long length=ftell(pack);
    rewind(pack);
    byte header[12];
    if(length<12 || fread(header,1,12,pack)!=12 || memcmp(header,"CFS\0",4)) return Fail("Invalid CFS header");
    uint32 count=LE32(header+4);
    if(count>uint32((length-12)/80)) return Fail("Invalid CFS file count");
    uint64_t offset=12;
    for(uint32 i=0;i<count;++i) {
        byte record[80];
        if(offset+80>uint64_t(length) || fseek(pack,long(offset),SEEK_SET) || fread(record,1,80,pack)!=80)
            return Fail("Truncated CFS record");
        Entry entry;
        memcpy(entry.name,record,64);
        entry.offset=LE32(record+64); entry.csize=LE32(record+68); entry.usize=LE32(record+72);
        if(!memchr(entry.name,0,64) || entry.offset!=offset || !entry.csize || !entry.usize ||
           entry.usize>64*1024*1024 || offset+80+entry.csize>uint64_t(length)) return Fail("Invalid CFS record");
        entries.push_back(entry);
        offset+=80+entry.csize;
    }
    return true;
}
void CFS_Shutdown() { entries.clear(); if(pack) fclose(pack); pack=nullptr; }
CFS_File::CFS_File(char* name) : m_name(nullptr),m_size(0),m_data(nullptr),m_ptr(nullptr) {
    m_name=new char[strlen(name)+1]; strcpy(m_name,name);
}
CFS_File::~CFS_File() { delete[] m_name; delete[] m_data; }
bool CFS_File::Open() {
    if(!pack) return false;
    for(const auto& entry:entries) if(!strcmp(entry.name,m_name)) {
        std::vector<byte> compressed(entry.csize);
        if(fseek(pack,long(entry.offset)+80,SEEK_SET) || fread(compressed.data(),1,entry.csize,pack)!=entry.csize) return false;
        delete[] m_data; m_data=new byte[entry.usize]; m_ptr=nullptr; m_size=0;
        uLongf size=entry.usize;
        int result=uncompress(m_data,&size,compressed.data(),uLong(entry.csize));
        if(result!=Z_OK || size!=entry.usize) return false;
        m_size=int32(size); m_ptr=m_data; return true;
    }
    Error error(const_cast<char*>("Missing asset: %s"),m_name); error.Report(); return false;
}
void CFS_File::Seek(int32 bytes,int32 from) {
    if(!m_ptr) return;
    int64_t base=from==SEEK_SET ? 0 : from==SEEK_CUR ? m_ptr-m_data : from==SEEK_END ? m_size : -1;
    int64_t position=base+int64_t(bytes);
    if(base<0 || position<0 || position>m_size) {
        Error error(const_cast<char*>("Invalid seek in %s"),m_name); error.Report(); return;
    }
    m_ptr=m_data+position;
}
bool CFS_File::Read(void* dest,int32 size,int32 count) {
    if(!m_ptr || size<0 || count<0) return false;
    uint64_t bytes=uint64_t(size)*uint64_t(count);
    if(bytes>uint64_t(m_size-(m_ptr-m_data))) return false;
    if(bytes) memcpy(dest,m_ptr,size_t(bytes));
    m_ptr+=bytes; return true;
}
