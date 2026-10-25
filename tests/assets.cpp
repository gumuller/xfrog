// SPDX-License-Identifier: GPL-2.0-or-later
#include "cfs.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
static void Require(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
int main(int argc,char** argv) {
    bool invalid=argc==3 && !strcmp(argv[2],"--invalid");
    try {
        bool loaded=CFS_Init(argc>1 ? argv[1] : nullptr);
        if(invalid) { CFS_Shutdown(); return loaded ? 1 : 0; }
        Require(loaded,"asset pack did not load");
        std::ifstream manifest(XFROG_ASSET_MANIFEST);
        Require(bool(manifest),"missing asset manifest");
        std::string name; int count=0;
        while(std::getline(manifest,name)) {
            CFS_File packed(const_cast<char*>(name.c_str()));
            Require(packed.Open(),"asset decompression failed");
            std::vector<byte> actual(packed.Size());
            Require(packed.Read(actual.data(),1,packed.Size()),"asset read failed");
            byte overflow;
            Require(!packed.Read(&overflow,1,1),"read past asset end was accepted");
            std::ifstream loose(std::string(XFROG_ASSET_ROOT)+"/"+name,std::ios::binary);
            Require(bool(loose),"loose asset missing");
            std::vector<byte> expected((std::istreambuf_iterator<char>(loose)),{});
            Require(actual==expected,"asset differs from original loose data");
            ++count;
        }
        Require(count==36,"expected 36 original assets");
        CFS_Shutdown(); printf("PASS: all %d assets match originals on this architecture; bounded reads\n",count); return 0;
    } catch(const std::exception& error) {
        CFS_Shutdown();
        if(invalid) { printf("PASS: rejected malformed pack: %s\n",error.what()); return 0; }
        fprintf(stderr,"FAIL: %s\n",error.what()); return 1;
    }
}
