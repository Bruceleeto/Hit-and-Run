//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


// stub GXM header, all pddi gxm code uses this instead of '#include <psp2/gxm.h>

#include <psp2/gxm.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/sysmem.h>
#include <pddi/base/debug.hpp>

#ifdef PDDI_USE_ASSERTS
#define CHK_GXM(x) PDDIASSERTNOBRK((x) == SCE_OK)
#else
#define CHK_GXM(x) ((x) == SCE_OK)
#endif
