// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "../types.h"
struct PaletteEntry { byte peRed, peGreen, peBlue, peFlags; };
const int PC_NOCOLLAPSE = 0;
enum surface_mem_t { MEM_VIDEO, MEM_SYSTEM };
class Video {
public:
    Video() = default;
    ~Video();
    bool SetMode(int32 width, int32 height);
    bool CreateFlipSystem();
    bool SetPalette() { return true; }
    void Flip();
    void WaitVsync() {}
    void ClearBackSurface(int32 color);
    byte* LockBackSurface() { return pixels.data(); }
    void UnlockBackSurface() {}
    byte* BackBuffer() { return pixels.data(); }
    int32 BackBufferPitch() const { return width; }
    void PutPixel(int32 x, int32 y, int32 color);
    uint32 PresentedFrames() const { return presented; }
    void Screenshot(const char* path);
    SDL_Window* Window() const { return window; }
    friend class Surface;
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    int width = 0, height = 0;
    uint32 presented = 0;
    std::vector<byte> pixels;
    std::vector<Uint32> rgba;
};
class Surface {
public:
    bool Create(int32 width, int32 height, surface_mem_t);
    void Clear(int32 color);
    byte* Lock() { return pixels.data(); }
    void Unlock() {}
    void Blit(int32 x, int32 y, bool transparent);
    void Mirror();
    byte* GetBuffer() { return pixels.data(); }
    int32 GetPitch() const { return width; }
    int32 GetWidth() const { return width; }
    int32 GetHeight() const { return height; }
private:
    int width = 0, height = 0;
    std::vector<byte> pixels;
};
bool VideoInit();
void VideoShutdown();
void VideoUpdate();
extern Video* g_video;
extern PaletteEntry g_actual_palette[256];
