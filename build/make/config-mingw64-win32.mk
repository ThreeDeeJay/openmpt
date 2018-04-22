
CC  = i686-w64-mingw32-gcc$(MINGW_FLAVOUR)
CXX = i686-w64-mingw32-g++$(MINGW_FLAVOUR)
LD  = i686-w64-mingw32-g++$(MINGW_FLAVOUR)
AR  = i686-w64-mingw32-ar$(MINGW_FLAVOUR)

CPPFLAGS += -DWIN32 -D_WIN32 -Iinclude/mingw-std-threads -DMPT_WITH_MINGWSTDTHREADS
CXXFLAGS += -std=c++11 -municode -mconsole
CFLAGS   += -std=c99   -municode -mconsole
LDFLAGS  +=
LDLIBS   += -lm -lrpcrt4 -lwinmm
ARFLAGS  := rcs

CFLAGS_SILENT += -Wno-unused-parameter -Wno-unused-function -Wno-cast-qual -Wno-old-style-declaration -Wno-type-limits -Wno-unused-but-set-variable

EXESUFFIX=.exe
SOSUFFIX=.dll
SOSUFFIXWINDOWS=1

DYNLINK=0
SHARED_LIB=1
STATIC_LIB=0
SHARED_SONAME=0

NO_ZLIB=1
NO_LTDL=1
NO_DL=1
NO_MPG123=1
NO_OGG=1
NO_VORBIS=1
NO_VORBISFILE=1
NO_PORTAUDIO=1
NO_PORTAUDIOCPP=1
NO_PULSEAUDIO=1
NO_SDL=1
NO_SDL2=1
NO_SNDFILE=1
NO_FLAC=1
