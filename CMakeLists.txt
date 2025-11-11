cmake_minimum_required(VERSION 3.20)

project(SRR2)

set(SRR2_TITLE "Simpsons Hit & Run")
set(SRR2_VERSION "00.61")
set(SRR2_FILENAME "Simpsons")
set(SRR2_REGION "NTSC" CACHE STRING "Select PAL for the EFIGS multi-language version")
set_property(CACHE SRR2_REGION PROPERTY STRINGS "NTSC" "PAL")

option(SRR2_USE_DLMALLOC "Use Doug Lea's memory allocator for fixed-size heaps" OFF)
option(SRR2_REROUTE_ALL_HEAP_ALLOCATIONS "Override new operator to track all heap allocations" OFF)
option(SRR2_USE_BINK "Use RAD Game Tools Bink SDK instead of FFmpeg" OFF)
option(SRR2_BUILD_TESTS "Build sample programs for internal libraries" ON)
option(SRR2_MICROPROFILE "Use microprofile instead of the internal profiler" OFF)
option(SRR2_P3D_CG "Use CG shaders rather than GLSL for OpenGL ES" OFF)
set(SRR2_P3D_PDDI "OpenGL" CACHE STRING "Pure3D PDDI implementation")
if(VITA)
	option(SRR2_USE_VITAGL "Use VitaGL as an OpenGL (ES) implementation" ON)
	set_property(CACHE SRR2_P3D_PDDI PROPERTY STRINGS "OpenGL" "GLES1" "GLES2" "GXM")
else()
	set_property(CACHE SRR2_P3D_PDDI PROPERTY STRINGS "OpenGL" "GLES1" "GLES2")
endif()

if(VITA)
	include("${VITASDK}/share/vita.cmake" REQUIRED)

	add_compile_definitions(RAD_VITA)
	## Configuration options for this app
	# Display name (under bubble in LiveArea)
	set(VITA_APP_NAME ${SRR2_TITLE})
	# Unique ID must be exactly 9 characters. Recommended: XXXXYYYYY where X =
	# unique string of developer and Y = a unique number for this app
	set(VITA_TITLEID  "ROADRAGE2")
	# Optional version string to show in LiveArea's more info screen
	set(VITA_VERSION ${SRR2_VERSION})
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

find_package(SDL3)
if(SDL3_FOUND)
	get_target_property(SDL_INCLUDE_DIR SDL3::Headers INTERFACE_INCLUDE_DIRECTORIES)
	target_include_directories(SDL3::Headers INTERFACE ${SDL_INCLUDE_DIR}/SDL3)
	set(SDL_LIBRARY SDL3::SDL3)
else()
	find_package(SDL2 REQUIRED)
	set(SDL_LIBRARY SDL2::SDL2 SDL2::SDL2main)
endif()
find_package(PNG REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SRR2_OPENAL_IMPL "OpenAL-Soft" CACHE STRING "OpenAL implementation")
set_property(CACHE SRR2_OPENAL_IMPL PROPERTY STRINGS "OpenAL-Soft" "MojoAL")

if (SRR2_OPENAL_IMPL STREQUAL "OpenAL-Soft")
	find_package(OpenAL REQUIRED)
	set(SRR2_OPENAL_TARGET OpenAL::OpenAL)
else()
	add_subdirectory(libs/sdks/mojoal SYSTEM)
	set(SRR2_OPENAL_TARGET mojoal)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fvisibility=hidden)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_compile_options(
			-Werror
			-ggdb
		)
	endif()
	if(VITA)
		add_compile_options(
			-mtune=cortex-a9
			-mfpu=neon
		)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(RAD_DEBUG)
else()
	add_compile_definitions(RAD_RELEASE)
endif()

add_compile_definitions(
	RAD_CONSOLE
	RAD_WIN32
)

if(SRR2_REGION STREQUAL "PAL")
	add_compile_definitions(PAL)
endif()
if(SRR2_USE_DLMALLOC)
	add_compile_definitions(USE_DOUG_LEA_HEAP)
endif()
if(SRR2_REROUTE_ALL_HEAP_ALLOCATIONS)
	add_compile_definitions(REROUTE_ALL_HEAP_ALLOCATIONS)
endif()
if(SRR2_USE_BINK)
	add_compile_definitions(RAD_MOVIEPLAYER_USE_BINK)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET
			libavformat
			libavcodec
			libavutil
			libswresample
			libswscale
	)
endif()
if(SRR2_MICROPROFILE)
	add_compile_definitions(MICROPROFILE_ENABLED)
	list(APPEND RADCORE_SOURCES	"src/radprofiler/microprofile.cpp")
endif()
if(SRR2_P3D_CG)
	add_compile_definitions(RAD_CG)
endif()
if(SRR2_USE_VITAGL)
    add_compile_definitions(RAD_VITAGL)
endif()
if(SRR2_P3D_PDDI STREQUAL "GXM")
    add_compile_definitions(RAD_GXM)
endif()
if(SRR2_P3D_PDDI STREQUAL "GLES1")
	add_compile_definitions(RAD_GLES)
	add_compile_definitions(RAD_GLES_VERSION=1)
endif()
if(SRR2_P3D_PDDI STREQUAL "GLES2")
	add_compile_definitions(RAD_GLES)
	add_compile_definitions(RAD_GLES_VERSION=2)
endif()

if(WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(libs)

set(SRR2_SOURCES
	"code/ai/actionbuttonhandler.cpp"
	"code/ai/actionbuttonmanager.cpp"
	"code/ai/actor/ActorAnimationUFO.cpp"
	"code/ai/actor/actoranimationwasp.cpp"
	"code/ai/actor/actor.cpp"
	"code/ai/actor/actordsg.cpp"
	"code/ai/actor/actormanager.cpp"
	"code/ai/actor/attackbehaviour.cpp"
	"code/ai/actor/attractionbehaviour.cpp"
	"code/ai/actor/cutcambehaviour.cpp"
	"code/ai/actor/evasionbehaviour.cpp"
	"code/ai/actor/flyingactor.cpp"
	"code/ai/actor/intersectionlist.cpp"
	"code/ai/actor/projectile.cpp"
	"code/ai/actor/projectiledsg.cpp"
	"code/ai/actor/spawnpoint.cpp"
	"code/ai/actor/stunnedbehaviour.cpp"
	"code/ai/actor/ufoattackbehaviour.cpp"
	"code/ai/actor/ufobeamalwaysonbehaviour.cpp"
	"code/ai/actor/ufobehaviour.cpp"
	"code/ai/automaticdoor.cpp"
	"code/ai/playanimonce.cpp"
	"code/ai/sequencer/actioncontroller.cpp"
	"code/ai/sequencer/action.cpp"
	"code/ai/sequencer/sequencer.cpp"
	"code/ai/sequencer/task.cpp"
	"code/ai/state.cpp"
	"code/ai/statemanager.cpp"
	"code/ai/vehicle/chaseai.cpp"
	"code/ai/vehicle/potentialfield.cpp"
	"code/ai/vehicle/potentials.cpp"
	"code/ai/vehicle/trafficai.cpp"
	"code/ai/vehicle/vehicleai.cpp"
	"code/ai/vehicle/vehicleairender.cpp"
	"code/ai/vehicle/waypointai.cpp"
	"code/atc/atcloader.cpp"
	"code/atc/atcmanager.cpp"
	"code/camera/animatedcam.cpp"
	"code/camera/bumpercam.cpp"
	"code/camera/chasecam.cpp"
	"code/camera/conversationcam.cpp"
	"code/camera/debugcam.cpp"
	"code/camera/firstpersoncam.cpp"
	"code/camera/followcam.cpp"
	"code/camera/kullcam.cpp"
	#"code/camera/pccam.cpp"
	"code/camera/railcam.cpp"
	"code/camera/relativeanimatedcam.cpp"
	"code/camera/reversecam.cpp"
	"code/camera/sinecosshaker.cpp"
	"code/camera/snapshotcam.cpp"
	"code/camera/staticcam.cpp"
	"code/camera/supercamcentral.cpp"
	"code/camera/supercamcontroller.cpp"
	"code/camera/supercam.cpp"
	"code/camera/supercammanager.cpp"
	"code/camera/supersprintcam.cpp"
	"code/camera/surveillancecam.cpp"
	"code/camera/trackercam.cpp"
	"code/camera/walkercam.cpp"
	"code/camera/wrecklesscam.cpp"
	"code/camera/wrecklesseventlistener.cpp"
	"code/cards/bonuscard.cpp"
	"code/cards/card.cpp"
	"code/cards/cardgallery.cpp"
	"code/cards/cardsdb.cpp"
	"code/cards/collectorcard.cpp"
	"code/cheats/cheatinputhandler.cpp"
	"code/cheats/cheatinputsystem.cpp"
	"code/cheats/cheats.cpp"
	"code/console/console.cpp"
	"code/console/debugconsolecallback.cpp"
	"code/console/fbstricmp.cpp"
	"code/console/nameinsensitive.cpp"
	"code/console/upcase.cpp"
	"code/contexts/bootupcontext.cpp"
	"code/contexts/context.cpp"
	"code/contexts/demo/democontext.cpp"
	"code/contexts/demo/loadingdemocontext.cpp"
	"code/contexts/entrycontext.cpp"
	"code/contexts/exitcontext.cpp"
	"code/contexts/frontendcontext.cpp"
	"code/contexts/gameplay/gameplaycontext.cpp"
	"code/contexts/gameplay/loadinggameplaycontext.cpp"
	"code/contexts/loadingcontext.cpp"
	"code/contexts/pausecontext.cpp"
	"code/contexts/playingcontext.cpp"
	"code/contexts/supersprint/loadingsupersprintcontext.cpp"
	"code/contexts/supersprint/supersprintcontext.cpp"
	"code/contexts/supersprint/supersprintfecontext.cpp"
	"code/data/gamedatamanager.cpp"
	"code/data/memcard/memorycardmanager.cpp"
	"code/data/PersistentWorldManager.cpp"
	"code/data/savegameinfo.cpp"
	"code/debug/debuginfo.cpp"
	"code/debug/profiler.cpp"
	"code/debug/section.cpp"
	"code/events/eventlistener.cpp"
	"code/events/eventmanager.cpp"
	"code/gameflow/gameflow.cpp"
	#"code/input/basedamper.cpp"
	"code/input/button.cpp"
	#"code/input/constanteffect.cpp"
	#"code/input/FEMouse.cpp"
	#"code/input/forceeffect.cpp"
	#"code/input/Gamepad.cpp"
	"code/input/inputmanager.cpp"
	#"code/input/Keyboard.cpp"
	"code/input/mappable.cpp"
	"code/input/mapper.cpp"
	#"code/input/Mouse.cpp"
	"code/input/MouseCursor.cpp"
	"code/input/RealController.cpp"
	"code/input/rumbleeffect.cpp"
	"code/input/rumblewin32.cpp"
	#"code/input/steeringspring.cpp"
	"code/input/usercontroller.cpp"
	#"code/input/usercontrollerWin32.cpp"
	#"code/input/virtualinputs.cpp"
	#"code/input/wheelrumble.cpp"
	"code/interiors/interiormanager.cpp"
	"code/loading/cameradataloader.cpp"
	"code/loading/cementfilehandler.cpp"
	"code/loading/choreofilehandler.cpp"
	"code/loading/consolefilehandler.cpp"
	"code/loading/filehandlerfactory.cpp"
	"code/loading/iconfilehandler.cpp"
	"code/loading/intersectionloader.cpp"
	"code/loading/loadingmanager.cpp"
	"code/loading/locatorloader.cpp"
	"code/loading/p3dfilehandler.cpp"
	"code/loading/pathloader.cpp"
	"code/loading/roaddatasegmentloader.cpp"
	"code/loading/roadloader.cpp"
	"code/loading/scroobyfilehandler.cpp"
	"code/loading/soundfilehandler.cpp"
	"code/main/commandlineoptions.cpp"
	"code/main/game.cpp"
	"code/main/pchsrr2.cpp"
	"code/main/singletons.cpp"
	"code/main/tuidunaligned.cpp"
	"code/main/win32main.cpp"
	"code/main/win32platform.cpp"
	"code/memory/classsizetracker.cpp"
	"code/memory/createheap.cpp"
	"code/memory/leakdetection.cpp"
	"code/memory/memorypool.cpp"
	"code/memory/memoryutilities.cpp"
	"code/memory/propstats.cpp"
	"code/memory/srrmemory.cpp"
	"code/meta/actioneventlocator.cpp"
	"code/meta/carstartlocator.cpp"
	"code/meta/directionallocator.cpp"
	"code/meta/eventlocator.cpp"
	"code/meta/fovlocator.cpp"
	"code/meta/interiorentrancelocator.cpp"
	"code/meta/locator.cpp"
	"code/meta/occlusionlocator.cpp"
	"code/meta/recttriggervolume.cpp"
	"code/meta/scriptlocator.cpp"
	"code/meta/spheretriggervolume.cpp"
	"code/meta/splinelocator.cpp"
	"code/meta/staticcamlocator.cpp"
	"code/meta/triggerlocator.cpp"
	"code/meta/triggervolume.cpp"
	"code/meta/triggervolumetracker.cpp"
	"code/meta/zoneeventlocator.cpp"
	"code/mission/animatedicon.cpp"
	"code/mission/bonusmissioninfo.cpp"
	"code/mission/charactersheet/charactersheetmanager.cpp"
	"code/mission/conditions/damagecondition.cpp"
	"code/mission/conditions/followcondition.cpp"
	"code/mission/conditions/getoutofcarcondition.cpp"
	"code/mission/conditions/keepbarrelcondition.cpp"
	"code/mission/conditions/leaveinteriorcondition.cpp"
	"code/mission/conditions/missioncondition.cpp"
	"code/mission/conditions/notabductedcondition.cpp"
	"code/mission/conditions/outofboundscondition.cpp"
	"code/mission/conditions/positioncondition.cpp"
	"code/mission/conditions/racecondition.cpp"
	"code/mission/conditions/timeoutcondition.cpp"
	"code/mission/conditions/vehiclecarryingstateprop.cpp"
	"code/mission/conditions/vehiclecondition.cpp"
	"code/mission/gameplaymanager.cpp"
	"code/mission/haspresentationinfo.cpp"
	"code/mission/mission.cpp"
	"code/mission/missionmanager.cpp"
	"code/mission/missionscriptloader.cpp"
	"code/mission/missionstage.cpp"
	"code/mission/nocopbonusobjective.cpp"
	"code/mission/nodamagebonusobjective.cpp"
	"code/mission/objectives/buycarobjective.cpp"
	"code/mission/objectives/buyskinobjective.cpp"
	"code/mission/objectives/coinobjective.cpp"
	"code/mission/objectives/collectdumpedobjective.cpp"
	"code/mission/objectives/collectibleobjective.cpp"
	"code/mission/objectives/deliveryobjective.cpp"
	"code/mission/objectives/destroybossobjective.cpp"
	"code/mission/objectives/destroyobjective.cpp"
	"code/mission/objectives/dialogueobjective.cpp"
	"code/mission/objectives/fmvobjective.cpp"
	"code/mission/objectives/followobjective.cpp"
	"code/mission/objectives/getinobjective.cpp"
	"code/mission/objectives/gooutsideobjective.cpp"
	"code/mission/objectives/gotoobjective.cpp"
	"code/mission/objectives/interiorobjective.cpp"
	"code/mission/objectives/loadvehicleobjective.cpp"
	"code/mission/objectives/loseobjective.cpp"
	"code/mission/objectives/missionobjective.cpp"
	"code/mission/objectives/pickupitemobjective.cpp"
	"code/mission/objectives/raceobjective.cpp"
	"code/mission/objectives/talktoobjective.cpp"
	"code/mission/objectives/timerobjective.cpp"
	"code/mission/racepositionbonusobjective.cpp"
	"code/mission/respawnmanager/respawnentity.cpp"
	"code/mission/respawnmanager/respawnmanager.cpp"
	"code/mission/rewards/merchandise.cpp"
	"code/mission/rewards/reward.cpp"
	"code/mission/rewards/rewardsmanager.cpp"
	"code/mission/safezone/safezone.cpp"
	"code/mission/statepropcollectible.cpp"
	"code/mission/timeremainbonusobjective.cpp"
	"code/mission/ufo/tractorbeam.cpp"
	"code/mission/ufo/ufo.cpp"
	"code/pedpaths/path.cpp"
	"code/pedpaths/pathmanager.cpp"
	"code/pedpaths/pathsegment.cpp"
	"code/presentation/animplayer.cpp"
	"code/presentation/blinker.cpp"
	"code/presentation/cameraplayer.cpp"
	"code/presentation/fmvplayer/fmvplayer.cpp"
	"code/presentation/fmvplayer/fmvuserinputhandler.cpp"
	"code/presentation/gui/backend/guimanagerbackend.cpp"
	"code/presentation/gui/backend/guiscreendemo.cpp"
	"code/presentation/gui/backend/guiscreenloading.cpp"
	"code/presentation/gui/backend/guiscreenloadingfe.cpp"
	"code/presentation/gui/bootup/guimanagerbootup.cpp"
	"code/presentation/gui/bootup/guimanagerlanguage.cpp"
	"code/presentation/gui/bootup/guiscreenbootupload.cpp"
	"code/presentation/gui/bootup/guiscreenlanguage.cpp"
	"code/presentation/gui/bootup/guiscreenlicense.cpp"
	"code/presentation/gui/frontend/guimanagerfrontend.cpp"
	"code/presentation/gui/frontend/guiscreencardgallery.cpp"
	"code/presentation/gui/frontend/guiscreencontroller.cpp"
	#"code/presentation/gui/frontend/guiscreencontrollerWin32.cpp"
	#"code/presentation/gui/frontend/guiscreendisplay.cpp"
	"code/presentation/gui/frontend/guiscreenloadgame.cpp"
	"code/presentation/gui/frontend/guiscreenmainmenu.cpp"
	"code/presentation/gui/frontend/guiscreenmissiongallery.cpp"
	#"code/presentation/gui/frontend/guiscreenmultichoosechar.cpp"
	#"code/presentation/gui/frontend/guiscreenmultisetup.cpp"
	"code/presentation/gui/frontend/guiscreenoptions.cpp"
	"code/presentation/gui/frontend/guiscreenplaymovie.cpp"
	"code/presentation/gui/frontend/guiscreenscrapbookcontents.cpp"
	"code/presentation/gui/frontend/guiscreenscrapbook.cpp"
	"code/presentation/gui/frontend/guiscreenscrapbookstats.cpp"
	"code/presentation/gui/frontend/guiscreenskingallery.cpp"
	"code/presentation/gui/frontend/guiscreensound.cpp"
	"code/presentation/gui/frontend/guiscreensplash.cpp"
	"code/presentation/gui/frontend/guiscreenvehiclegallery.cpp"
	"code/presentation/gui/frontend/guiscreenviewcredits.cpp"
	"code/presentation/gui/frontend/guiscreenviewmovies.cpp"
	"code/presentation/gui/guientity.cpp"
	"code/presentation/gui/guimanager.cpp"
	"code/presentation/gui/guimenu.cpp"
	"code/presentation/gui/guimenuitem.cpp"
	"code/presentation/gui/guiscreen.cpp"
	"code/presentation/gui/guiscreenmemcardcheck.cpp"
	"code/presentation/gui/guiscreenmemorycard.cpp"
	"code/presentation/gui/guiscreenmessage.cpp"
	"code/presentation/gui/guiscreenprompt.cpp"
	"code/presentation/gui/guisystem.cpp"
	"code/presentation/gui/guitextbible.cpp"
	"code/presentation/gui/guiuserinputhandler.cpp"
	"code/presentation/gui/guiwindow.cpp"
	"code/presentation/gui/ingame/guimanageringame.cpp"
	"code/presentation/gui/ingame/guiscreencreditspostfmv.cpp"
	"code/presentation/gui/ingame/guiscreenhastransitions.cpp"
	"code/presentation/gui/ingame/guiscreenhud.cpp"
	#"code/presentation/gui/ingame/guiscreenhudmap.cpp"
	"code/presentation/gui/ingame/guiscreeniriswipe.cpp"
	"code/presentation/gui/ingame/guiscreenletterbox.cpp"
	"code/presentation/gui/ingame/guiscreenlevelend.cpp"
	"code/presentation/gui/ingame/guiscreenlevelstats.cpp"
	"code/presentation/gui/ingame/guiscreenmissionbase.cpp"
	"code/presentation/gui/ingame/guiscreenmissionload.cpp"
	"code/presentation/gui/ingame/guiscreenmissionover.cpp"
	"code/presentation/gui/ingame/guiscreenmissionselect.cpp"
	"code/presentation/gui/ingame/guiscreenmissionsuccess.cpp"
	"code/presentation/gui/ingame/guiscreenmultihud.cpp"
	"code/presentation/gui/ingame/guiscreenpausecontroller.cpp"
	"code/presentation/gui/ingame/guiscreenpause.cpp"
	#"code/presentation/gui/ingame/guiscreenpausedisplay.cpp"
	"code/presentation/gui/ingame/guiscreenpausemission.cpp"
	"code/presentation/gui/ingame/guiscreenpauseoptions.cpp"
	"code/presentation/gui/ingame/guiscreenpausesettings.cpp"
	"code/presentation/gui/ingame/guiscreenpausesound.cpp"
	"code/presentation/gui/ingame/guiscreenpausesunday.cpp"
	"code/presentation/gui/ingame/guiscreenphonebooth.cpp"
	"code/presentation/gui/ingame/guiscreenpurchaserewards.cpp"
	"code/presentation/gui/ingame/guiscreenrewards.cpp"
	"code/presentation/gui/ingame/guiscreensavegame.cpp"
	"code/presentation/gui/ingame/guiscreentutorial.cpp"
	"code/presentation/gui/ingame/guiscreenviewcards.cpp"
	"code/presentation/gui/ingame/hudevents/hudcardcollected.cpp"
	"code/presentation/gui/ingame/hudevents/hudcoincollected.cpp"
	"code/presentation/gui/ingame/hudevents/hudcountdown.cpp"
	"code/presentation/gui/ingame/hudevents/hudeventhandler.cpp"
	"code/presentation/gui/ingame/hudevents/hudhitnrun.cpp"
	"code/presentation/gui/ingame/hudevents/huditemdropped.cpp"
	"code/presentation/gui/ingame/hudevents/hudmissionobjective.cpp"
	"code/presentation/gui/ingame/hudevents/hudmissionprogress.cpp"
	"code/presentation/gui/ingame/hudevents/hudwaspdestroyed.cpp"
	"code/presentation/gui/minigame/guimanagerminigame.cpp"
	"code/presentation/gui/minigame/guiscreenminihud.cpp"
	"code/presentation/gui/minigame/guiscreenminimenu.cpp"
	"code/presentation/gui/minigame/guiscreenminipause.cpp"
	"code/presentation/gui/minigame/guiscreenminisummary.cpp"
	"code/presentation/gui/utility/hudmapcam.cpp"
	"code/presentation/gui/utility/hudmap.cpp"
	"code/presentation/gui/utility/numerictext.cpp"
	"code/presentation/gui/utility/scrollingtext.cpp"
	"code/presentation/gui/utility/slider.cpp"
	"code/presentation/gui/utility/specialfx.cpp"
	"code/presentation/gui/utility/teletypetext.cpp"
	"code/presentation/gui/utility/transitions.cpp"
	"code/presentation/language.cpp"
	"code/presentation/mouthflapper.cpp"
	"code/presentation/nisplayer.cpp"
	"code/presentation/playerdrawable.cpp"
	"code/presentation/presentationanimator.cpp"
	"code/presentation/presentation.cpp"
	"code/presentation/presevents/fmvevent.cpp"
	"code/presentation/presevents/nisevent.cpp"
	"code/presentation/presevents/presentationevent.cpp"
	"code/presentation/presevents/transevent.cpp"
	"code/presentation/simpleanimationplayer.cpp"
	"code/presentation/transitionplayer.cpp"
	"code/presentation/tutorialmanager.cpp"
	"code/render/animentitydsgmanager/animentitydsgmanager.cpp"
	"code/render/breakables/breakablesmanager.cpp"
	"code/render/Culling/BoxPts.cpp"
	"code/render/Culling/CellBlock.cpp"
	"code/render/Culling/Cell.cpp"
	"code/render/Culling/CoordSubList.cpp"
	"code/render/Culling/CullData.cpp"
	"code/render/Culling/HexahedronP.cpp"
	"code/render/Culling/ISpatialProxy.cpp"
	"code/render/Culling/OctTreeData.cpp"
	"code/render/Culling/OctTreeNode.cpp"
	"code/render/Culling/SpatialTree.cpp"
	"code/render/Culling/SpatialTreeIter.cpp"
	"code/render/Culling/SphereSP.cpp"
	"code/render/Culling/VectorLib.cpp"
	"code/render/Culling/WorldScene.cpp"
	"code/render/DSG/animcollisionentitydsg.cpp"
	"code/render/DSG/animentitydsg.cpp"
	"code/render/DSG/breakableobjectdsg.cpp"
	"code/render/DSG/collisionentitydsg.cpp"
	"code/render/DSG/DSGFactory.cpp"
	"code/render/DSG/DynaPhysDSG.cpp"
	"code/render/DSG/FenceEntityDSG.cpp"
	"code/render/DSG/IEntityDSG.cpp"
	"code/render/DSG/InstAnimDynaPhysDSG.cpp"
	"code/render/DSG/InstDynaPhysDSG.cpp"
	"code/render/DSG/InstStatEntityDSG.cpp"
	"code/render/DSG/InstStatPhysDSG.cpp"
	"code/render/DSG/IntersectDSG.cpp"
	"code/render/DSG/LensFlareDSG.cpp"
	"code/render/DSG/StatePropDSG.cpp"
	"code/render/DSG/StaticEntityDSG.cpp"
	"code/render/DSG/StaticPhysDSG.cpp"
	"code/render/DSG/TriStripDSG.cpp"
	"code/render/DSG/WorldSphereDSG.cpp"
	"code/render/IntersectManager/IntersectManager.cpp"
	"code/render/Loaders/AllWrappers.cpp"
	"code/render/Loaders/AnimCollLoader.cpp"
	"code/render/Loaders/AnimDSGLoader.cpp"
	"code/render/Loaders/AnimDynaPhysLoader.cpp"
	"code/render/Loaders/BillboardWrappedLoader.cpp"
	"code/render/Loaders/breakableobjectloader.cpp"
	"code/render/Loaders/DynaPhysLoader.cpp"
	"code/render/Loaders/FenceLoader.cpp"
	"code/render/Loaders/GeometryWrappedLoader.cpp"
	"code/render/Loaders/instparticlesystemloader.cpp"
	"code/render/Loaders/InstStatEntityLoader.cpp"
	"code/render/Loaders/InstStatPhysLoader.cpp"
	"code/render/Loaders/IntersectLoader.cpp"
	"code/render/Loaders/LensFlareLoader.cpp"
	"code/render/Loaders/StaticEntityLoader.cpp"
	"code/render/Loaders/StaticPhysLoader.cpp"
	"code/render/Loaders/TreeDSGLoader.cpp"
	"code/render/Loaders/WorldSphereLoader.cpp"
	"code/render/Particles/particlemanager.cpp"
	"code/render/Particles/particlesystemdsg.cpp"
	"code/render/Particles/vehicleparticleemitter.cpp"
	"code/render/RenderFlow/renderflow.cpp"
	"code/render/RenderManager/FrontEndRenderLayer.cpp"
	"code/render/RenderManager/RenderLayer.cpp"
	"code/render/RenderManager/RenderManager.cpp"
	"code/render/RenderManager/WorldRenderLayer.cpp"
	"code/roads/geometry.cpp"
	"code/roads/intersection.cpp"
	"code/roads/lane.cpp"
	"code/roads/road.cpp"
	"code/roads/roadmanager.cpp"
	"code/roads/roadrender.cpp"
	"code/roads/roadrendertest.cpp"
	"code/roads/roadsegment.cpp"
	"code/roads/roadsegmentdata.cpp"
	"code/roads/trafficcontrol.cpp"
	"code/sound/avatar/avatarsoundplayer.cpp"
	"code/sound/avatar/carsoundparameters.cpp"
	"code/sound/avatar/soundavatar.cpp"
	"code/sound/avatar/vehiclesounddebugpage.cpp"
	"code/sound/avatar/vehiclesoundplayer.cpp"
	"code/sound/dialog/conversation.cpp"
	"code/sound/dialog/conversationmatcher.cpp"
	"code/sound/dialog/dialogcoordinator.cpp"
	"code/sound/dialog/dialogline.cpp"
	"code/sound/dialog/dialoglist.cpp"
	"code/sound/dialog/dialogpriorityqueue.cpp"
	"code/sound/dialog/dialogqueueelement.cpp"
	"code/sound/dialog/dialogselectiongroup.cpp"
	"code/sound/dialog/dialogsounddebugpage.cpp"
	"code/sound/dialog/playabledialog.cpp"
	"code/sound/dialog/selectabledialog.cpp"
	"code/sound/listener.cpp"
	"code/sound/movingpositional/actorplayer.cpp"
	"code/sound/movingpositional/aivehiclesoundplayer.cpp"
	"code/sound/movingpositional/animobjsoundplayer.cpp"
	"code/sound/movingpositional/avatarvehicleposnplayer.cpp"
	"code/sound/movingpositional/movingsoundmanager.cpp"
	"code/sound/movingpositional/platformsoundplayer.cpp"
	"code/sound/movingpositional/trafficsoundplayer.cpp"
	"code/sound/movingpositional/vehicleposnsoundplayer.cpp"
	"code/sound/movingpositional/waspsoundplayer.cpp"
	"code/sound/music/musicplayer.cpp"
	"code/sound/nis/nissoundplayer.cpp"
	"code/sound/positionalsoundplayer.cpp"
	"code/sound/simpsonssoundplayer.cpp"
	"code/sound/soundcluster.cpp"
	"code/sound/sounddebug/sounddebugdisplay.cpp"
	"code/sound/sounddebug/sounddebugpage.cpp"
	"code/sound/soundfx/positionalsoundsettings.cpp"
	"code/sound/soundfx/reverbcontroller.cpp"
	"code/sound/soundfx/reverbsettings.cpp"
	"code/sound/soundfx/soundeffectplayer.cpp"
	"code/sound/soundfx/soundfxfrontendlogic.cpp"
	"code/sound/soundfx/soundfxgameplaylogic.cpp"
	"code/sound/soundfx/soundfxlogic.cpp"
	"code/sound/soundfx/soundfxpauselogic.cpp"
	"code/sound/soundfx/win32reverbcontroller.cpp"
	"code/sound/soundloader.cpp"
	"code/sound/soundmanager.cpp"
	"code/sound/soundrenderercallback.cpp"
	"code/sound/soundrenderer/dasoundplayer.cpp"
	"code/sound/soundrenderer/fader.cpp"
	"code/sound/soundrenderer/musicsoundplayer.cpp"
	"code/sound/soundrenderer/playermanager.cpp"
	"code/sound/soundrenderer/scripts/apu.cpp"
	"code/sound/soundrenderer/scripts/bart.cpp"
	"code/sound/soundrenderer/scripts/cars.cpp"
	"code/sound/soundrenderer/scripts/effects.cpp"
	"code/sound/soundrenderer/scripts/english.cpp"
	"code/sound/soundrenderer/scripts/homer.cpp"
	"code/sound/soundrenderer/scripts/levels.cpp"
	"code/sound/soundrenderer/scripts/lisa.cpp"
	"code/sound/soundrenderer/scripts/marge.cpp"
	"code/sound/soundrenderer/scripts/tuning.cpp"
	"code/sound/soundrenderer/soundallocatedresource.cpp"
	"code/sound/soundrenderer/sounddynaload.cpp"
	"code/sound/soundrenderer/soundnucleus.cpp"
	"code/sound/soundrenderer/soundrenderingmanager.cpp"
	"code/sound/soundrenderer/soundresource.cpp"
	"code/sound/soundrenderer/soundresourcemanager.cpp"
	"code/sound/soundrenderer/soundtuner.cpp"
	"code/sound/soundrenderer/tunerdebugpage.cpp"
	"code/sound/soundrenderer/wireplayers.cpp"
	"code/sound/soundrenderer/wiresystem.cpp"
	"code/sound/tuning/globalsettings.cpp"
	"code/stateprop/stateprop.cpp"
	"code/stateprop/statepropdata.cpp"
	"code/supersprint/supersprintdata.cpp"
	"code/supersprint/supersprintmanager.cpp"
	"code/worldsim/avatar.cpp"
	"code/worldsim/avatarmanager.cpp"
	"code/worldsim/character/aicharactercontroller.cpp"
	"code/worldsim/character/charactercontroller.cpp"
	"code/worldsim/character/character.cpp"
	"code/worldsim/character/charactermanager.cpp"
	"code/worldsim/character/charactermappable.cpp"
	"code/worldsim/character/characterrenderable.cpp"
	"code/worldsim/character/charactertarget.cpp"
	"code/worldsim/character/footprint/footprintmanager.cpp"
	"code/worldsim/coins/coinmanager.cpp"
	"code/worldsim/coins/sparkle.cpp"
	"code/worldsim/groundplanepool.cpp"
	"code/worldsim/harass/chasemanager.cpp"
	"code/worldsim/hitnrunmanager.cpp"
	"code/worldsim/huskpool.cpp"
	"code/worldsim/parkedcars/parkedcarmanager.cpp"
	"code/worldsim/ped/pedestrian.cpp"
	"code/worldsim/ped/pedestrianmanager.cpp"
	"code/worldsim/redbrick/geometryvehicle.cpp"
	"code/worldsim/redbrick/physicslocomotioncontrollerforces.cpp"
	"code/worldsim/redbrick/physicslocomotion.cpp"
	"code/worldsim/redbrick/redbrickcollisionsolveragent.cpp"
	"code/worldsim/redbrick/rootmatrixdriver.cpp"
	"code/worldsim/redbrick/suspensionjointdriver.cpp"
	"code/worldsim/redbrick/trafficbodydrawable.cpp"
	"code/worldsim/redbrick/trafficlocomotion.cpp"
	"code/worldsim/redbrick/vehiclecontroller/aivehiclecontroller.cpp"
	"code/worldsim/redbrick/vehiclecontroller/humanvehiclecontroller.cpp"
	"code/worldsim/redbrick/vehiclecontroller/vehiclecontroller.cpp"
	"code/worldsim/redbrick/vehiclecontroller/vehiclemappable.cpp"
	"code/worldsim/redbrick/vehicle.cpp"
	"code/worldsim/redbrick/vehicleeventlistener.cpp"
	"code/worldsim/redbrick/vehicleinit.cpp"
	"code/worldsim/redbrick/vehiclelocomotion.cpp"
	"code/worldsim/redbrick/wheel.cpp"
	"code/worldsim/skidmarks/skidmark.cpp"
	"code/worldsim/skidmarks/SkidMarkGenerator.cpp"
	"code/worldsim/skidmarks/skidmarkmanager.cpp"
	"code/worldsim/spawn/spawnmanager.cpp"
	"code/worldsim/traffic/trafficmanager.cpp"
	"code/worldsim/vehiclecentral.cpp"
	"code/worldsim/worldcollisionsolveragent.cpp"
	"code/worldsim/worldobject.cpp"
	"code/worldsim/worldphysicsmanager.cpp"
)
if(SRR2_PAL)
	list(APPEND SRR2_SOURCES
		"code/sound/soundrenderer/scripts/french.cpp"
		"code/sound/soundrenderer/scripts/german.cpp"
		"code/sound/soundrenderer/scripts/spanish.cpp"
	)
endif()

add_executable(${PROJECT_NAME} ${SRR2_SOURCES})
target_link_libraries(${PROJECT_NAME}
	${SDL_LIBRARY}
	choreo
	p3d
	pddi
	poser
	radcore
	radcontent
	radmath
	radmovie
	radmusic
	radsound
	radscript
	scrooby
	sim
	Threads::Threads
)
if(SRR2_MICROPROFILE AND WIN32)
	# needed for microprofile
	target_link_libraries(${PROJECT_NAME} wsock32)
endif()
if(VITA)
	if(SRR2_USE_VITAGL)
		target_link_libraries(${PROJECT_NAME}
			vitaGL
			vitashark
		)
	endif()
	target_link_libraries(${PROJECT_NAME}
		SceLibKernel_stub
		ScePvf_stub
		mathneon
		SceAudio_stub
		SceAppMgr_stub
		SceAppUtil_stub
		ScePgf_stub
		SceShaccCgExt
		SceShaccCg_stub
		SceCtrl_stub
		taihen_stub
		SceKernelDmacMgr_stub
		SceRazorCapture_stub_weak
	)
endif()
if(SRR2_USE_BINK)
	target_link_libraries(${PROJECT_NAME} bink)
else()
	target_link_libraries(${PROJECT_NAME} PkgConfig::FFmpeg)
endif()
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

option(SRR2_USE_PCH ON)
if (SRR2_USE_PCH)
	target_precompile_headers(${PROJECT_NAME} PRIVATE "code/main/pchsrr2.h")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "code/")

if(VITA)
	## Create Vita files
	if(SRR2_P3D_PDDI STREQUAL "GLES2" AND NOT SRR2_USE_VITAGL)
		vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
		# The FILE directive lets you add additional files to the VPK, the syntax is
		# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
		vita_create_vpk(${SRR2_FILENAME}${SRR2_REGION}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
			VERSION ${VITA_VERSION}
			NAME ${VITA_APP_NAME}
			FILE sce_sys/icon0.png sce_sys/icon0.png
			sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
			sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
			sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
			art art

			#suprx
			module/libGLESv2.suprx module/libGLESv2.suprx
			module/libgpu_es4_ext.suprx module/libgpu_es4_ext.suprx
			module/libIMGEGL.suprx module/libIMGEGL.suprx
			module/libpvrPSP2_WSEGL.suprx module/libpvrPSP2_WSEGL.suprx
		)
	else()
		vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
		# The FILE directive lets you add additional files to the VPK, the syntax is
		# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
		vita_create_vpk(${SRR2_FILENAME}${SRR2_REGION}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
			VERSION ${VITA_VERSION}
			NAME ${VITA_APP_NAME}
			FILE sce_sys/icon0.png sce_sys/icon0.png
			sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
			sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
			sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
			art art

			#gxp
			libs/pure3d/pddi/gxm/shaders/vertex_cg.gxp shaders/vertex_cg.gxp
			libs/pure3d/pddi/gxm/shaders/color_cg.gxp shaders/color_cg.gxp
			libs/pure3d/pddi/gxm/shaders/texture_cg.gxp shaders/texture_cg.gxp
			libs/pure3d/pddi/gxm/shaders/alpha_cg.gxp shaders/alpha_cg.gxp
		)
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
	nx_generate_nacp(OUTPUT ${PROJECT_NAME}.nacp
		NAME ${SRR2_TITLE}
		AUTHOR "Radical Entertainment"
		VERSION ${SRR2_VERSION}
	)
	nx_create_nro(${PROJECT_NAME}
		OUTPUT ${SRR2_FILENAME}${SRR2_REGION}.nro
		ICON ${PROJECT_NAME}.jpg
		NACP ${PROJECT_NAME}.nacp
		ROMFS romfs
	)
endif()
