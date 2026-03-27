//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef _GLSURF_HPP_
#define _GLSURF_HPP_
#include <pddi/pddi.hpp>

class pglContext;
class pglWrapper;
#ifndef __DREAMCAST__
struct SDL_mutex;
#endif

class pglDisplay : public pddiDisplay
{
public:
    pglDisplay(pddiDisplayInfo* info);
    ~pglDisplay();

    // cross-platform functions
    bool InitDisplay(const pddiDisplayInit*);
    bool InitDisplay(int x, int y, int bpp);

    pddiDisplayInfo* GetDisplayInfo(void);

    int GetHeight(void);
    int GetWidth(void);
    int GetDepth(void);
    pddiDisplayMode GetDisplayMode(void);
    int GetNumColourBuffer(void);
    unsigned GetBufferMask(void);

    unsigned GetFreeTextureMem(void);

    void SwapBuffers(void);

    unsigned Screenshot(pddiColour* buffer, int nBytes);

#ifdef __DREAMCAST__
    long  ProcessWindowMessage(void* wnd, const void* event);
    void  SetWindow(void* wnd);
#else
    long  ProcessWindowMessage(SDL_Window* wnd, const SDL_WindowEvent* event);
    void  SetWindow(SDL_Window* wnd);
#endif

    // internal functions

    void BeginTiming();
    float EndTiming();

    void SetContext(pglContext* c) {context = c;}
    bool ExtBGRA(void) { return extBGRA;}
#ifdef RAD_GLES
    bool ExtBlend(void) { return extBlend; }
#endif
    bool CheckExtension(const char*);
    bool HasReset(void) { return reset; }
    bool GetForceVSync(void) { return m_ForceVSync; }

    void BeginContext(void);
    void EndContext(void);

    void SetGamma(float r, float g, float b);
    void GetGamma(float* r, float* g, float* b);


private:
    pddiDisplayMode mode;
    int winWidth;
    int winHeight;
    int winBitDepth;

    pddiDisplayInit displayInit;
    pddiDisplayInfo* displayInfo;

    pglContext* context;

#ifndef __DREAMCAST__
    unsigned short initialGammaRamp[3][256];
#endif
    float gammaR,gammaG,gammaB;

#ifdef __DREAMCAST__
    void* win;
#else
    SDL_Window* win;
#endif
    void* hRC;
    void* prevRC;

    bool extBGRA;
#ifdef RAD_GLES
    bool extBlend;
#endif
    bool reset;
    bool m_ForceVSync;

    float beginTime;
};

#endif
