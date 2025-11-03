//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <pddi/gxm/gxm.hpp>
#include <pddi/gxm/device.hpp>
#include <pddi/gxm/display.hpp>
#include <pddi/gxm/context.hpp>
#include <pddi/gxm/texture.hpp>
#include <pddi/gxm/material.hpp>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <io.h>
#include <pddi/base/debug.hpp>
#include <SDL.h>

#define PDDI_GXM_BUILD 36

// Helper macro to align a value
#define ALIGN(x, a)					(((x) + ((a) - 1)) & ~((a) - 1))

static gxmDevice gblDevice;

char libName [] = "GXM";

int pddiCreate(int versionMajor, int versionMinor, pddiDevice** device)
{
    if((versionMajor != PDDI_VERSION_MAJOR) || (versionMinor != PDDI_VERSION_MINOR))
    {
        *device = NULL;
        return PDDI_VERSION_ERROR;
    }

    *device = &gblDevice;
    return PDDI_OK;
}

//--------------------------------------------------------------
gxmDevice::gxmDevice() 
{
    nDisplays = 0;
    displayInfo = NULL;
}

//--------------------------------------------------------------
gxmDevice::~gxmDevice()
{
    for( int i = 0; i < nDisplays; i++ )
    {
        delete[] displayInfo[i].modeInfo;
    }
    delete[] displayInfo;
    displayInfo = NULL;
}

//--------------------------------------------------------------
void gxmDevice::GetLibraryInfo(pddiLibInfo* info)
{
    info->versionMajor = PDDI_VERSION_MAJOR;
    info->versionMinor = PDDI_VERSION_MINOR;
    info->versionBuild = PDDI_GXM_BUILD;
    info->libID = PDDI_LIBID_VITA;
    strcpy( info->description, libName );
}

unsigned gxmDevice::GetCaps()
{
    return 0;
}

int gxmDevice::GetDisplayInfo(pddiDisplayInfo** info)
{
    *info = displayInfo;

    if (displayInfo)
    {
        return nDisplays;
    }

    int totalDisplay = SDL_GetNumVideoDisplays();
    displayInfo = new pddiDisplayInfo[totalDisplay];

    nDisplays = 0;
    for(int i = 0; i < totalDisplay; i++)
    {
        const char* displayName = SDL_GetDisplayName(i);
        int totalModes = SDL_GetNumDisplayModes(i);
        if (!displayName || totalModes <= 0)
            continue;

        displayInfo[nDisplays].id = i;
        strcpy(displayInfo[0].description,SDL_GetDisplayName(i));
        displayInfo[nDisplays].pci = 0;
        displayInfo[nDisplays].vendor = 0;
        displayInfo[nDisplays].fullscreenOnly = false;
        displayInfo[nDisplays].caps = 0;

        displayInfo[nDisplays].modeInfo = new pddiModeInfo[totalModes];
        displayInfo[nDisplays].nDisplayModes = gxmDisplay::FillDisplayModes(i, displayInfo[nDisplays].modeInfo);
        displayInfo[nDisplays].modeInfo = displayInfo[nDisplays].modeInfo;
        nDisplays++;
    }

    return nDisplays;
}

const char* gxmDevice::GetDeviceDescription()
{
    return libName;
}

void gxmDevice::SetCurrentContext(pddiRenderContext* c)
{
    context = c;
}

pddiRenderContext* gxmDevice::GetCurrentContext(void)
{
    return context;
}

pddiDisplay *gxmDevice::NewDisplay(int id)
{
    pddiDisplayInfo* dummy;
    GetDisplayInfo(&dummy);

    PDDIASSERT(id < nDisplays);
    gxmDisplay* display = new gxmDisplay(&displayInfo[id]);

    if(display->GetLastError() != PDDI_OK)
    {
        delete display;
        return NULL;
    }

    return (pddiDisplay *)display;
}
//--------------------------------------------------------------
pddiRenderContext *gxmDevice::NewRenderContext(pddiDisplay* display)
{
    gxmContext* context = new gxmContext(this, (gxmDisplay*)display);

    if(context->GetLastError() != PDDI_OK)
    {
        delete context;
        return NULL;
    }
    return context;
}

//--------------------------------------------------------------
pddiTexture* gxmDevice::NewTexture(pddiTextureDesc* desc)
{
    gxmTexture* tex = new gxmTexture((gxmContext*)context);
    if(!tex->Create(desc->GetSizeX(), desc->GetSizeY(), desc->GetBitDepth(), 
                     desc->GetAlphaDepth(), desc->GetMipMapCount(),desc->GetType(),desc->GetUsage()))
    {
        lastError = tex->GetLastError();
        delete tex;
        return NULL;
    }
    return tex;
}
//--------------------------------------------------------------
pddiShader *gxmDevice::NewShader(const char* name, const char*) 
{ 
    gxmMat* mat= new gxmMat((gxmContext*)context);
    if(mat->GetLastError() != PDDI_OK) {
        delete mat;
        return NULL;
    }

    return mat;
}

pddiPrimBuffer *gxmDevice::NewPrimBuffer(pddiPrimBufferDesc* desc) 
{ 
    return new gxmPrimBuffer((gxmContext*)context, desc->GetPrimType(), desc->GetVertexFormat(), desc->GetVertexCount(), desc->GetIndexCount());;
}

void gxmDevice::AddCustomShader(const char* name, const char* aux)
{
}

void gxmDevice::Release(void)
{
}
