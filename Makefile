# Simpsons Hit & Run - Linux Makefile
# Desktop OpenGL, 32-bit, NTSC
#
# Usage:
#   make -j$(nproc)
#   make AUDIO=0 -j$(nproc)   # build without OpenAL audio
#   make clean

# Set to 0 to build without OpenAL audio support
AUDIO ?= 1

ROOT     := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILDDIR := $(ROOT)build-mk

CXX := g++
CC  := gcc
AR  := ar rcs

PKG_CONFIG := PKG_CONFIG_LIBDIR=/usr/lib/pkgconfig pkg-config

CFLAGS_COMMON := -m32 -O2 -g \
	-fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks \
	-fvisibility=hidden \
	-DRAD_CONSOLE -DRAD_WIN32 -DRAD_SDL -DRAD_RELEASE

CXXFLAGS := $(CFLAGS_COMMON) -std=gnu++17
CFLAGS   := $(CFLAGS_COMMON) -std=gnu11

LDFLAGS := -m32 -Wl,--allow-shlib-undefined -Wl,--as-needed -Wl,-z,lazy

SDL2_CFLAGS  := $(shell $(PKG_CONFIG) --cflags sdl2)
SDL2_LIBS    := $(shell $(PKG_CONFIG) --libs sdl2)
PNG_CFLAGS   := $(shell $(PKG_CONFIG) --cflags libpng)
PNG_LIBS     := $(shell $(PKG_CONFIG) --libs libpng)

ifeq ($(AUDIO),1)
OPENAL_CFLAGS := $(shell $(PKG_CONFIG) --cflags openal 2>/dev/null || echo -I/usr/include/AL) -include al.h -include alc.h -include efx.h
OPENAL_LIBS   := $(shell $(PKG_CONFIG) --libs openal 2>/dev/null || echo -lopenal)
else
OPENAL_CFLAGS :=
OPENAL_LIBS   :=
CFLAGS_COMMON += -DRAD_NO_AUDIO
CXXFLAGS := $(CFLAGS_COMMON) -std=gnu++17
CFLAGS   := $(CFLAGS_COMMON) -std=gnu11
endif

LIBS := $(OPENAL_LIBS) $(SDL2_LIBS) $(PNG_LIBS) -lpthread

I_RADCORE    := -I$(ROOT)libs/radcore/inc -I$(ROOT)libs/radcore/src/pch -I$(ROOT)code
I_RADMATH    := -I$(ROOT)libs/radmath
I_P3D        := -I$(ROOT)libs/pure3d -I$(ROOT)libs
I_PDDI       := -I$(ROOT)libs/pure3d -I$(ROOT)libs/pure3d/pddi/gl/glad/include
I_RADCONTENT := -I$(ROOT)libs/radcontent/inc -I$(ROOT)libs/radcontent/src
I_RADSCRIPT  := -I$(ROOT)libs/radscript/inc -I$(ROOT)libs/radscript/src/pch
I_RADSOUND   := -I$(ROOT)libs/radsound/inc -I$(ROOT)libs/radsound/src/pch -I$(ROOT)libs/radsound/src/common
I_RADMUSIC   := -I$(ROOT)libs/radmusic/inc -I$(ROOT)libs/radmusic/src/pch -I$(ROOT)libs/radmusic/src
I_RADMOVIE   := -I$(ROOT)libs/radmovie/inc -I$(ROOT)libs/radmovie/src/pch
I_POSER      := -I$(ROOT)libs/poser/inc
I_SCROOBY    := -I$(ROOT)libs/scrooby/inc -I$(ROOT)libs/scrooby/src
I_SIM        := -I$(ROOT)libs/sim
I_CHOREO     := -I$(ROOT)libs/choreo/inc

I_ALL := $(I_RADCORE) $(I_RADMATH) $(I_P3D) $(I_PDDI) $(I_RADCONTENT) $(I_RADSCRIPT) \
         $(I_RADSOUND) $(I_RADMUSIC) $(I_RADMOVIE) $(I_POSER) $(I_SCROOBY) $(I_SIM) $(I_CHOREO) \
         $(SDL2_CFLAGS) $(PNG_CFLAGS) $(OPENAL_CFLAGS)

# =============================================================================
# Source files
# =============================================================================

RADMATH_SRC := \
	libs/radmath/radmath/matrix.cpp \
	libs/radmath/radmath/numerical.cpp \
	libs/radmath/radmath/plane.cpp \
	libs/radmath/radmath/quaternion.cpp \
	libs/radmath/radmath/radmath.cpp \
	libs/radmath/radmath/random.cpp \
	libs/radmath/radmath/spline.cpp \
	libs/radmath/radmath/triangle.cpp \
	libs/radmath/radmath/trig.cpp \
	libs/radmath/radmath/vector.cpp

RADCORE_SRC := \
	libs/radcore/src/radcontroller/sdlcontroller.cpp \
	libs/radcore/src/raddebugcommunication/targetx.cpp \
	libs/radcore/src/raddebugconsole/consoleclient.cpp \
	libs/radcore/src/raddebug/debug.cpp \
	libs/radcore/src/raddispatch/dispatcher.cpp \
	libs/radcore/src/radfile/common/cementer.cpp \
	libs/radcore/src/radfile/common/drive.cpp \
	libs/radcore/src/radfile/common/drivethread.cpp \
	libs/radcore/src/radfile/common/filecache.cpp \
	libs/radcore/src/radfile/common/file.cpp \
	libs/radcore/src/radfile/common/filesystem.cpp \
	libs/radcore/src/radfile/common/instancedrive.cpp \
	libs/radcore/src/radfile/common/platformdrives.cpp \
	libs/radcore/src/radfile/common/radfile.cpp \
	libs/radcore/src/radfile/common/remotedrive.cpp \
	libs/radcore/src/radfile/common/requests.cpp \
	libs/radcore/src/radfile/sdl/sdldrive.cpp \
	libs/radcore/src/radkey/radkey.cpp \
	libs/radcore/src/radmemory/align.cpp \
	libs/radcore/src/radmemory/binallocator.cpp \
	libs/radcore/src/radmemory/dlheap.cpp \
	libs/radcore/src/radmemory/externalmemoryheap.cpp \
	libs/radcore/src/radmemory/externalmemoryobject.cpp \
	libs/radcore/src/radmemory/memorymanager.cpp \
	libs/radcore/src/radmemory/memoryobject.cpp \
	libs/radcore/src/radmemory/memorypool.cpp \
	libs/radcore/src/radmemory/memoryspacewin32.cpp \
	libs/radcore/src/radmemorymonitor/memmonitorclient.cpp \
	libs/radcore/src/radmemorymonitor/memmonitorclienthelp.cpp \
	libs/radcore/src/radmemory/platalloc.cpp \
	libs/radcore/src/radmemory/staticheap.cpp \
	libs/radcore/src/radmemory/trackingheap.cpp \
	libs/radcore/src/radmemory/twowayallocator.cpp \
	libs/radcore/src/radobjectbtree/objectavltree.cpp \
	libs/radcore/src/radobjectbtree/objectbtree.cpp \
	libs/radcore/src/radobjectlist/dynamicarray.cpp \
	libs/radcore/src/radobjectlist/objectlist.cpp \
	libs/radcore/src/radobjectlist/weakcallbackwrapper.cpp \
	libs/radcore/src/radobjectlist/weakinterfacewrapper.cpp \
	libs/radcore/src/radobject/object.cpp \
	libs/radcore/src/radplatform/platform.cpp \
	libs/radcore/src/radprofiler/profiler2.cpp \
	libs/radcore/src/radprofiler/profiler.cpp \
	libs/radcore/src/radprofiler/profilesample.cpp \
	libs/radcore/src/radremotecommand/functionlist.cpp \
	libs/radcore/src/radremotecommand/remotecommandserver.cpp \
	libs/radcore/src/radremotecommand/remotecommandtarget.cpp \
	libs/radcore/src/radstacktrace/win32/stacktrace.cpp \
	libs/radcore/src/radstats/simplestat.cpp \
	libs/radcore/src/radstats/statmanager.cpp \
	libs/radcore/src/radstring/string.cpp \
	libs/radcore/src/radtextdisplay/textdisplay.cpp \
	libs/radcore/src/radthread/mutex.cpp \
	libs/radcore/src/radthread/semaphore.cpp \
	libs/radcore/src/radthread/system.cpp \
	libs/radcore/src/radthread/thread.cpp \
	libs/radcore/src/radtime/stopwatch.cpp \
	libs/radcore/src/radtime/time.cpp

PDDI_SRC := \
	libs/pure3d/pddi/base/basecontext.cpp \
	libs/pure3d/pddi/base/baseshader.cpp \
	libs/pure3d/pddi/base/font.cpp \
	libs/pure3d/pddi/base/pddiobj.cpp \
	libs/pure3d/pddi/gl/glcon.cpp \
	libs/pure3d/pddi/gl/gldev.cpp \
	libs/pure3d/pddi/gl/glmat.cpp \
	libs/pure3d/pddi/gl/gltex.cpp \
	libs/pure3d/pddi/gl/display_win32/gldisplay.cpp

PDDI_C_SRC := \
	libs/pure3d/pddi/gl/glad/src/glad.c

P3D_SRC := \
	libs/pure3d/p3d/ambientlight.cpp \
	libs/pure3d/p3d/anim/animate.cpp \
	libs/pure3d/p3d/anim/animatedobject.cpp \
	libs/pure3d/p3d/anim/billboardobjectanimation.cpp \
	libs/pure3d/p3d/anim/cameraanimation.cpp \
	libs/pure3d/p3d/anim/channel.cpp \
	libs/pure3d/p3d/anim/compositedrawable.cpp \
	libs/pure3d/p3d/anim/drawablepose.cpp \
	libs/pure3d/p3d/anim/eventanimation.cpp \
	libs/pure3d/p3d/anim/event.cpp \
	libs/pure3d/p3d/anim/expressionanimation.cpp \
	libs/pure3d/p3d/anim/expression.cpp \
	libs/pure3d/p3d/anim/expressionoffsets.cpp \
	libs/pure3d/p3d/anim/instancedgeometry.cpp \
	libs/pure3d/p3d/anim/lightanimation.cpp \
	libs/pure3d/p3d/anim/multicontroller.cpp \
	libs/pure3d/p3d/anim/polyskin.cpp \
	libs/pure3d/p3d/anim/poseanimation.cpp \
	libs/pure3d/p3d/anim/pose.cpp \
	libs/pure3d/p3d/anim/sequencer.cpp \
	libs/pure3d/p3d/anim/shaderanimation.cpp \
	libs/pure3d/p3d/anim/skeleton.cpp \
	libs/pure3d/p3d/anim/textureanimation.cpp \
	libs/pure3d/p3d/anim/vertexanimcontroller.cpp \
	libs/pure3d/p3d/anim/vertexanimkey.cpp \
	libs/pure3d/p3d/anim/vertexanimobject.cpp \
	libs/pure3d/p3d/anim/vertexoffsetexpression.cpp \
	libs/pure3d/p3d/anim/visibilityanimation.cpp \
	libs/pure3d/p3d/billboardobject.cpp \
	libs/pure3d/p3d/bmp.cpp \
	libs/pure3d/p3d/camera.cpp \
	libs/pure3d/p3d/cameraloader.cpp \
	libs/pure3d/p3d/chunkfile.cpp \
	libs/pure3d/p3d/context.cpp \
	libs/pure3d/p3d/debugdraw.cpp \
	libs/pure3d/p3d/directionallight.cpp \
	libs/pure3d/p3d/displaylist.cpp \
	libs/pure3d/p3d/drawable.cpp \
	libs/pure3d/p3d/dxtn.cpp \
	libs/pure3d/p3d/effects/effect.cpp \
	libs/pure3d/p3d/effects/optic.cpp \
	libs/pure3d/p3d/effects/opticlensflare.cpp \
	libs/pure3d/p3d/effects/opticloader.cpp \
	libs/pure3d/p3d/effects/particlearray.cpp \
	libs/pure3d/p3d/effects/particleemitter.cpp \
	libs/pure3d/p3d/effects/particlegenerator.cpp \
	libs/pure3d/p3d/effects/particleloader.cpp \
	libs/pure3d/p3d/effects/particlepool.cpp \
	libs/pure3d/p3d/effects/particlesystem.cpp \
	libs/pure3d/p3d/effects/particletype.cpp \
	libs/pure3d/p3d/effects/particleutility.cpp \
	libs/pure3d/p3d/effects/transitionmanager.cpp \
	libs/pure3d/p3d/entity.cpp \
	libs/pure3d/p3d/error.cpp \
	libs/pure3d/p3d/file.cpp \
	libs/pure3d/p3d/fileftt.cpp \
	libs/pure3d/p3d/font.cpp \
	libs/pure3d/p3d/gameattr.cpp \
	libs/pure3d/p3d/geometry.cpp \
	libs/pure3d/p3d/imageconverter.cpp \
	libs/pure3d/p3d/image.cpp \
	libs/pure3d/p3d/imagefactory.cpp \
	libs/pure3d/p3d/imagefont.cpp \
	libs/pure3d/p3d/inventory.cpp \
	libs/pure3d/p3d/light.cpp \
	libs/pure3d/p3d/lightloader.cpp \
	libs/pure3d/p3d/lightschooser.cpp \
	libs/pure3d/p3d/loaders.cpp \
	libs/pure3d/p3d/loadmanager.cpp \
	libs/pure3d/p3d/locator.cpp \
	libs/pure3d/p3d/lzr.cpp \
	libs/pure3d/p3d/matrixstack.cpp \
	libs/pure3d/p3d/memheap.cpp \
	libs/pure3d/p3d/memory.cpp \
	libs/pure3d/p3d/memorysection.cpp \
	libs/pure3d/p3d/platform/win32/platform.cpp \
	libs/pure3d/p3d/png.cpp \
	libs/pure3d/p3d/pointcamera.cpp \
	libs/pure3d/p3d/pointlight.cpp \
	libs/pure3d/p3d/primgroup.cpp \
	libs/pure3d/p3d/rawimage.cpp \
	libs/pure3d/p3d/refcounted.cpp \
	libs/pure3d/p3d/scenegraph/nodeanimation.cpp \
	libs/pure3d/p3d/scenegraph/scenegraph.cpp \
	libs/pure3d/p3d/shader.cpp \
	libs/pure3d/p3d/shadow/shadow_common.cpp \
	libs/pure3d/p3d/shadow/shadow_gc.cpp \
	libs/pure3d/p3d/shadow/shadow_generic.cpp \
	libs/pure3d/p3d/shadow/shadow_ps2.cpp \
	libs/pure3d/p3d/spotlight.cpp \
	libs/pure3d/p3d/sprite.cpp \
	libs/pure3d/p3d/table.cpp \
	libs/pure3d/p3d/targa.cpp \
	libs/pure3d/p3d/textdataparser.cpp \
	libs/pure3d/p3d/textstring.cpp \
	libs/pure3d/p3d/texture.cpp \
	libs/pure3d/p3d/texturefont.cpp \
	libs/pure3d/p3d/unicode.cpp \
	libs/pure3d/p3d/utility.cpp \
	libs/pure3d/p3d/vectorcamera.cpp \
	libs/pure3d/p3d/vertexlist.cpp \
	libs/pure3d/p3d/view.cpp

RADCONTENT_SRC := \
	libs/radcontent/src/radload/request.cpp \
	libs/radcontent/src/radload/queue.cpp \
	libs/radcontent/src/radload/hashtable.cpp \
	libs/radcontent/src/radload/inventory.cpp \
	libs/radcontent/src/radload/stream.cpp \
	libs/radcontent/src/radload/manager.cpp \
	libs/radcontent/src/radload/object.cpp

RADSCRIPT_SRC := \
	libs/radscript/src/factory/radfactory.cpp \
	libs/radscript/src/namespace/namespace.cpp \
	libs/radscript/src/pch/pch.cpp \
	libs/radscript/src/script/remotescript.cpp \
	libs/radscript/src/script/script.cpp \
	libs/radscript/src/typeinfo/TypeInfoEnum.cpp \
	libs/radscript/src/typeinfo/TypeInfoInstance.cpp \
	libs/radscript/src/typeinfo/TypeInfoInterface.cpp \
	libs/radscript/src/typeinfo/TypeInfoIntLiteral.cpp \
	libs/radscript/src/typeinfo/TypeInfoLoader.cpp \
	libs/radscript/src/typeinfo/TypeInfoMethod.cpp \
	libs/radscript/src/typeinfo/TypeInfoParam.cpp \
	libs/radscript/src/typeinfo/TypeInfoSystem.cpp \
	libs/radscript/src/typeinfoutil/typeinfodistributor.cpp \
	libs/radscript/src/typeinfoutil/typeinfoutil.cpp \
	libs/radscript/src/typeinfo/win32/win32typeinfovfcall.cpp

ifeq ($(AUDIO),1)
RADSOUND_SRC := \
	libs/radsound/src/common/radsoundobject.cpp \
	libs/radsound/src/common/radsoundupdatableobject.cpp \
	libs/radsound/src/hal/common/audioformat.cpp \
	libs/radsound/src/hal/common/banner.cpp \
	libs/radsound/src/hal/common/memoryregion.cpp \
	libs/radsound/src/hal/common/radsoundfile.cpp \
	libs/radsound/src/hal/common/rolloff.cpp \
	libs/radsound/src/hal/win32/buffer.cpp \
	libs/radsound/src/hal/win32/bufferloader.cpp \
	libs/radsound/src/hal/win32/effect.cpp \
	libs/radsound/src/hal/win32/listener.cpp \
	libs/radsound/src/hal/win32/positionalgroup.cpp \
	libs/radsound/src/hal/win32/radsoundwin.cpp \
	libs/radsound/src/hal/win32/system.cpp \
	libs/radsound/src/hal/win32/voice.cpp \
	libs/radsound/src/math/radsoundmath.cpp \
	libs/radsound/src/pch/pch.cpp \
	libs/radsound/src/radsound/buffereddatasource.cpp \
	libs/radsound/src/radsound/clip.cpp \
	libs/radsound/src/radsound/clipplayer.cpp \
	libs/radsound/src/radsound/datacache.cpp \
	libs/radsound/src/radsound/memorydatasource.cpp \
	libs/radsound/src/radsound/memoryspaceobject.cpp \
	libs/radsound/src/radsound/radicaladpcm.cpp \
	libs/radsound/src/radsound/rsdfiledatasource.cpp \
	libs/radsound/src/radsound/stitcheddatasource.cpp \
	libs/radsound/src/radsound/streamplayer.cpp

RADMUSIC_SRC := \
	libs/radmusic/src/memory/memory.cpp \
	libs/radmusic/src/ods/ods_codegen.cpp \
	libs/radmusic/src/ods/ods.cpp \
	libs/radmusic/src/ods/ods_memory.cpp \
	libs/radmusic/src/ods/ods_parser.cpp \
	libs/radmusic/src/ods/ods_util.cpp \
	libs/radmusic/src/pch/pch.cpp \
	libs/radmusic/src/radmusic/framework/framework.cpp \
	libs/radmusic/src/radmusic/music/music_composition.cpp \
	libs/radmusic/src/radmusic/music/music_engine.cpp \
	libs/radmusic/src/radmusic/music/music_engine_state_fade.cpp \
	libs/radmusic/src/radmusic/music/music_engine_state_steady.cpp \
	libs/radmusic/src/radmusic/music/music_engine_state_stitch.cpp \
	libs/radmusic/src/radmusic/music/music_engine_state_stopped.cpp \
	libs/radmusic/src/radmusic/music/music_performance.cpp \
	libs/radmusic/src/radmusic/resource/resource_manager.cpp \
	libs/radmusic/src/radmusic/schema/schema_util.cpp \
	libs/radmusic/src/radmusic/sequence/sequence_player.cpp \
	libs/radmusic/src/radmusic/sequence/sequence_region.cpp \
	libs/radmusic/src/radmusic/sequence/sequence_stream_graph.cpp

RADMOVIE_SRC := \
	libs/radmovie/src/common/ffmpegmovieplayer.cpp \
	libs/radmovie/src/common/binkrenderstrategy.cpp \
	libs/radmovie/src/pch/pch.cpp
else
RADSOUND_SRC :=
RADMUSIC_SRC :=
RADMOVIE_SRC :=
endif

POSER_SRC := \
	libs/poser/src/joint.cpp \
	libs/poser/src/pose.cpp \
	libs/poser/src/posedriver.cpp \
	libs/poser/src/poseengine.cpp \
	libs/poser/src/transform.cpp

SCROOBY_SRC := \
	libs/scrooby/src/FeApp.cpp \
	libs/scrooby/src/FeBoundedDrawable.cpp \
	libs/scrooby/src/FeChunkHandler.cpp \
	libs/scrooby/src/FeDrawable.cpp \
	libs/scrooby/src/FeEntity.cpp \
	libs/scrooby/src/FeGroup.cpp \
	libs/scrooby/src/FeLanguage.cpp \
	libs/scrooby/src/FeLayer.cpp \
	libs/scrooby/src/FeLoaders.cpp \
	libs/scrooby/src/FeMovie.cpp \
	libs/scrooby/src/FeOwner.cpp \
	libs/scrooby/src/FePage.cpp \
	libs/scrooby/src/FeParent.cpp \
	libs/scrooby/src/FePolygon.cpp \
	libs/scrooby/src/FeProject.cpp \
	libs/scrooby/src/FePure3dObject.cpp \
	libs/scrooby/src/FeScreen.cpp \
	libs/scrooby/src/FeSprite.cpp \
	libs/scrooby/src/FeTextBible.cpp \
	libs/scrooby/src/FeText.cpp \
	libs/scrooby/src/FeTextStyle.cpp \
	libs/scrooby/src/ResourceManager/FeResourceManager.cpp \
	libs/scrooby/src/scrooby/App.cpp \
	libs/scrooby/src/strings/pcstring.cpp \
	libs/scrooby/src/strings/stricmp.cpp \
	libs/scrooby/src/strings/UnicodeString.cpp \
	libs/scrooby/src/tLinearTable.cpp \
	libs/scrooby/src/utility/AsyncFileLoader.cpp \
	libs/scrooby/src/utility/BufferReader.cpp \
	libs/scrooby/src/utility/debugMessages.cpp \
	libs/scrooby/src/utility/EnumConversion.cpp \
	libs/scrooby/src/xml/XMLParser.cpp \
	libs/scrooby/src/xml/XMLTree.cpp

SIM_SRC := \
	libs/sim/simcollision/collisionanalyser.cpp \
	libs/sim/simcollision/collisionanalyserdata.cpp \
	libs/sim/simcollision/collisionanalyserdataUID.cpp \
	libs/sim/simcollision/collisionanalyserevent.cpp \
	libs/sim/simcollision/collisionanalyserinfo.cpp \
	libs/sim/simcollision/collisionanalysertranslator.cpp \
	libs/sim/simcollision/collision.cpp \
	libs/sim/simcollision/collisiondetector.cpp \
	libs/sim/simcollision/collisiondisplay.cpp \
	libs/sim/simcollision/collisionmanager.cpp \
	libs/sim/simcollision/collisionobject.cpp \
	libs/sim/simcollision/collisionvolume.cpp \
	libs/sim/simcollision/impulsebasedcollisionsolver.cpp \
	libs/sim/simcollision/proximitydetection.cpp \
	libs/sim/simcollision/subcollisiondetector.cpp \
	libs/sim/simcommon/arraymath.cpp \
	libs/sim/simcommon/dline2.cpp \
	libs/sim/simcommon/impulselink.cpp \
	libs/sim/simcommon/kalmann.cpp \
	libs/sim/simcommon/largesymmetricmatrix.cpp \
	libs/sim/simcommon/physicsproperties.cpp \
	libs/sim/simcommon/sbmatrix.cpp \
	libs/sim/simcommon/simconstraint.cpp \
	libs/sim/simcommon/simenvironment.cpp \
	libs/sim/simcommon/simmath.cpp \
	libs/sim/simcommon/simstatearticulated.cpp \
	libs/sim/simcommon/simstate.cpp \
	libs/sim/simcommon/simstateflexible.cpp \
	libs/sim/simcommon/simstatetarget.cpp \
	libs/sim/simcommon/simtarget.cpp \
	libs/sim/simcommon/simulatedobject.cpp \
	libs/sim/simcommon/simutility.cpp \
	libs/sim/simcommon/skeletoninfo.cpp \
	libs/sim/simcommon/symmatrix.cpp \
	libs/sim/simcommon/tlist.cpp \
	libs/sim/simcommon/trackerjointmodifier.cpp \
	libs/sim/simcommon/trajectoryextrapolator.cpp \
	libs/sim/simphysics/articulatedphysicsobject.cpp \
	libs/sim/simphysics/physicsjoint.cpp \
	libs/sim/simphysics/physicsobject.cpp \
	libs/sim/simphysics/restingdetector.cpp \
	libs/sim/simphysics/virtualcm.cpp

CHOREO_SRC := \
	libs/choreo/src/animation.cpp \
	libs/choreo/src/bank.cpp \
	libs/choreo/src/basebank.cpp \
	libs/choreo/src/blend.cpp \
	libs/choreo/src/blendtemplate.cpp \
	libs/choreo/src/constants.cpp \
	libs/choreo/src/driver.cpp \
	libs/choreo/src/engine.cpp \
	libs/choreo/src/footblenddriver.cpp \
	libs/choreo/src/footblender.cpp \
	libs/choreo/src/foot.cpp \
	libs/choreo/src/footplanter.cpp \
	libs/choreo/src/internalanimation.cpp \
	libs/choreo/src/jointblenddriver.cpp \
	libs/choreo/src/jointblender.cpp \
	libs/choreo/src/load.cpp \
	libs/choreo/src/locomotion.cpp \
	libs/choreo/src/partition.cpp \
	libs/choreo/src/puppet.cpp \
	libs/choreo/src/replayblenddriver.cpp \
	libs/choreo/src/replayblender.cpp \
	libs/choreo/src/replay.cpp \
	libs/choreo/src/rig.cpp \
	libs/choreo/src/rootblenddriver.cpp \
	libs/choreo/src/rootblender.cpp \
	libs/choreo/src/root.cpp \
	libs/choreo/src/scriptreader.cpp \
	libs/choreo/src/scriptwriter.cpp \
	libs/choreo/src/synchronization.cpp \
	libs/choreo/src/transition.cpp \
	libs/choreo/src/utility.cpp

ifeq ($(AUDIO),1)
SRR2_SOUND_SRC := \
	code/sound/avatar/avatarsoundplayer.cpp code/sound/avatar/carsoundparameters.cpp \
	code/sound/avatar/soundavatar.cpp code/sound/avatar/vehiclesounddebugpage.cpp \
	code/sound/avatar/vehiclesoundplayer.cpp \
	code/sound/dialog/conversation.cpp code/sound/dialog/conversationmatcher.cpp \
	code/sound/dialog/dialogcoordinator.cpp code/sound/dialog/dialogline.cpp \
	code/sound/dialog/dialoglist.cpp code/sound/dialog/dialogpriorityqueue.cpp \
	code/sound/dialog/dialogqueueelement.cpp \
	code/sound/dialog/dialogselectiongroup.cpp \
	code/sound/dialog/dialogsounddebugpage.cpp \
	code/sound/dialog/playabledialog.cpp code/sound/dialog/selectabledialog.cpp \
	code/sound/listener.cpp \
	code/sound/movingpositional/actorplayer.cpp \
	code/sound/movingpositional/aivehiclesoundplayer.cpp \
	code/sound/movingpositional/animobjsoundplayer.cpp \
	code/sound/movingpositional/avatarvehicleposnplayer.cpp \
	code/sound/movingpositional/movingsoundmanager.cpp \
	code/sound/movingpositional/platformsoundplayer.cpp \
	code/sound/movingpositional/trafficsoundplayer.cpp \
	code/sound/movingpositional/vehicleposnsoundplayer.cpp \
	code/sound/movingpositional/waspsoundplayer.cpp \
	code/sound/music/musicplayer.cpp code/sound/nis/nissoundplayer.cpp \
	code/sound/positionalsoundplayer.cpp code/sound/simpsonssoundplayer.cpp \
	code/sound/soundcluster.cpp code/sound/sounddebug/sounddebugdisplay.cpp \
	code/sound/sounddebug/sounddebugpage.cpp \
	code/sound/soundfx/positionalsoundsettings.cpp \
	code/sound/soundfx/reverbcontroller.cpp code/sound/soundfx/reverbsettings.cpp \
	code/sound/soundfx/soundeffectplayer.cpp \
	code/sound/soundfx/soundfxfrontendlogic.cpp \
	code/sound/soundfx/soundfxgameplaylogic.cpp \
	code/sound/soundfx/soundfxlogic.cpp code/sound/soundfx/soundfxpauselogic.cpp \
	code/sound/soundfx/win32reverbcontroller.cpp \
	code/sound/soundloader.cpp code/sound/soundmanager.cpp \
	code/sound/soundrenderercallback.cpp \
	code/sound/soundrenderer/dasoundplayer.cpp code/sound/soundrenderer/fader.cpp \
	code/sound/soundrenderer/musicsoundplayer.cpp \
	code/sound/soundrenderer/playermanager.cpp \
	code/sound/soundrenderer/scripts/apu.cpp code/sound/soundrenderer/scripts/bart.cpp \
	code/sound/soundrenderer/scripts/cars.cpp \
	code/sound/soundrenderer/scripts/effects.cpp \
	code/sound/soundrenderer/scripts/english.cpp \
	code/sound/soundrenderer/scripts/homer.cpp \
	code/sound/soundrenderer/scripts/levels.cpp \
	code/sound/soundrenderer/scripts/lisa.cpp \
	code/sound/soundrenderer/scripts/marge.cpp \
	code/sound/soundrenderer/scripts/tuning.cpp \
	code/sound/soundrenderer/soundallocatedresource.cpp \
	code/sound/soundrenderer/sounddynaload.cpp \
	code/sound/soundrenderer/soundnucleus.cpp \
	code/sound/soundrenderer/soundrenderingmanager.cpp \
	code/sound/soundrenderer/soundresource.cpp \
	code/sound/soundrenderer/soundresourcemanager.cpp \
	code/sound/soundrenderer/soundtuner.cpp \
	code/sound/soundrenderer/tunerdebugpage.cpp \
	code/sound/soundrenderer/wireplayers.cpp \
	code/sound/soundrenderer/wiresystem.cpp code/sound/tuning/globalsettings.cpp
else
SRR2_SOUND_SRC := code/sound/soundmanager_stub.cpp
endif

SRR2_SRC := \
	code/ai/actionbuttonhandler.cpp code/ai/actionbuttonmanager.cpp \
	code/ai/actor/ActorAnimationUFO.cpp code/ai/actor/actoranimationwasp.cpp \
	code/ai/actor/actor.cpp code/ai/actor/actordsg.cpp code/ai/actor/actormanager.cpp \
	code/ai/actor/attackbehaviour.cpp code/ai/actor/attractionbehaviour.cpp \
	code/ai/actor/cutcambehaviour.cpp code/ai/actor/evasionbehaviour.cpp \
	code/ai/actor/flyingactor.cpp code/ai/actor/intersectionlist.cpp \
	code/ai/actor/projectile.cpp code/ai/actor/projectiledsg.cpp \
	code/ai/actor/spawnpoint.cpp code/ai/actor/stunnedbehaviour.cpp \
	code/ai/actor/ufoattackbehaviour.cpp code/ai/actor/ufobeamalwaysonbehaviour.cpp \
	code/ai/actor/ufobehaviour.cpp code/ai/automaticdoor.cpp code/ai/playanimonce.cpp \
	code/ai/sequencer/actioncontroller.cpp code/ai/sequencer/action.cpp \
	code/ai/sequencer/sequencer.cpp code/ai/sequencer/task.cpp \
	code/ai/state.cpp code/ai/statemanager.cpp \
	code/ai/vehicle/chaseai.cpp code/ai/vehicle/potentialfield.cpp \
	code/ai/vehicle/potentials.cpp code/ai/vehicle/trafficai.cpp \
	code/ai/vehicle/vehicleai.cpp code/ai/vehicle/vehicleairender.cpp \
	code/ai/vehicle/waypointai.cpp \
	code/atc/atcloader.cpp code/atc/atcmanager.cpp \
	code/camera/animatedcam.cpp code/camera/bumpercam.cpp code/camera/chasecam.cpp \
	code/camera/conversationcam.cpp code/camera/debugcam.cpp \
	code/camera/firstpersoncam.cpp code/camera/followcam.cpp code/camera/kullcam.cpp \
	code/camera/railcam.cpp code/camera/relativeanimatedcam.cpp \
	code/camera/reversecam.cpp code/camera/sinecosshaker.cpp \
	code/camera/snapshotcam.cpp code/camera/staticcam.cpp \
	code/camera/supercamcentral.cpp code/camera/supercamcontroller.cpp \
	code/camera/supercam.cpp code/camera/supercammanager.cpp \
	code/camera/supersprintcam.cpp code/camera/surveillancecam.cpp \
	code/camera/trackercam.cpp code/camera/walkercam.cpp \
	code/camera/wrecklesscam.cpp code/camera/wrecklesseventlistener.cpp \
	code/cards/bonuscard.cpp code/cards/card.cpp code/cards/cardgallery.cpp \
	code/cards/cardsdb.cpp code/cards/collectorcard.cpp \
	code/cheats/cheatinputhandler.cpp code/cheats/cheatinputsystem.cpp \
	code/cheats/cheats.cpp \
	code/console/console.cpp code/console/debugconsolecallback.cpp \
	code/console/fbstricmp.cpp code/console/nameinsensitive.cpp code/console/upcase.cpp \
	code/contexts/bootupcontext.cpp code/contexts/context.cpp \
	code/contexts/demo/democontext.cpp code/contexts/demo/loadingdemocontext.cpp \
	code/contexts/entrycontext.cpp code/contexts/exitcontext.cpp \
	code/contexts/frontendcontext.cpp code/contexts/gameplay/gameplaycontext.cpp \
	code/contexts/gameplay/loadinggameplaycontext.cpp code/contexts/loadingcontext.cpp \
	code/contexts/pausecontext.cpp code/contexts/playingcontext.cpp \
	code/contexts/supersprint/loadingsupersprintcontext.cpp \
	code/contexts/supersprint/supersprintcontext.cpp \
	code/contexts/supersprint/supersprintfecontext.cpp \
	code/data/gamedatamanager.cpp code/data/memcard/memorycardmanager.cpp \
	code/data/PersistentWorldManager.cpp code/data/savegameinfo.cpp \
	code/debug/debuginfo.cpp code/debug/profiler.cpp code/debug/section.cpp \
	code/events/eventlistener.cpp code/events/eventmanager.cpp \
	code/gameflow/gameflow.cpp \
	code/input/button.cpp code/input/inputmanager.cpp code/input/mappable.cpp \
	code/input/mapper.cpp code/input/MouseCursor.cpp code/input/RealController.cpp \
	code/input/rumbleeffect.cpp code/input/rumblewin32.cpp code/input/usercontroller.cpp \
	code/interiors/interiormanager.cpp \
	code/loading/cameradataloader.cpp code/loading/cementfilehandler.cpp \
	code/loading/choreofilehandler.cpp code/loading/consolefilehandler.cpp \
	code/loading/filehandlerfactory.cpp code/loading/iconfilehandler.cpp \
	code/loading/intersectionloader.cpp code/loading/loadingmanager.cpp \
	code/loading/locatorloader.cpp code/loading/p3dfilehandler.cpp \
	code/loading/pathloader.cpp code/loading/roaddatasegmentloader.cpp \
	code/loading/roadloader.cpp code/loading/scroobyfilehandler.cpp \
	code/loading/soundfilehandler.cpp \
	code/main/commandlineoptions.cpp code/main/game.cpp code/main/pchsrr2.cpp \
	code/main/singletons.cpp code/main/tuidunaligned.cpp \
	code/main/win32main.cpp code/main/win32platform.cpp \
	code/memory/classsizetracker.cpp code/memory/createheap.cpp \
	code/memory/leakdetection.cpp code/memory/memorypool.cpp \
	code/memory/memoryutilities.cpp code/memory/propstats.cpp code/memory/srrmemory.cpp \
	code/meta/actioneventlocator.cpp code/meta/carstartlocator.cpp \
	code/meta/directionallocator.cpp code/meta/eventlocator.cpp \
	code/meta/fovlocator.cpp code/meta/interiorentrancelocator.cpp \
	code/meta/locator.cpp code/meta/occlusionlocator.cpp \
	code/meta/recttriggervolume.cpp code/meta/scriptlocator.cpp \
	code/meta/spheretriggervolume.cpp code/meta/splinelocator.cpp \
	code/meta/staticcamlocator.cpp code/meta/triggerlocator.cpp \
	code/meta/triggervolume.cpp code/meta/triggervolumetracker.cpp \
	code/meta/zoneeventlocator.cpp \
	code/mission/animatedicon.cpp code/mission/bonusmissioninfo.cpp \
	code/mission/charactersheet/charactersheetmanager.cpp \
	code/mission/conditions/damagecondition.cpp code/mission/conditions/followcondition.cpp \
	code/mission/conditions/getoutofcarcondition.cpp \
	code/mission/conditions/keepbarrelcondition.cpp \
	code/mission/conditions/leaveinteriorcondition.cpp \
	code/mission/conditions/missioncondition.cpp \
	code/mission/conditions/notabductedcondition.cpp \
	code/mission/conditions/outofboundscondition.cpp \
	code/mission/conditions/positioncondition.cpp \
	code/mission/conditions/racecondition.cpp \
	code/mission/conditions/timeoutcondition.cpp \
	code/mission/conditions/vehiclecarryingstateprop.cpp \
	code/mission/conditions/vehiclecondition.cpp \
	code/mission/gameplaymanager.cpp code/mission/haspresentationinfo.cpp \
	code/mission/mission.cpp code/mission/missionmanager.cpp \
	code/mission/missionscriptloader.cpp code/mission/missionstage.cpp \
	code/mission/nocopbonusobjective.cpp code/mission/nodamagebonusobjective.cpp \
	code/mission/objectives/buycarobjective.cpp \
	code/mission/objectives/buyskinobjective.cpp \
	code/mission/objectives/coinobjective.cpp \
	code/mission/objectives/collectdumpedobjective.cpp \
	code/mission/objectives/collectibleobjective.cpp \
	code/mission/objectives/deliveryobjective.cpp \
	code/mission/objectives/destroybossobjective.cpp \
	code/mission/objectives/destroyobjective.cpp \
	code/mission/objectives/dialogueobjective.cpp \
	code/mission/objectives/fmvobjective.cpp \
	code/mission/objectives/followobjective.cpp \
	code/mission/objectives/getinobjective.cpp \
	code/mission/objectives/gooutsideobjective.cpp \
	code/mission/objectives/gotoobjective.cpp \
	code/mission/objectives/interiorobjective.cpp \
	code/mission/objectives/loadvehicleobjective.cpp \
	code/mission/objectives/loseobjective.cpp \
	code/mission/objectives/missionobjective.cpp \
	code/mission/objectives/pickupitemobjective.cpp \
	code/mission/objectives/raceobjective.cpp \
	code/mission/objectives/talktoobjective.cpp \
	code/mission/objectives/timerobjective.cpp \
	code/mission/racepositionbonusobjective.cpp \
	code/mission/respawnmanager/respawnentity.cpp \
	code/mission/respawnmanager/respawnmanager.cpp \
	code/mission/rewards/merchandise.cpp code/mission/rewards/reward.cpp \
	code/mission/rewards/rewardsmanager.cpp code/mission/safezone/safezone.cpp \
	code/mission/statepropcollectible.cpp code/mission/timeremainbonusobjective.cpp \
	code/mission/ufo/tractorbeam.cpp code/mission/ufo/ufo.cpp \
	code/pedpaths/path.cpp code/pedpaths/pathmanager.cpp code/pedpaths/pathsegment.cpp \
	code/presentation/animplayer.cpp code/presentation/blinker.cpp \
	code/presentation/cameraplayer.cpp \
	code/presentation/fmvplayer/fmvplayer.cpp \
	code/presentation/fmvplayer/fmvuserinputhandler.cpp \
	code/presentation/gui/backend/guimanagerbackend.cpp \
	code/presentation/gui/backend/guiscreendemo.cpp \
	code/presentation/gui/backend/guiscreenloading.cpp \
	code/presentation/gui/backend/guiscreenloadingfe.cpp \
	code/presentation/gui/bootup/guimanagerbootup.cpp \
	code/presentation/gui/bootup/guimanagerlanguage.cpp \
	code/presentation/gui/bootup/guiscreenbootupload.cpp \
	code/presentation/gui/bootup/guiscreenlanguage.cpp \
	code/presentation/gui/bootup/guiscreenlicense.cpp \
	code/presentation/gui/frontend/guimanagerfrontend.cpp \
	code/presentation/gui/frontend/guiscreencardgallery.cpp \
	code/presentation/gui/frontend/guiscreencontroller.cpp \
	code/presentation/gui/frontend/guiscreenloadgame.cpp \
	code/presentation/gui/frontend/guiscreenmainmenu.cpp \
	code/presentation/gui/frontend/guiscreenmissiongallery.cpp \
	code/presentation/gui/frontend/guiscreenoptions.cpp \
	code/presentation/gui/frontend/guiscreenplaymovie.cpp \
	code/presentation/gui/frontend/guiscreenscrapbookcontents.cpp \
	code/presentation/gui/frontend/guiscreenscrapbook.cpp \
	code/presentation/gui/frontend/guiscreenscrapbookstats.cpp \
	code/presentation/gui/frontend/guiscreenskingallery.cpp \
	code/presentation/gui/frontend/guiscreensound.cpp \
	code/presentation/gui/frontend/guiscreensplash.cpp \
	code/presentation/gui/frontend/guiscreenvehiclegallery.cpp \
	code/presentation/gui/frontend/guiscreenviewcredits.cpp \
	code/presentation/gui/frontend/guiscreenviewmovies.cpp \
	code/presentation/gui/guientity.cpp code/presentation/gui/guimanager.cpp \
	code/presentation/gui/guimenu.cpp code/presentation/gui/guimenuitem.cpp \
	code/presentation/gui/guiscreen.cpp code/presentation/gui/guiscreenmemcardcheck.cpp \
	code/presentation/gui/guiscreenmemorycard.cpp \
	code/presentation/gui/guiscreenmessage.cpp \
	code/presentation/gui/guiscreenprompt.cpp code/presentation/gui/guisystem.cpp \
	code/presentation/gui/guitextbible.cpp \
	code/presentation/gui/guiuserinputhandler.cpp \
	code/presentation/gui/guiwindow.cpp \
	code/presentation/gui/ingame/guimanageringame.cpp \
	code/presentation/gui/ingame/guiscreencreditspostfmv.cpp \
	code/presentation/gui/ingame/guiscreenhastransitions.cpp \
	code/presentation/gui/ingame/guiscreenhud.cpp \
	code/presentation/gui/ingame/guiscreeniriswipe.cpp \
	code/presentation/gui/ingame/guiscreenletterbox.cpp \
	code/presentation/gui/ingame/guiscreenlevelend.cpp \
	code/presentation/gui/ingame/guiscreenlevelstats.cpp \
	code/presentation/gui/ingame/guiscreenmissionbase.cpp \
	code/presentation/gui/ingame/guiscreenmissionload.cpp \
	code/presentation/gui/ingame/guiscreenmissionover.cpp \
	code/presentation/gui/ingame/guiscreenmissionselect.cpp \
	code/presentation/gui/ingame/guiscreenmissionsuccess.cpp \
	code/presentation/gui/ingame/guiscreenmultihud.cpp \
	code/presentation/gui/ingame/guiscreenpausecontroller.cpp \
	code/presentation/gui/ingame/guiscreenpause.cpp \
	code/presentation/gui/ingame/guiscreenpausemission.cpp \
	code/presentation/gui/ingame/guiscreenpauseoptions.cpp \
	code/presentation/gui/ingame/guiscreenpausesettings.cpp \
	code/presentation/gui/ingame/guiscreenpausesound.cpp \
	code/presentation/gui/ingame/guiscreenpausesunday.cpp \
	code/presentation/gui/ingame/guiscreenphonebooth.cpp \
	code/presentation/gui/ingame/guiscreenpurchaserewards.cpp \
	code/presentation/gui/ingame/guiscreenrewards.cpp \
	code/presentation/gui/ingame/guiscreensavegame.cpp \
	code/presentation/gui/ingame/guiscreentutorial.cpp \
	code/presentation/gui/ingame/guiscreenviewcards.cpp \
	code/presentation/gui/ingame/hudevents/hudcardcollected.cpp \
	code/presentation/gui/ingame/hudevents/hudcoincollected.cpp \
	code/presentation/gui/ingame/hudevents/hudcountdown.cpp \
	code/presentation/gui/ingame/hudevents/hudeventhandler.cpp \
	code/presentation/gui/ingame/hudevents/hudhitnrun.cpp \
	code/presentation/gui/ingame/hudevents/huditemdropped.cpp \
	code/presentation/gui/ingame/hudevents/hudmissionobjective.cpp \
	code/presentation/gui/ingame/hudevents/hudmissionprogress.cpp \
	code/presentation/gui/ingame/hudevents/hudwaspdestroyed.cpp \
	code/presentation/gui/minigame/guimanagerminigame.cpp \
	code/presentation/gui/minigame/guiscreenminihud.cpp \
	code/presentation/gui/minigame/guiscreenminimenu.cpp \
	code/presentation/gui/minigame/guiscreenminipause.cpp \
	code/presentation/gui/minigame/guiscreenminisummary.cpp \
	code/presentation/gui/utility/hudmapcam.cpp \
	code/presentation/gui/utility/hudmap.cpp \
	code/presentation/gui/utility/numerictext.cpp \
	code/presentation/gui/utility/scrollingtext.cpp \
	code/presentation/gui/utility/slider.cpp \
	code/presentation/gui/utility/specialfx.cpp \
	code/presentation/gui/utility/teletypetext.cpp \
	code/presentation/gui/utility/transitions.cpp \
	code/presentation/language.cpp code/presentation/mouthflapper.cpp \
	code/presentation/nisplayer.cpp code/presentation/playerdrawable.cpp \
	code/presentation/presentationanimator.cpp code/presentation/presentation.cpp \
	code/presentation/presevents/fmvevent.cpp \
	code/presentation/presevents/nisevent.cpp \
	code/presentation/presevents/presentationevent.cpp \
	code/presentation/presevents/transevent.cpp \
	code/presentation/simpleanimationplayer.cpp \
	code/presentation/transitionplayer.cpp code/presentation/tutorialmanager.cpp \
	code/render/animentitydsgmanager/animentitydsgmanager.cpp \
	code/render/breakables/breakablesmanager.cpp \
	code/render/Culling/BoxPts.cpp code/render/Culling/CellBlock.cpp \
	code/render/Culling/Cell.cpp code/render/Culling/CoordSubList.cpp \
	code/render/Culling/CullData.cpp code/render/Culling/HexahedronP.cpp \
	code/render/Culling/ISpatialProxy.cpp code/render/Culling/OctTreeData.cpp \
	code/render/Culling/OctTreeNode.cpp code/render/Culling/SpatialTree.cpp \
	code/render/Culling/SpatialTreeIter.cpp code/render/Culling/SphereSP.cpp \
	code/render/Culling/VectorLib.cpp code/render/Culling/WorldScene.cpp \
	code/render/DSG/animcollisionentitydsg.cpp code/render/DSG/animentitydsg.cpp \
	code/render/DSG/breakableobjectdsg.cpp code/render/DSG/collisionentitydsg.cpp \
	code/render/DSG/DSGFactory.cpp code/render/DSG/DynaPhysDSG.cpp \
	code/render/DSG/FenceEntityDSG.cpp code/render/DSG/IEntityDSG.cpp \
	code/render/DSG/InstAnimDynaPhysDSG.cpp code/render/DSG/InstDynaPhysDSG.cpp \
	code/render/DSG/InstStatEntityDSG.cpp code/render/DSG/InstStatPhysDSG.cpp \
	code/render/DSG/IntersectDSG.cpp code/render/DSG/LensFlareDSG.cpp \
	code/render/DSG/StatePropDSG.cpp code/render/DSG/StaticEntityDSG.cpp \
	code/render/DSG/StaticPhysDSG.cpp code/render/DSG/TriStripDSG.cpp \
	code/render/DSG/WorldSphereDSG.cpp \
	code/render/IntersectManager/IntersectManager.cpp \
	code/render/Loaders/AllWrappers.cpp code/render/Loaders/AnimCollLoader.cpp \
	code/render/Loaders/AnimDSGLoader.cpp code/render/Loaders/AnimDynaPhysLoader.cpp \
	code/render/Loaders/BillboardWrappedLoader.cpp \
	code/render/Loaders/breakableobjectloader.cpp \
	code/render/Loaders/DynaPhysLoader.cpp code/render/Loaders/FenceLoader.cpp \
	code/render/Loaders/GeometryWrappedLoader.cpp \
	code/render/Loaders/instparticlesystemloader.cpp \
	code/render/Loaders/InstStatEntityLoader.cpp \
	code/render/Loaders/InstStatPhysLoader.cpp \
	code/render/Loaders/IntersectLoader.cpp code/render/Loaders/LensFlareLoader.cpp \
	code/render/Loaders/StaticEntityLoader.cpp \
	code/render/Loaders/StaticPhysLoader.cpp \
	code/render/Loaders/TreeDSGLoader.cpp code/render/Loaders/WorldSphereLoader.cpp \
	code/render/Particles/particlemanager.cpp \
	code/render/Particles/particlesystemdsg.cpp \
	code/render/Particles/vehicleparticleemitter.cpp \
	code/render/RenderFlow/renderflow.cpp \
	code/render/RenderManager/FrontEndRenderLayer.cpp \
	code/render/RenderManager/RenderLayer.cpp \
	code/render/RenderManager/RenderManager.cpp \
	code/render/RenderManager/WorldRenderLayer.cpp \
	code/roads/geometry.cpp code/roads/intersection.cpp code/roads/lane.cpp \
	code/roads/road.cpp code/roads/roadmanager.cpp code/roads/roadrender.cpp \
	code/roads/roadrendertest.cpp code/roads/roadsegment.cpp \
	code/roads/roadsegmentdata.cpp code/roads/trafficcontrol.cpp \
	$(SRR2_SOUND_SRC) \
	code/stateprop/stateprop.cpp code/stateprop/statepropdata.cpp \
	code/supersprint/supersprintdata.cpp code/supersprint/supersprintmanager.cpp \
	code/worldsim/avatar.cpp code/worldsim/avatarmanager.cpp \
	code/worldsim/character/aicharactercontroller.cpp \
	code/worldsim/character/charactercontroller.cpp \
	code/worldsim/character/character.cpp code/worldsim/character/charactermanager.cpp \
	code/worldsim/character/charactermappable.cpp \
	code/worldsim/character/characterrenderable.cpp \
	code/worldsim/character/charactertarget.cpp \
	code/worldsim/character/footprint/footprintmanager.cpp \
	code/worldsim/coins/coinmanager.cpp code/worldsim/coins/sparkle.cpp \
	code/worldsim/groundplanepool.cpp code/worldsim/harass/chasemanager.cpp \
	code/worldsim/hitnrunmanager.cpp code/worldsim/huskpool.cpp \
	code/worldsim/parkedcars/parkedcarmanager.cpp \
	code/worldsim/ped/pedestrian.cpp code/worldsim/ped/pedestrianmanager.cpp \
	code/worldsim/redbrick/geometryvehicle.cpp \
	code/worldsim/redbrick/physicslocomotioncontrollerforces.cpp \
	code/worldsim/redbrick/physicslocomotion.cpp \
	code/worldsim/redbrick/redbrickcollisionsolveragent.cpp \
	code/worldsim/redbrick/rootmatrixdriver.cpp \
	code/worldsim/redbrick/suspensionjointdriver.cpp \
	code/worldsim/redbrick/trafficbodydrawable.cpp \
	code/worldsim/redbrick/trafficlocomotion.cpp \
	code/worldsim/redbrick/vehiclecontroller/aivehiclecontroller.cpp \
	code/worldsim/redbrick/vehiclecontroller/humanvehiclecontroller.cpp \
	code/worldsim/redbrick/vehiclecontroller/vehiclecontroller.cpp \
	code/worldsim/redbrick/vehiclecontroller/vehiclemappable.cpp \
	code/worldsim/redbrick/vehicle.cpp code/worldsim/redbrick/vehicleeventlistener.cpp \
	code/worldsim/redbrick/vehicleinit.cpp \
	code/worldsim/redbrick/vehiclelocomotion.cpp code/worldsim/redbrick/wheel.cpp \
	code/worldsim/skidmarks/skidmark.cpp \
	code/worldsim/skidmarks/SkidMarkGenerator.cpp \
	code/worldsim/skidmarks/skidmarkmanager.cpp \
	code/worldsim/spawn/spawnmanager.cpp code/worldsim/traffic/trafficmanager.cpp \
	code/worldsim/vehiclecentral.cpp code/worldsim/worldcollisionsolveragent.cpp \
	code/worldsim/worldobject.cpp code/worldsim/worldphysicsmanager.cpp

# =============================================================================
# Object file generation
# =============================================================================

to_obj = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(patsubst %.c,$(BUILDDIR)/%.o,$(1)))

RADMATH_OBJ    := $(call to_obj,$(RADMATH_SRC))
RADCORE_OBJ    := $(call to_obj,$(RADCORE_SRC))
PDDI_OBJ       := $(call to_obj,$(PDDI_SRC)) $(call to_obj,$(PDDI_C_SRC))
P3D_OBJ        := $(call to_obj,$(P3D_SRC))
RADCONTENT_OBJ := $(call to_obj,$(RADCONTENT_SRC))
RADSCRIPT_OBJ  := $(call to_obj,$(RADSCRIPT_SRC))
RADSOUND_OBJ   := $(call to_obj,$(RADSOUND_SRC))
RADMUSIC_OBJ   := $(call to_obj,$(RADMUSIC_SRC))
RADMOVIE_OBJ   := $(call to_obj,$(RADMOVIE_SRC))
POSER_OBJ      := $(call to_obj,$(POSER_SRC))
SCROOBY_OBJ    := $(call to_obj,$(SCROOBY_SRC))
SIM_OBJ        := $(call to_obj,$(SIM_SRC))
CHOREO_OBJ     := $(call to_obj,$(CHOREO_SRC))
SRR2_OBJ       := $(call to_obj,$(SRR2_SRC))

ALL_LIBS := \
	$(BUILDDIR)/libradmath.a \
	$(BUILDDIR)/libradcore.a \
	$(BUILDDIR)/libpddi.a \
	$(BUILDDIR)/libp3d.a \
	$(BUILDDIR)/libradcontent.a \
	$(BUILDDIR)/libradscript.a \
	$(BUILDDIR)/libposer.a \
	$(BUILDDIR)/libscrooby.a \
	$(BUILDDIR)/libsim.a \
	$(BUILDDIR)/libchoreo.a

ifeq ($(AUDIO),1)
ALL_LIBS += \
	$(BUILDDIR)/libradsound.a \
	$(BUILDDIR)/libradmusic.a \
	$(BUILDDIR)/libradmovie.a
AUDIO_LINK := -lradmovie -lradmusic -lradsound
else
AUDIO_LINK :=
endif

TARGET := $(ROOT)base/SRR2

# =============================================================================
# Rules
# =============================================================================

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRR2_OBJ) $(ALL_LIBS)
	$(CXX) $(LDFLAGS) -o $@ $(SRR2_OBJ) \
		-L$(BUILDDIR) \
		-lchoreo -lscrooby -lsim -lposer $(AUDIO_LINK) \
		-lp3d -lpddi -lradcontent -lradscript -lradcore -lradmath \
		$(LIBS)

$(BUILDDIR)/libradmath.a: $(RADMATH_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libradcore.a: $(RADCORE_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libpddi.a: $(PDDI_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libp3d.a: $(P3D_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libradcontent.a: $(RADCONTENT_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libradscript.a: $(RADSCRIPT_OBJ)
	$(AR) $@ $^
ifeq ($(AUDIO),1)
$(BUILDDIR)/libradsound.a: $(RADSOUND_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libradmusic.a: $(RADMUSIC_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libradmovie.a: $(RADMOVIE_OBJ)
	$(AR) $@ $^
endif
$(BUILDDIR)/libposer.a: $(POSER_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libscrooby.a: $(SCROOBY_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libsim.a: $(SIM_OBJ)
	$(AR) $@ $^
$(BUILDDIR)/libchoreo.a: $(CHOREO_OBJ)
	$(AR) $@ $^

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(I_ALL) -c -o $@ $<

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(I_ALL) -c -o $@ $<

clean:
	rm -rf $(BUILDDIR) $(TARGET)
