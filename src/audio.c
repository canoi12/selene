#include "selene.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

static int l_audio_engine;

static void _audio_callback(void* userdata, Uint8* stream, int len) {
    lua_State* L = (lua_State*)userdata;
}

static BEGIN_FUNCTION(audio, GetCallback)
    PUSH_LUDATA(_audio_callback);
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadOgg)
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadWav)
END_FUNCTION(1)


#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"
