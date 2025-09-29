//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef _PROGRAM_HPP_
#define _PROGRAM_HPP_

#include <pddi/pddi.hpp>
#include <pddi/base/basecontext.hpp>
#include <pddi/gxm/gxm.hpp>
struct gxmTextureEnv;

class gxmProgram : public pddiObject
{
public:
    gxmProgram(const SceGxmProgram* prog);
    ~gxmProgram();

    const SceGxmProgram* GetProgram() { return program; }

    void SetProjectionMatrix( void* buffer, const pddiMatrix* matrix );
    void SetModelViewMatrix( void* buffer, const pddiMatrix* matrix );
    void SetTextureEnvironment( void* buffer, const gxmTextureEnv* texEnv );
    void SetLightState( void* buffer, int handle, const pddiLight* lightState );
    void SetAmbientLight( void* buffer, pddiColour ambient );

    inline bool SupportsLighting() { return acs != nullptr; }
    inline bool SupportsTextures() { return sampler != nullptr; }

protected:
    const SceGxmProgram* program;

    // Uniform locations
    const SceGxmProgramParameter* projection;
    const SceGxmProgramParameter* modelview;
    const SceGxmProgramParameter* normalmatrix;
    const SceGxmProgramParameter* alpharef;
    const SceGxmProgramParameter* sampler;
    struct {
        const SceGxmProgramParameter* enabled;
        const SceGxmProgramParameter* position;
        const SceGxmProgramParameter* colour;
        const SceGxmProgramParameter* attenuation;
    } lights[PDDI_MAX_LIGHTS];
    const SceGxmProgramParameter* acs;
    const SceGxmProgramParameter* acm;
    const SceGxmProgramParameter* dcm;
    const SceGxmProgramParameter* scm;
    const SceGxmProgramParameter* ecm;
    const SceGxmProgramParameter* srm;
};

#endif
