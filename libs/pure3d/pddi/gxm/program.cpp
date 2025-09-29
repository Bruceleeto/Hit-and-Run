//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <pddi/gxm/gxm.hpp>
#include <pddi/gxm/program.hpp>
#include <pddi/gxm/material.hpp>

#include <string>
#include <vector>
#include <SDL.h>

static inline void UniformColour(void* buffer, const SceGxmProgramParameter* param, pddiColour c)
{
    float colour[] = { float(c.Red()) / 255, float(c.Green()) / 255, float(c.Blue()) / 255, float(c.Alpha()) / 255 };
    sceGxmSetUniformDataF(buffer, param, 0, 4, colour);
}

gxmProgram::gxmProgram(const SceGxmProgram* prog)
    : program(prog)
{
    projection = sceGxmProgramFindParameterByName(program, "projection");
    modelview = sceGxmProgramFindParameterByName(program, "modelview");
    normalmatrix = sceGxmProgramFindParameterByName(program, "normalmatrix");
    alpharef = sceGxmProgramFindParameterByName(program, "alpharef");
    sampler = sceGxmProgramFindParameterByName(program, "tex");

    for(int i = 0; i < PDDI_MAX_LIGHTS; i++)
    {
        std::string prefix = std::string("lights[") + char('0' + i) + "].";
        lights[i].enabled = sceGxmProgramFindParameterByName(program, (prefix + "enabled").c_str());
        lights[i].position = sceGxmProgramFindParameterByName(program, (prefix + "position").c_str());
        lights[i].colour = sceGxmProgramFindParameterByName(program, (prefix + "colour").c_str());
        lights[i].attenuation = sceGxmProgramFindParameterByName(program, (prefix + "attenuation").c_str());
    }

    acs = sceGxmProgramFindParameterByName(program, "acs");
    acm = sceGxmProgramFindParameterByName(program, "acm");
    dcm = sceGxmProgramFindParameterByName(program, "dcm");
    scm = sceGxmProgramFindParameterByName(program, "scm");
    ecm = sceGxmProgramFindParameterByName(program, "ecm");
    srm = sceGxmProgramFindParameterByName(program, "srm");
}

gxmProgram::~gxmProgram()
{
}

void gxmProgram::SetProjectionMatrix(void* buffer, const pddiMatrix* matrix)
{
    if(projection)
        sceGxmSetUniformDataF(buffer, projection, 0, 16, matrix->m[0]);
}

void gxmProgram::SetModelViewMatrix(void* buffer, const pddiMatrix* matrix)
{
    if(modelview >= 0)
        sceGxmSetUniformDataF(buffer, modelview, 0, 16, matrix->m[0]);
    if(normalmatrix >= 0)
    {
        pddiMatrix inverse;
        inverse.Invert(*matrix);
        inverse.Transpose();
        sceGxmSetUniformDataF(buffer, normalmatrix, 0, 16, inverse.m[0]);
    }
}

void gxmProgram::SetTextureEnvironment(void* buffer, const gxmTextureEnv* texEnv)
{
    if(texEnv->lit)
    {
        UniformColour(buffer, acm, texEnv->ambient);
        UniformColour(buffer, ecm, texEnv->emissive);
        UniformColour(buffer, dcm, texEnv->diffuse);
        UniformColour(buffer, scm, texEnv->specular);
        sceGxmSetUniformDataF(buffer, srm, 0, 1, &texEnv->shininess);
    }
    else
    {
        UniformColour(buffer, acm, pddiColour(-1));
        UniformColour(buffer, ecm, pddiColour(-1));
        UniformColour(buffer, dcm, pddiColour(-1));
        UniformColour(buffer, scm, pddiColour(-1));
        float shininess = 0.0f;
        sceGxmSetUniformDataF(buffer, srm, 0, 1, &texEnv->shininess);
    }

    if(texEnv->alphaTest && alpharef >= 0)
    {
        PDDIASSERT(texEnv->alphaCompareMode == PDDI_COMPARE_GREATER ||
            texEnv->alphaCompareMode == PDDI_COMPARE_GREATEREQUAL);
        float ref = texEnv->alphaTest ? texEnv->alphaRef : 0.0f;
        sceGxmSetUniformDataF(buffer, alpharef, 0, 1, &ref);
    }
}

void gxmProgram::SetLightState( void* buffer, int handle, const pddiLight* lightState )
{
    if(handle >= PDDI_MAX_LIGHTS)
        return;

    float dir[4];
    switch(lightState->type)
    {
        case PDDI_LIGHT_DIRECTIONAL :
            dir[0] = -lightState->worldDirection.x;
            dir[1] = -lightState->worldDirection.y;
            dir[2] = -lightState->worldDirection.z;
            dir[3] = 0.0f;
            break;

        case PDDI_LIGHT_POINT :
            dir[0] = lightState->worldPosition.x;
            dir[1] = lightState->worldPosition.y;
            dir[2] = lightState->worldPosition.z;
            dir[3] = 1.0f;
            break;

        case PDDI_LIGHT_SPOT :
            PDDIASSERT(0);
            break;
    }

    int enabled = lightState->enabled ? 1 : 0;
    sceGxmSetUniformDataF(buffer, lights[handle].enabled, 0, 1, (float*)&enabled);
    sceGxmSetUniformDataF(buffer, lights[handle].position, 0, 4, dir);
    UniformColour(buffer, lights[handle].colour, lightState->colour);
    float attenuation[] = { lightState->attenA, lightState->attenB, lightState->attenC };
    sceGxmSetUniformDataF(buffer, lights[handle].attenuation, 0, 3, attenuation);
}

void gxmProgram::SetAmbientLight(void* buffer, pddiColour ambient)
{
    if(acs)
        UniformColour(buffer, acs, ambient);
}
