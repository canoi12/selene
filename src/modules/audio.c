#ifndef SELENE_NO_AUDIO
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"
#include "dr_mp3.h"
#include "dr_flac.h"

struct AudioDecoder {
    char format;
    union {
        drwav wav;
        drmp3 mp3;
        drflac* flac;
        stb_vorbis* ogg;
    };
    AudioInfo info;
#if defined(OS_ANDROID)
    void* audio_data;
#endif
};

static MODULE_FUNCTION(AudioDecoder, close) {
    CHECK_META(AudioDecoder);
    switch (self->format) {
        case SELENE_WAV_FORMAT: {
           drwav_uninit(&(self->wav));
        }
        break;
        case SELENE_OGG_FORMAT: {
            stb_vorbis_close(self->ogg);
        }
        break;
        case SELENE_MP3_FORMAT: {
           drmp3_uninit(&(self->mp3));
        }
        break;
        case SELENE_FLAC_FORMAT: {
           drflac_close(self->flac);
        }
        break;
        default: break;
    }
    return 0;
}

static MODULE_FUNCTION(AudioDecoder, clone) {
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, seek) {
    CHECK_META(AudioDecoder);
    CHECK_INTEGER(index);
    switch (self->format) {
        case SELENE_WAV_FORMAT: {
            drwav_seek_to_pcm_frame(&(self->wav), index);
        }
        break;
        case SELENE_OGG_FORMAT: {
            stb_vorbis_seek(self->ogg, index);
        }
        break;
        case SELENE_MP3_FORMAT: {
            drmp3_seek_to_pcm_frame(&(self->mp3), index);
        }
        break;
        case SELENE_FLAC_FORMAT: {
            drflac_seek_to_pcm_frame(self->flac, index);
        }
        break;
        default: break;
    }
    return 0;
}

static MODULE_FUNCTION(AudioDecoder, decode_data) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, readS16) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data[0]);
    int frame_count;
    switch (self->format) {
        case SELENE_WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, (drwav_int16*)(&data[1]));
        }
        break;
        case SELENE_OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, (short*)(&data[1]), len);
        }
        break;
        case SELENE_MP3_FORMAT: {
            frame_count = drmp3_read_pcm_frames_s16(&(self->mp3), len, (drmp3_int16*)(&data[1]));
        }
        break;
        case SELENE_FLAC_FORMAT: {
            frame_count = drflac_read_pcm_frames_s16(self->flac, len, (drflac_int16*)(&data[1]));
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, readF32) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data[0]);
    int frame_count;
    switch (self->format) {
        case SELENE_WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_f32(&(self->wav), len, (float*)(&data[1]));
        }
        break;
        case SELENE_OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_float_interleaved(self->ogg, self->info.channels, (float*)(&data[1]), len);
        }
        break;
        case SELENE_MP3_FORMAT: {
            frame_count = drmp3_read_pcm_frames_f32(&(self->mp3), len, (float*)(&data[1]));
        }
        break;
        case SELENE_FLAC_FORMAT: {
            frame_count = drflac_read_pcm_frames_f32(self->flac, len, (float*)(&data[1]));
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, get_chunk) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data[0]);
    int frame_count;
    switch (self->format) {
        case SELENE_WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, (drwav_int16*)(&data[1]));
        }
        break;
        case SELENE_OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, (short*)(&data[1]), len);
        }
        break;
        case SELENE_MP3_FORMAT: {
            frame_count = drmp3_read_pcm_frames_s16(&(self->mp3), len, (drmp3_int16*)(&data[1]));
        }
        break;
        case SELENE_FLAC_FORMAT: {
            frame_count = drflac_read_pcm_frames_s16(self->flac, len, (drflac_int16*)(&data[1]));
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, get_sample_rate) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.sample_rate);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, get_channels) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.channels);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, get_bit_depth) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.bit_depth);
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, getFrameCount) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.frame_count);
    return 1;
}

static int l_AudioDecoder_meta(lua_State* LUA_STATE_NAME) {
    BEGIN_REG(reg)
        REG_FIELD(AudioDecoder, close),
        REG_FIELD(AudioDecoder, seek),
        REG_FIELD(AudioDecoder, decode_data),
        REG_FIELD(AudioDecoder, get_chunk),
        REG_FIELD(AudioDecoder, get_sample_rate),
        REG_FIELD(AudioDecoder, get_channels),
        REG_FIELD(AudioDecoder, get_bit_depth),
    END_REG()
    luaL_newmetatable(L, "AudioDecoder");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    // NEW_META(AudioDecoder, reg, index_reg);
    return 1;
}

/**
 * Load an audio decoder from a file (ogg, wav, mp3 and flac)
 */
static MODULE_FUNCTION(audio, load_decoder) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);
    char* p = (char*)path + len;
    while (*p != '.')
        p--;
    int format = SELENE_UNKNOWN_AUDIO_FORMAT;
    if (!strcmp(p, ".ogg"))
        format = SELENE_OGG_FORMAT;
    else if (!strcmp(p ,".wav"))
        format = SELENE_WAV_FORMAT;
    else if (!strcmp(p ,".mp3"))
        format = SELENE_MP3_FORMAT;
    else if (!strcmp(p ,".flac"))
        format = SELENE_FLAC_FORMAT;
    else
        return luaL_error(L, "Unsupported audio format: %s\n", path);

    char* data = NULL;
    size_t data_size;
#if defined(OS_ANDROID)
    SDL_RWops* fp = SDL_RWFromFile(path, "rb");
    data_size = SDL_RWsize(fp);
    data = (char*)malloc(data_size);
    SDL_RWread(fp, data, 1, data_size);
    SDL_RWclose(fp);
#endif
    NEW_UDATA(AudioDecoder, dec);
    dec->format = format;
    switch (format) {
        case SELENE_WAV_FORMAT: {
#if !defined(OS_ANDROID)
            int success = drwav_init_file(&(dec->wav), path, NULL);
#else
            int success = drwav_init_memory(&(dec->wav), (const void*)data, data_size, NULL);
#endif
            if (!success)
                return luaL_error(L, "Failed to load wav: %s\n", path);
            dec->info.bit_depth = dec->wav.bitsPerSample;
            dec->info.sample_rate = dec->wav.sampleRate;
            dec->info.channels = dec->wav.channels;
            dec->info.size = dec->wav.dataChunkDataSize;
            dec->info.frame_count = dec->wav.totalPCMFrameCount;
        }
        break;
        case SELENE_OGG_FORMAT: {
#if !defined(OS_ANDROID)
            stb_vorbis* ogg = stb_vorbis_open_filename(path, NULL, NULL);
#else
            stb_vorbis* ogg = stb_vorbis_open_memory(data, data_size, NULL, NULL);
#endif
            if (ogg == NULL)
                return luaL_error(L, "Failed to load ogg: %s\n", path);
            stb_vorbis_info info = stb_vorbis_get_info(ogg);
            dec->info.sample_rate = info.sample_rate;
            dec->info.channels = info.channels;
            dec->ogg = ogg;
        }
        break;
        case SELENE_MP3_FORMAT: {
#if !defined(OS_ANDROID)
            int success = drmp3_init_file(&(dec->mp3), path, NULL);
#else
            int success = drmp3_init_memory(&(dec->mp3), data, data_size, NULL);
#endif
            if (!success)
                return luaL_error(L, "Failed to load mp3: %s\n", path);
            // dec->info.bit_depth = dec->mp3.bit;
            dec->info.sample_rate = dec->mp3.sampleRate;
            dec->info.channels = dec->mp3.channels;
            dec->info.size = dec->mp3.dataSize;
            dec->info.frame_count = dec->mp3.pcmFramesRemainingInMP3Frame;
        }
        break;
        case SELENE_FLAC_FORMAT: {
#if !defined(OS_ANDROID)
            dec->flac = drflac_open_file(path, NULL);
#else
            dec->flac = drflac_open_memory(data, data_size, NULL);
#endif
            if (dec->flac == NULL)
                return luaL_error(L, "Failed to load flac: %s\n", path);
            dec->info.bit_depth = dec->flac->bitsPerSample;
            dec->info.sample_rate = dec->flac->sampleRate;
            dec->info.channels = dec->flac->channels;
            dec->info.size = dec->flac->maxBlockSizeInPCMFrames;
            dec->info.frame_count = dec->flac->totalPCMFrameCount;
        }
        break;
        default: break;
    }
#if defined(OS_ANDROID)
    dec->audio_data = data;
#endif
    return 1;
}

int luaopen_audio(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(audio, load_decoder),
    END_REG()
    luaL_newlib(L, reg);
    l_AudioDecoder_meta(L);
    lua_setfield(L, -2, "AudioDecoder");
    return 1;
}

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#endif /* SELENE_NO_AUDIO */
