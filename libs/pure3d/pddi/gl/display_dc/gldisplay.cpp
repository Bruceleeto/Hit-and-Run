//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
// Dreamcast GLdc display implementation
//=============================================================================

#include <pddi/gl/gl.hpp>
#include <pddi/gl/glcon.hpp>
#include <pddi/gl/gldisplay.hpp>
#include <pddi/base/debug.hpp>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <kos.h>
#include <GL/gl.h>
#include <GL/glkos.h>

bool pglDisplay::CheckExtension( const char *extName )
{
    const char* exts = (const char*)glGetString(GL_EXTENSIONS);
    if( !exts || !extName ) return false;
    return ( strstr( exts, extName ) != NULL );
}

pglDisplay::pglDisplay(pddiDisplayInfo* info)
{
    displayInfo = info;
    mode = PDDI_DISPLAY_FULLSCREEN;
    winWidth = 640;
    winHeight = 480;
    winBitDepth = 16;

    context = NULL;

    win = NULL;
    hRC = NULL;
    prevRC = NULL;

    extBGRA = false;

    gammaR = gammaG = gammaB = 1.0f;

    reset = true;
    m_ForceVSync = false;
}

pglDisplay::~pglDisplay()
{
}

bool pglDisplay::InitDisplay(const pddiDisplayInit* init)
{
    displayInit = *init;

    winWidth = init->xsize;
    winHeight = init->ysize;
    winBitDepth = init->bpp;

    glKosInit();

    char* glVendor   = (char*)glGetString(GL_VENDOR);
    char* glRenderer = (char*)glGetString(GL_RENDERER);
    char* glVersion  = (char*)glGetString(GL_VERSION);

    extBGRA = CheckExtension("GL_EXT_bgra") ||
              CheckExtension("GL_EXT_texture_format_BGRA8888");

    printf("OpenGL - Vendor: %s, Renderer: %s, Version: %s\n",
           glVendor, glRenderer, glVersion);

    reset = false;
    return true;
}

bool pglDisplay::InitDisplay(int x, int y, int bpp)
{
    winWidth = x;
    winHeight = y;
    winBitDepth = bpp;
    return true;
}

pddiDisplayInfo* pglDisplay::GetDisplayInfo(void)
{
    return displayInfo;
}

int pglDisplay::GetHeight(void) { return winHeight; }
int pglDisplay::GetWidth(void) { return winWidth; }
int pglDisplay::GetDepth(void) { return winBitDepth; }

pddiDisplayMode pglDisplay::GetDisplayMode(void)
{
    return mode;
}

int pglDisplay::GetNumColourBuffer(void) { return 2; }

unsigned pglDisplay::GetBufferMask(void)
{
    return displayInit.bufferMask;
}

unsigned pglDisplay::GetFreeTextureMem(void)
{
    return 0;
}

void pglDisplay::SwapBuffers(void)
{
    glKosSwapBuffers();
}

unsigned pglDisplay::Screenshot(pddiColour* buffer, int nBytes)
{
    (void)buffer;
    (void)nBytes;
    return 0;
}

void pglDisplay::BeginContext(void)
{
}

void pglDisplay::EndContext(void)
{
}

long pglDisplay::ProcessWindowMessage(void* wnd, const void* event)
{
    (void)wnd;
    (void)event;
    return 0;
}

void pglDisplay::SetWindow(void* wnd)
{
    (void)wnd;
}

void pglDisplay::BeginTiming()
{
}

float pglDisplay::EndTiming()
{
    return 0.0f;
}

void pglDisplay::SetGamma(float r, float g, float b)
{
    gammaR = r;
    gammaG = g;
    gammaB = b;
}

void pglDisplay::GetGamma(float* r, float* g, float* b)
{
    *r = gammaR;
    *g = gammaG;
    *b = gammaB;
}
