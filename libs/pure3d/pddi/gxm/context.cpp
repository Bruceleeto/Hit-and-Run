//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <pddi/gxm/gxm.hpp>
#include <pddi/gxm/context.hpp>
#include <pddi/gxm/device.hpp>
#include <pddi/gxm/display.hpp>
#include <pddi/gxm/texture.hpp>
#include <pddi/gxm/material.hpp>
#include <pddi/gxm/program.hpp>

#include <pddi/base/debug.hpp>
#include <p3d/utility.hpp>
#include <radmemory.hpp>

#include <math.h>
#include <string.h>
#include <SDL.h>
#include <vector>

// vertex arrays rendering
SceGxmPrimitiveType primTypeTable[5] =
{
    SCE_GXM_PRIMITIVE_TRIANGLES,      // PDDI_PRIM_TRIANGLES
    SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, // PDDI_PRIM_TRISTRIP
    SCE_GXM_PRIMITIVE_LINES,          // PDDI_PRIM_LINES
    SCE_GXM_PRIMITIVE_LINES,          // PDDI_PRIM_LINESTRIP
    SCE_GXM_PRIMITIVE_POINTS,         // PDDI_PRIM_POINTS
};

class gxmExtGamma : public pddiExtGammaControl
{
public:
    gxmExtGamma(gxmDisplay* d) { display = d;}

    void SetGamma(float r, float g, float b)     {display->SetGamma(r,g,b);}
    void GetGamma(float *r, float *g, float *b)  {display->GetGamma(r,g,b);}

protected:
    gxmDisplay* display;
};

void* gxmContext::patcherHostAlloc( void* userData, uint32_t size )
{
    return malloc( size );
}

void gxmContext::patcherHostFree( void* userData, void* mem )
{
    free( mem );
}

gxmContext::gxmContext(gxmDevice* dev, gxmDisplay* disp) : pddiBaseContext((pddiDisplay*)disp, (pddiDevice*)dev)
{
    device = dev;
    display = disp;
    context = display->GetGXMContext();
    vertexShader = nullptr;
    fragmentShader = nullptr;

    device->AddRef();
    display->AddRef();
    disp->SetContext(this);

    DefaultState();
    contextID = 0;

    extGamma = new gxmExtGamma(display);

    // set buffer sizes for this sample
    const uint32_t patcherBufferSize = 64 * 1024;
    const uint32_t patcherVertexUsseSize = 64 * 1024;
    const uint32_t patcherFragmentUsseSize = 64 * 1024;

    // allocate memory for buffers and USSE code
    radMemorySetAllocationName("patcherBuffer");
    patcherBuffer = radMemorySpaceAlloc(
        radMemorySpace_User,
        radMemoryGetCurrentAllocator(),
        patcherBufferSize);

    uint32_t patcherVertexUsseOffset;
    void* patcherVertexUsse = device->vertexUsseAlloc(
        patcherVertexUsseSize,
        &patcherVertexUsseUid,
        &patcherVertexUsseOffset);
    uint32_t patcherFragmentUsseOffset;
    void* patcherFragmentUsse = device->fragmentUsseAlloc(
        patcherFragmentUsseSize,
        &patcherFragmentUsseUid,
        &patcherFragmentUsseOffset);

    // create a shader patcher
    SceGxmShaderPatcherParams patcherParams;
    memset(&patcherParams, 0, sizeof(SceGxmShaderPatcherParams));
    patcherParams.userData = NULL;
    patcherParams.hostAllocCallback = &patcherHostAlloc;
    patcherParams.hostFreeCallback = &patcherHostFree;
    patcherParams.bufferAllocCallback = NULL;
    patcherParams.bufferFreeCallback = NULL;
    patcherParams.bufferMem = patcherBuffer;
    patcherParams.bufferMemSize = patcherBufferSize;
    patcherParams.vertexUsseAllocCallback = NULL;
    patcherParams.vertexUsseFreeCallback = NULL;
    patcherParams.vertexUsseMem = patcherVertexUsse;
    patcherParams.vertexUsseMemSize = patcherVertexUsseSize;
    patcherParams.vertexUsseOffset = patcherVertexUsseOffset;
    patcherParams.fragmentUsseAllocCallback = NULL;
    patcherParams.fragmentUsseFreeCallback = NULL;
    patcherParams.fragmentUsseMem = patcherFragmentUsse;
    patcherParams.fragmentUsseMemSize = patcherFragmentUsseSize;
    patcherParams.fragmentUsseOffset = patcherFragmentUsseOffset;
    CHK_GXM(sceGxmShaderPatcherCreate(&patcherParams, &shaderPatcher));

    // use embedded GXP files
    vertexProgram = new gxmProgram(shaderPatcher, p3d::openFile("app0:/shaders/vertex_cg.gxp", false));
    vertexProgram->AddRef();
    colorProgram = new gxmProgram(shaderPatcher, p3d::openFile("app0:/shaders/color_cg.gxp", false));
    colorProgram->AddRef();
    textureProgram = new gxmProgram(shaderPatcher, p3d::openFile("app0:/shaders/texture_cg.gxp", false));
    textureProgram->AddRef();
    alphaTestProgram = new gxmProgram(shaderPatcher, p3d::openFile("app0:/shaders/alpha_cg.gxp", false));
    alphaTestProgram->AddRef();

    radMemorySetAllocationName("dummyVector");
    dummyVector = (pddiVector4*)radMemorySpaceAlloc(
        radMemorySpace_User,
        radMemoryGetCurrentAllocator(),
        sizeof(pddiVector4));
    dummyVector->Set(1.0f, 1.0f, 1.0f);
    CHK_GXM(sceGxmSetVertexStream(context, 1, dummyVector));

    defaultShader = new gxmMat(this);
    defaultShader->AddRef();
}

gxmContext::~gxmContext()
{
    defaultShader->Release();
    vertexProgram->Release();
    colorProgram->Release();
    textureProgram->Release();
    alphaTestProgram->Release();

    delete extGamma;

    device->vertexUsseFree(patcherVertexUsseUid);
    device->fragmentUsseFree(patcherFragmentUsseUid);

    CHK_GXM(sceGxmShaderPatcherDestroy(shaderPatcher));
    radMemorySpaceFree(radMemorySpace_User, radMemoryGetCurrentAllocator(), patcherBuffer);

    CHK_GXM(sceGxmUnmapMemory(dummyVector));
    radMemorySpaceFree(radMemorySpace_User, radMemoryGetCurrentAllocator(), dummyVector);

    display->SetContext(NULL);
    display->Release();
    device->Release();
}

// frame synchronisation
void gxmContext::BeginFrame()
{
    pddiBaseContext::BeginFrame();

    sceGxmFinish(context);
    for(pddiPrimBuffer* buffer : streams)
        buffer->Release();
    streams.clear();
    for(SceGxmFragmentProgram* shader : shaders)
        CHK_GXM(sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, shader));
    shaders.clear();

    CHK_GXM(sceGxmBeginScene(
        context,
        0,
        display->GetRenderTarget(),
        NULL,
        NULL,
        display->GetFragementSyncObj(),
        display->GetColorSurface(),
        display->GetDepthSurface()
    ));

    if(display->HasReset())
    {
        contextID++;

        SyncState(0xffffffff);
    }

    projection.Identity();
}

void gxmContext::EndFrame()
{
    pddiBaseContext::EndFrame();

    CHK_GXM(sceGxmEndScene(context, NULL, NULL));
    CHK_GXM(sceGxmPadHeartbeat(display->GetColorSurface(), display->GetFragementSyncObj()));
}

// buffer clearing
void gxmContext::Clear(unsigned bufferMask)
{
    pddiBaseContext::Clear(bufferMask);

    PushState((pddiStateMask)(PDDI_STATE_RENDER | PDDI_STATE_VIEW));

    SetProjectionMode(PDDI_PROJECTION_ORTHOGRAPHIC);
    EnableZBuffer(false);
    if(bufferMask & PDDI_BUFFER_COLOUR)
        SetColourWrite(true, true, true, true);
    else
        SetColourWrite(false, false, false, false);
    SetZWrite(bufferMask & PDDI_BUFFER_DEPTH);

    PushIdentityMatrix(PDDI_MATRIX_MODELVIEW);
    pddiVector a, b, c, d;
    a.Set( -1.0f, -1.0f, state.viewState->clearDepth);
    b.Set(3.0f, -1.0f, state.viewState->clearDepth);
    c.Set(-1.0f, 3.0f, state.viewState->clearDepth);
    pddiPrimStream* stream = BeginPrims(defaultShader, PDDI_PRIM_TRIANGLES, PDDI_V_C, 3);
    stream->Vertex(&a, state.viewState->clearColour);
    stream->Vertex(&b, state.viewState->clearColour);
    stream->Vertex(&c, state.viewState->clearColour);
    EndPrims(stream);
    PopMatrix(PDDI_MATRIX_MODELVIEW);

    PopState((pddiStateMask)(PDDI_STATE_RENDER | PDDI_STATE_VIEW));
}

void gxmContext::SetupHardwareProjection(void)
{
    switch(state.viewState->projectionMode)
    {
        case PDDI_PROJECTION_DEVICE :
            projection.Identity();
            projection.SetOrthographic(0, display->GetWidth(),
                      display->GetHeight(), 0,
                      (state.viewState->camera.nearPlane),(state.viewState->camera.farPlane));
            sceGxmSetViewport(context, 0, display->GetWidth(), 0, display->GetHeight(),
                state.viewState->zRange[0], state.viewState->zRange[1] - state.viewState->zRange[0]);
            break;

        case PDDI_PROJECTION_ORTHOGRAPHIC :
            projection.Identity();
            projection.SetOrthographic(-0.5,  0.5,
                      -((1/state.viewState->camera.aspect)/2),  ((1/state.viewState->camera.aspect)/2),
                      (state.viewState->camera.nearPlane),(state.viewState->camera.farPlane));
            sceGxmSetViewport(context,
                              (state.viewState->viewWindow.left + 0.5f) * display->GetWidth(),
                              (state.viewState->viewWindow.right - state.viewState->viewWindow.left) * display->GetWidth() / 2.0f,
                              (state.viewState->viewWindow.top + 0.5f) * display->GetHeight(),
                              (state.viewState->viewWindow.top - state.viewState->viewWindow.bottom) * display->GetHeight() / 2.0f,
                              state.viewState->zRange[0], state.viewState->zRange[1] - state.viewState->zRange[0]);
            break;

        case PDDI_PROJECTION_PERSPECTIVE :
            projection.Identity();
            projection.SetPerspective(state.viewState->camera.fov,state.viewState->camera.aspect,state.viewState->camera.nearPlane,state.viewState->camera.farPlane);
            sceGxmSetViewport(context,
                            (state.viewState->viewWindow.left + 0.5f)* display->GetWidth(),
                            (state.viewState->viewWindow.right - state.viewState->viewWindow.left) * display->GetWidth() / 2.0f,
                            (state.viewState->viewWindow.top + 0.5f) * display->GetHeight(),
                            (state.viewState->viewWindow.top - state.viewState->viewWindow.bottom) * display->GetHeight() / 2.0f,
                            state.viewState->zRange[0], state.viewState->zRange[1] - state.viewState->zRange[0]);
            break;
        default:
            PDDIASSERTMSG(0, "Bad projection mode","");
            break;
    }
}

void gxmContext::LoadHardwareMatrix(pddiMatrixType id)
{
}

// viewport clipping
void gxmContext::SetScissor(pddiRect* rect)
{
    pddiBaseContext::SetScissor(rect);
    // TODO
}

class gxmPrimStream : public pddiPrimStream
{
public:
    pddiPrimBuffer* buffer;
    pddiPrimBufferStream* stream;

    void Coord(float x, float y, float z)  
    {
        stream->Coord(x, y, z);
    }

    void Normal(float x, float y, float z) 
    {
        stream->Normal(x, y, z);
    }

    void Colour(pddiColour colour, int channel = 0)
    {
        stream->Colour(colour, channel);
    }

    void UV(float u, float v, int channel = 0) 
    { 
        stream->UV(u, v, channel);
    }

    void Specular(pddiColour colour) 
    {
        stream->Specular(colour);
    }

    void Vertex(pddiVector* v, pddiColour c) 
    {
        stream->Vertex(v, c);
    }

    void Vertex(pddiVector* v, pddiVector* n)
    {
        stream->Vertex(v, n);
    }

    void Vertex(pddiVector* v, pddiVector2* uv)
    {
        stream->Vertex(v, uv);
    }

    void Vertex(pddiVector* v, pddiColour c, pddiVector2* uv)
    {
        stream->Vertex(v, c, uv);
    }

    void Vertex(pddiVector* v, pddiVector* n, pddiVector2* uv)
    {
        stream->Vertex(v, n, uv);
    }

} thePrimStream;

pddiPrimStream* gxmContext::BeginPrims(pddiShader* mat, pddiPrimType primType, unsigned vertexType, int vertexCount, unsigned pass)
{
    if(!mat)
        mat = defaultShader;
    PDDIASSERT(vertexCount);

    pddiBaseContext::BeginPrims(mat, primType, vertexType, vertexCount);

    pddiPrimBufferDesc desc(primType, vertexType, vertexCount);
    thePrimStream.buffer = device->NewPrimBuffer(&desc);
    thePrimStream.stream = thePrimStream.buffer->Lock();
    streams.push_back(thePrimStream.buffer);
    SetVertexShader(vertexType, ((gxmPrimBuffer*)thePrimStream.buffer)->GetStride());

    pddiBaseShader* material = (pddiBaseShader*)mat;
    ADD_STAT(PDDI_STAT_MATERIAL_OPS, !material->IsCurrent());
    material->SetMaterial();

    return &thePrimStream;
}

void gxmContext::EndPrims(pddiPrimStream* stream)
{
    pddiBaseContext::EndPrims(stream);
    gxmPrimStream* gxmstream = (gxmPrimStream*)stream;
    gxmstream->buffer->Unlock(thePrimStream.stream);
    ((gxmPrimBuffer*)gxmstream->buffer)->Display();
}

class gxmPrimBufferStream : public pddiPrimBufferStream
{
public:
    gxmPrimBuffer* buffer;

    gxmPrimBufferStream(gxmPrimBuffer* b)
    {
        buffer = b;
    }

    void Next(void)  
    {
        if(buffer->coord)
            buffer->coord = (float*)((char*)buffer->coord + buffer->stride);

        if(buffer->normal)
            buffer->normal = (float*)((char*)buffer->normal + buffer->stride);

        if(buffer->uv)
            buffer->uv = (float*)((char*)buffer->uv + buffer->stride);

        if(buffer->colour)
            buffer->colour += buffer->stride;

        buffer->total++;
        PDDIASSERT(buffer->total <= buffer->allocated);
    }

    void Position(float x, float y, float z)  
    { 
        buffer->coord[0] = x;
        buffer->coord[1] = y;
        buffer->coord[2] = z;
        Next();
    }

    void Normal(float x, float y, float z) 
    { 
        buffer->normal[0] = x;
        buffer->normal[1] = y;
        buffer->normal[2] = z;
    }

    void Colour(pddiColour colour, int channel = 0)         
    {
        // HBW: Multiple CBVs not yet implemented.  For now just ignore channel.
        buffer->colour[0] = colour.Red();
        buffer->colour[1] = colour.Green();
        buffer->colour[2] = colour.Blue();
        buffer->colour[3] = colour.Alpha();
    }

    void TexCoord1(float u, int channel = 0) {}

    void TexCoord2(float u, float v, int channel = 0) 
    { 
        if(channel == 0)
        {
            buffer->uv[0] = u;
            buffer->uv[1] = v;
        }
    }

    void TexCoord3(float u, float v, float s, int channel = 0) {}
    void TexCoord4(float u, float v, float s, float t, int channel = 0) {}

    void Specular(pddiColour colour) 
    {
        //
    }

    void SkinIndices(unsigned, unsigned, unsigned, unsigned)
    {
    }

    void SkinWeights(float, float, float)
    {
    }

    void Vertex(pddiVector* v, pddiColour c) 
    {
        buffer->colour[0] = c.Red();
        buffer->colour[1] = c.Green();
        buffer->colour[2] = c.Blue();
        buffer->colour[3] = c.Alpha();
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiVector* n)
    {
        buffer->normal[0] = n->x;
        buffer->normal[1] = n->y;
        buffer->normal[2] = n->z;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiVector2* uv)
    {
        buffer->uv[0] = uv->u;
        buffer->uv[1] = uv->v;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiColour c, pddiVector2* uv)
    {
        buffer->colour[0] = c.Red();
        buffer->colour[1] = c.Green();
        buffer->colour[2] = c.Blue();
        buffer->colour[3] = c.Alpha();
        buffer->uv[0] = uv->u;
        buffer->uv[1] = uv->v;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiVector* n, pddiVector2* uv)
    {
        buffer->normal[0] = n->x;
        buffer->normal[1] = n->y;
        buffer->normal[2] = n->z;
        buffer->uv[0] = uv->u;
        buffer->uv[1] = uv->v;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    bool CheckMemImageVersion(int version) { return false; }
    void* GetMemImagePtr()                 { return NULL; }
    unsigned GetMemImageLength()           { return 0; }

};

gxmPrimBuffer::gxmPrimBuffer(gxmContext* c, pddiPrimType type, unsigned vertexFormat, int nVertex, int nIndex) : context(c)
{
    stream = new gxmPrimBufferStream(this);

    total = allocated = stride = nStrips = 0;
    coord = normal = uv = NULL;
    colour = NULL;
    strips = NULL;
    indices = NULL;

    valid = false;
    buffer = 0;

    primType = type;
    vertexType = vertexFormat;

    allocated = nVertex;
    
    stride = 36;

    mem = stride * nVertex;
    PDDIASSERT(mem);
    radMemorySetAllocationName("gxmPrimBuffer");
    buffer = (uint8_t*)radMemorySpaceAlloc(
        radMemorySpace_User,
        radMemoryGetCurrentAllocator(),
        mem);

    unsigned char* ptr = buffer;
    coord = (float*)ptr;
    ptr += 12;
    
    if(vertexFormat & PDDI_V_NORMAL)
    {
        normal = (float*)ptr;
        ptr += 12;
    }
    
    if(vertexFormat & 0xf)
    {
        uv = (float*)ptr;
        ptr += 8;
    }
    
    if(vertexFormat & PDDI_V_COLOUR)
    {
        colour = ptr;
        ptr += 4;
    }

    indexCount = nIndex > 0 ? nIndex : nVertex;
    if(primType == PDDI_PRIM_LINESTRIP)
    {
        PDDIASSERT(!nIndex);
        indexCount *= 2;
    }

    PDDIASSERT(indexCount);
    radMemorySetAllocationName("gxmPrimBuffer");
    indices = (uint16_t*)radMemorySpaceAlloc(
        radMemorySpace_User,
        radMemoryGetCurrentAllocator(),
        indexCount * sizeof(uint16_t));

    if(!nIndex)
    {
        if(primType == PDDI_PRIM_LINESTRIP)
        {
            for(int i = 0; i < nVertex; i++)
            {
                indices[i * 2 + 0] = i;
                indices[i * 2 + 1] = i + 1;
            }
        }
        else
        {
            for(int i = 0; i < nVertex; i++)
                indices[i] = i;
        }
    }

    nStrips = 0;
    strips = NULL;

    context->ADD_STAT(PDDI_STAT_BUFFERED_COUNT, 1);
    context->ADD_STAT(PDDI_STAT_BUFFERED_ALLOC, mem / 1024.0f);
}

gxmPrimBuffer::~gxmPrimBuffer()
{
    delete stream;

    radMemorySpaceFree(radMemorySpace_User, radMemoryGetCurrentAllocator(), buffer);
    if(indices)
        radMemorySpaceFree(radMemorySpace_User, radMemoryGetCurrentAllocator(), indices);

    context->ADD_STAT(PDDI_STAT_BUFFERED_COUNT, -1);
    context->ADD_STAT(PDDI_STAT_BUFFERED_ALLOC, -mem / 1024.0f);
}

pddiPrimBufferStream* gxmPrimBuffer::Lock()
{
    total = 0;
    return stream;
}

void gxmPrimBuffer::Unlock(pddiPrimBufferStream* stream)
{
    if(coord)
        coord = (float*)((char*)coord - total * stride);

    if(normal)
        normal = (float*)((char*)normal - total * stride);

    if(uv)
        uv = (float*)((char*)uv - total * stride);

    if(colour)
        colour -= total * stride;

    valid = false;
}

unsigned char* gxmPrimBuffer::LockIndexBuffer()
{
    PDDIASSERT(0);
    return NULL;
}

void gxmPrimBuffer::UnlockIndexBuffer(int count)
{
    PDDIASSERT(0);
}

void gxmPrimBuffer::SetIndices(unsigned short* i, int count)
{
    PDDIASSERT(count <= (int)indexCount);
    memcpy(indices, i, count * sizeof(unsigned short));
    valid = false;
}

void gxmPrimBuffer::Display(void)
{
    if(!valid)
    {
        valid = true;
    }

    CHK_GXM(sceGxmSetVertexStream(context->context, 0, buffer));
    CHK_GXM(sceGxmDraw(context->context, primTypeTable[primType], SCE_GXM_INDEX_FORMAT_U16, indices, indexCount));
}

/*
protected:
    float* coord;
    float* normal;
    float* uv;
    unsigned char* colour;

    unsigned allocated;
    unsigned total;

};
*/

void gxmContext::DrawPrimBuffer(pddiShader* mat, pddiPrimBuffer* buffer)
{
    if(!mat)
        mat = defaultShader;

    pddiBaseShader* material = (pddiBaseShader*)mat;
    ADD_STAT(PDDI_STAT_MATERIAL_OPS, !material->IsCurrent());
    SetVertexShader(((gxmPrimBuffer*)buffer)->GetVertexFormat(), ((gxmPrimBuffer*)buffer)->GetStride());
    material->SetMaterial();

    ((gxmPrimBuffer*)buffer)->Display();
}

// lighting

int gxmContext::GetMaxLights(void)
{
    return PDDI_MAX_LIGHTS;
}

void gxmContext::SetupHardwareLight(int handle)
{
}

void gxmContext::SetAmbientLight(pddiColour col)
{
    pddiBaseContext::SetAmbientLight(col);
}

// backface culling
SceGxmCullMode cullModeTable[3] =
{
    SCE_GXM_CULL_NONE, // PDDI_CULL_NONE
    SCE_GXM_CULL_CW,   // PDDI_CULL_NORMAL
    SCE_GXM_CULL_CCW   // PDDI_CULL_INVERTED
};

void gxmContext::EnableCulling(bool enable)
{
    sceGxmSetCullMode(context, enable ? SCE_GXM_CULL_NONE : cullModeTable[state.renderState->cullMode]);
}

void gxmContext::SetCullMode(pddiCullMode mode)
{
    pddiBaseContext::SetCullMode(mode);
    sceGxmSetCullMode(context, cullModeTable[mode]);
}

// z-buffer control
SceGxmDepthFunc compTable[8] = {
    SCE_GXM_DEPTH_FUNC_NEVER,
    SCE_GXM_DEPTH_FUNC_ALWAYS,
    SCE_GXM_DEPTH_FUNC_LESS,
    SCE_GXM_DEPTH_FUNC_LESS_EQUAL,
    SCE_GXM_DEPTH_FUNC_GREATER,
    SCE_GXM_DEPTH_FUNC_GREATER_EQUAL,
    SCE_GXM_DEPTH_FUNC_EQUAL,
    SCE_GXM_DEPTH_FUNC_NOT_EQUAL,
};

void gxmContext::SetColourWrite(bool red, bool green, bool blue, bool alpha)
{
    pddiBaseContext::SetColourWrite(red, green, blue, alpha);
    // TODO
}

void gxmContext::EnableZBuffer(bool enable)
{
    pddiBaseContext::EnableZBuffer(enable);
    if(enable)
    {
        sceGxmSetFrontDepthFunc(context, compTable[state.renderState->zCompare]);
        sceGxmSetBackDepthWriteEnable(context, state.renderState->zWrite ?
            SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
    }
    else
    {
        sceGxmSetFrontDepthFunc(context, SCE_GXM_DEPTH_FUNC_ALWAYS);
        sceGxmSetBackDepthWriteEnable(context, SCE_GXM_DEPTH_WRITE_DISABLED);
    }
}


void gxmContext::SetZCompare(pddiCompareMode compareMode)
{
    pddiBaseContext::SetZCompare(compareMode);
    sceGxmSetFrontDepthFunc(context, compTable[compareMode]);
}

void gxmContext::SetZWrite(bool b)
{
    pddiBaseContext::SetZWrite(b);
    sceGxmSetBackDepthWriteEnable(context, b ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
}

void gxmContext::SetZBias(float bias)
{
    pddiBaseContext::SetZBias(bias);
//TODO : Figure out how ro do this
}

void gxmContext::SetZRange(float n, float f)
{
    pddiBaseContext::SetZRange(n,f);
    SetupHardwareProjection();
}

// stencil buffer control
SceGxmStencilFunc stencilTable[8] = {
    SCE_GXM_STENCIL_FUNC_NEVER,
    SCE_GXM_STENCIL_FUNC_ALWAYS,
    SCE_GXM_STENCIL_FUNC_LESS,
    SCE_GXM_STENCIL_FUNC_LESS_EQUAL,
    SCE_GXM_STENCIL_FUNC_GREATER,
    SCE_GXM_STENCIL_FUNC_GREATER_EQUAL,
    SCE_GXM_STENCIL_FUNC_EQUAL,
    SCE_GXM_STENCIL_FUNC_NOT_EQUAL,
};
SceGxmStencilOp stencilOpTable[6] = {
    SCE_GXM_STENCIL_OP_KEEP,
    SCE_GXM_STENCIL_OP_ZERO,
    SCE_GXM_STENCIL_OP_REPLACE,
    SCE_GXM_STENCIL_OP_INCR,
    SCE_GXM_STENCIL_OP_DECR,
    SCE_GXM_STENCIL_OP_INVERT
};

void gxmContext::EnableStencilBuffer(bool enable)
{
    pddiBaseContext::EnableStencilBuffer(enable);
    SetupHardwareStencil();
}

void gxmContext::SetupHardwareStencil(void)
{
    if(state.stencilState->enabled)
    {
        sceGxmSetFrontStencilFunc(context,
            stencilTable[state.stencilState->compare],
            stencilOpTable[state.stencilState->failOp],
            stencilOpTable[state.stencilState->zFailOp],
            stencilOpTable[state.stencilState->zPassOp],
            state.stencilState->mask,
            state.stencilState->writeMask);
    }
    else
    {
        sceGxmSetBackStencilFunc(context,
            SCE_GXM_STENCIL_FUNC_ALWAYS,
            SCE_GXM_STENCIL_OP_KEEP,
            SCE_GXM_STENCIL_OP_KEEP,
            SCE_GXM_STENCIL_OP_KEEP,
            0, 0);
    }
}
        
void gxmContext::SetStencilCompare(pddiCompareMode compare)
{
    pddiBaseContext::SetStencilCompare(compare);
    SetupHardwareStencil();
}

void gxmContext::SetStencilRef(int ref)
{
    pddiBaseContext::SetStencilRef(ref);
    sceGxmSetFrontStencilRef(context, ref);
}

void gxmContext::SetStencilMask(unsigned mask)
{
    pddiBaseContext::SetStencilMask(mask);
    SetupHardwareStencil();
}

void gxmContext::SetStencilWriteMask(unsigned mask)
{
    pddiBaseContext::SetStencilWriteMask(mask);
    SetupHardwareStencil();
}

void gxmContext::SetStencilOp(pddiStencilOp failOp, pddiStencilOp zFailOp, pddiStencilOp zPassOp)
{
    pddiBaseContext::SetStencilOp(failOp, zFailOp, zPassOp);
    SetupHardwareStencil();
}

void gxmContext::SetFillMode(pddiFillMode mode)
{
    pddiBaseContext::SetFillMode(mode);
}

// fog
void gxmContext::EnableFog(bool enable)
{
    pddiBaseContext::EnableFog(enable);
}

void gxmContext::SetFog(pddiColour colour, float start, float end)
{
    pddiBaseContext::SetFog(colour,start,end);

    float fog[4];
    fog[0] = float(colour.Red()) / 255;
    fog[1] = float(colour.Green()) / 255;
    fog[2] = float(colour.Blue()) / 255;
    fog[3] = float(colour.Alpha()) / 255;
}

int gxmContext::GetMaxTextureDimension(void)
{
    return 4096;
}

pddiExtension* gxmContext::GetExtension(unsigned extID)
{ 
    switch(extID)
    {
        case PDDI_EXT_GAMMACONTROL :
            return extGamma;
    }

    return pddiBaseContext::GetExtension(extID);
}

bool gxmContext::VerifyExtension(unsigned extID)
{ 
    switch(extID)
    {
        case PDDI_EXT_GAMMACONTROL :
            return true;
    }

    return pddiBaseContext::VerifyExtension(extID);
}

void  gxmContext::BeginTiming(void)
{
    display->BeginTiming();
}

float gxmContext::EndTiming(void)
{
    return display->EndTiming();
}

void gxmContext::SetVertexShader(unsigned int vertexType, uint16_t stride)
{
    SceGxmVertexProgram* vert = vertexCache[vertexType];
    if(!vert)
    {
        vert = vertexProgram->PatchVertexShader(vertexType, stride);
        vertexCache[vertexType] = vert;
    }

    if(vert != vertexShader)
    {
        vertexShader = vert;
        sceGxmSetVertexProgram(context, vert);
    }
}

void gxmContext::SetFragmentShader(SceGxmFragmentProgram* frag)
{
    if(frag == fragmentShader)
        return;

    fragmentShader = frag;
    sceGxmSetFragmentProgram(context, frag);
    if(!frag)
        return;

    shaders.push_back(fragmentShader);
    CHK_GXM(sceGxmShaderPatcherAddRefFragmentProgram(shaderPatcher, fragmentShader));
}

gxmProgram* gxmContext::GetFragmentProgram(const gxmTextureEnv* texEnv)
{
    if(texEnv->texture)
        return texEnv->alphaTest ? alphaTestProgram : textureProgram;
    else
        return colorProgram;
}

void gxmContext::SetTextureEnvironment(const gxmTextureEnv* texEnv, gxmProgram* fragProgram)
{
    CHK_GXM(sceGxmReserveFragmentDefaultUniformBuffer(context, &fragmentUniformBuffer));
    CHK_GXM(sceGxmReserveVertexDefaultUniformBuffer(context, &vertexUniformBuffer));
    for(int i = 0; i < PDDI_MAX_LIGHTS; i++)
        vertexProgram->SetLightState(vertexUniformBuffer, i, &state.lightingState->light[i]);
    vertexProgram->SetAmbientLight(vertexUniformBuffer, state.lightingState->ambient);
    vertexProgram->SetModelViewMatrix(vertexUniformBuffer, state.matrixStack[PDDI_MATRIX_MODELVIEW]->Top());
    vertexProgram->SetProjectionMatrix(vertexUniformBuffer, &projection);
    vertexProgram->SetTextureEnvironment(vertexUniformBuffer, texEnv);
    fragProgram->SetAlphaTest(fragmentUniformBuffer, texEnv);
}
