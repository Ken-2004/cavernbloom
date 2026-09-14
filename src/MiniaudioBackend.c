// Compile the fetched Ogg decoder and playback library in one private target.
#define STB_VORBIS_HEADER_ONLY
#include <extras/stb_vorbis.c>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#undef STB_VORBIS_HEADER_ONLY
#include <extras/stb_vorbis.c>
