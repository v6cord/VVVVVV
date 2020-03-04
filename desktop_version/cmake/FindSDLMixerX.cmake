# - Try to find SDLMixerX
# Once done this will define
#  SDLMIXERX_FOUND - System has SDLMixerX
#  SDLMIXERX_INCLUDE_DIRS - The SDLMixerX include directories
#  SDLMIXERX_LIBRARIES - The libraries needed to use SDLMixerX
#  SDLMIXERX_DEFINITIONS - Compiler switches required for using SDLMixerX

find_package(PkgConfig)
pkg_check_modules(PC_SDLMIXERX QUIET SDL2_mixer_ext)
set(SDLMIXERX_DEFINITIONS ${PC_SDLMIXERX_CFLAGS_OTHER})

find_path(SDLMIXERX_INCLUDE_DIR SDL2/sdl_mixer_ext.h
          HINTS ${PC_SDLMIXERX_INCLUDEDIR} ${PC_SDLMIXERX_INCLUDE_DIRS}
          PATH_SUFFIXES libxml2 )

find_library(SDLMIXERX_LIBRARY NAMES SDL2_mixer_ext libSDL2_mixer_ext
             HINTS ${PC_SDLMIXERX_LIBDIR} ${PC_SDLMIXERX_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SDLMIXERX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SDLMixerX  DEFAULT_MSG
                                  SDLMIXERX_LIBRARY SDLMIXERX_INCLUDE_DIR)

mark_as_advanced(SDLMIXERX_INCLUDE_DIR SDLMIXERX_LIBRARY )

set(SDLMIXERX_LIBRARIES ${SDLMIXERX_LIBRARY} )
set(SDLMIXERX_INCLUDE_DIRS ${SDLMIXERX_INCLUDE_DIR} )

