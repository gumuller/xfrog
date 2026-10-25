// SPDX-License-Identifier: GPL-2.0-or-later
#include "video.h"
#include "../tga.h"
#include <algorithm>
#include <stdexcept>
Video* g_video = nullptr;
PaletteEntry g_actual_palette[256] = {};
static void Check(bool ok) { if (!ok) throw std::runtime_error(SDL_GetError()); }
bool VideoInit() {
    g_video = new Video;
    if (!g_video->SetMode(640,480) || !g_video->CreateFlipSystem()) return false;
    Surface* palette = LoadTGA(const_cast<char*>("gfx/palette.tga"), MEM_SYSTEM, g_actual_palette);
    if (!palette) return false;
    delete palette;
    g_actual_palette[0] = {0,0,0,0};
    g_actual_palette[255] = {255,255,255,0};
    return true;
}
void VideoShutdown() { delete g_video; g_video = nullptr; }
void VideoUpdate() { g_video->Flip(); }
Video::~Video() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
bool Video::SetMode(int32 w, int32 h) {
    width=w; height=h;
    window=SDL_CreateWindow("Xfrog",w*2,h*2,SDL_WINDOW_RESIZABLE);
    if (!window) return false;
    renderer=SDL_CreateRenderer(window,nullptr);
    if (!renderer) return false;
    SDL_SetRenderVSync(renderer,1);
    Check(SDL_SetRenderLogicalPresentation(renderer,w,h,SDL_LOGICAL_PRESENTATION_LETTERBOX));
    return true;
}
bool Video::CreateFlipSystem() {
    pixels.resize(width*height);
    rgba.resize(width*height);
    texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,width,height);
    if (!texture) return false;
    return SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
}
void Video::Flip() {
    Uint32 colors[256];
    for (int i=0;i<256;++i) {
        const auto& p=g_actual_palette[i];
        colors[i]=0xff000000u | (Uint32(p.peRed)<<16) | (Uint32(p.peGreen)<<8) | p.peBlue;
    }
    for (size_t i=0;i<pixels.size();++i) rgba[i]=colors[pixels[i]];
    Check(SDL_UpdateTexture(texture,nullptr,rgba.data(),width*4));
    Check(SDL_SetRenderDrawColor(renderer,0,0,0,255));
    Check(SDL_RenderClear(renderer));
    Check(SDL_RenderTexture(renderer,texture,nullptr,nullptr));
    Check(SDL_RenderPresent(renderer));
    ++presented;
}
void Video::Screenshot(const char* path) {
    SDL_Surface* surface=SDL_CreateSurfaceFrom(width,height,SDL_PIXELFORMAT_ARGB8888,rgba.data(),width*4);
    if (!surface) throw std::runtime_error(SDL_GetError());
    bool saved=SDL_SaveBMP(surface,path);
    SDL_DestroySurface(surface);
    Check(saved);
}
void Video::ClearBackSurface(int32 color) { std::fill(pixels.begin(),pixels.end(),byte(color)); }
void Video::PutPixel(int32 x,int32 y,int32 color) {
    if(x>=0 && y>=0 && x<width && y<height) pixels[y*width+x]=byte(color);
}
bool Surface::Create(int32 w,int32 h,surface_mem_t) {
    if(w<=0 || h<=0) return false;
    width=w; height=h; pixels.resize(w*h); return true;
}
void Surface::Clear(int32 color) { std::fill(pixels.begin(),pixels.end(),byte(color)); }
void Surface::Blit(int32 x,int32 y,bool transparent) {
    // Clip both the source and destination; cars regularly cross screen edges.
    int left=std::max(0,-x), top=std::max(0,-y);
    int right=std::min(width,g_video->width-x), bottom=std::min(height,g_video->height-y);
    for(int sy=top;sy<bottom;++sy) for(int sx=left;sx<right;++sx) {
        byte p=pixels[sy*width+sx];
        if(!transparent || p) g_video->pixels[(sy+y)*g_video->width+sx+x]=p;
    }
}
void Surface::Mirror() {
    for(int y=0;y<height;++y) std::reverse(pixels.begin()+y*width,pixels.begin()+(y+1)*width);
}
