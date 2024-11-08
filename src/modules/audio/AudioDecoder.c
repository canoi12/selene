#include "AudioDecoder.h"
#include "lua_helper.h"

static MODULE_FUNCTION(AudioDecoder, new) {
    INIT_ARG();
    CHECK_STRING(path);
    return 1;
}

/**
 * Load an audio decoder from a file (ogg, wav, mp3 and flac)
 */
static MODULE_FUNCTION(AudioDecoder, load) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);
    char* p = (char*)path + len;
    while (*p != '.')
        p--;
    int format = UNKNOWN_FORMAT;
    if (!strcmp(p, ".ogg"))
        format = OGG_FORMAT;
    else if (!strcmp(p ,".wav"))
        format = WAV_FORMAT;
    else if (!strcmp(p ,".mp3"))
        format = MP3_FORMAT;
    else if (!strcmp(p ,".flac"))
        format = FLAC_FORMAT;
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
        case WAV_FORMAT: {
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
        case OGG_FORMAT: {
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
        case MP3_FORMAT: {
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
        case FLAC_FORMAT: {
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

static META_FUNCTION(AudioDecoder, close) {
    CHECK_META(AudioDecoder);
    switch (self->format) {
        case WAV_FORMAT: {
           drwav_uninit(&(self->wav));
        }
        break;
        case OGG_FORMAT: {
            stb_vorbis_close(self->ogg);
        }
        break;
        case MP3_FORMAT: {
           drmp3_uninit(&(self->mp3));
        }
        break;
        case FLAC_FORMAT: {
           drflac_close(self->flac);
        }
        break;
        default: break;
    }
    return 0;
}

static META_FUNCTION(AudioDecoder, clone) {
    return 1;
}

static META_FUNCTION(AudioDecoder, seek) {
    CHECK_META(AudioDecoder);
    CHECK_INTEGER(index);
    switch (self->format) {
        case WAV_FORMAT: {
            drwav_seek_to_pcm_frame(&(self->wav), index);
        }
        break;
        case OGG_FORMAT: {
            stb_vorbis_seek(self->ogg, index);
        }
        break;
        case MP3_FORMAT: {
            drmp3_seek_to_pcm_frame(&(self->mp3), index);
        }
        break;
        case FLAC_FORMAT: {
            drflac_seek_to_pcm_frame(self->flac, index);
        }
        break;
        default: break;
    }
    return 0;
}

static META_FUNCTION(AudioDecoder, decodeData) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    return 1;
}

static META_FUNCTION(AudioDecoder, readS16) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data->size);
    int frame_count;
    switch (self->format) {
        case WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, (drwav_int16*)data->root);
        }
        break;
        case OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, (short*)data->root, len);
        }
        break;
        case MP3_FORMAT: {
            frame_count = drmp3_read_pcm_frames_s16(&(self->mp3), len, (drmp3_int16*)data->root);
        }
        break;
        case FLAC_FORMAT: {
            frame_count = drflac_read_pcm_frames_s16(self->flac, len, (drflac_int16*)data->root);
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static META_FUNCTION(AudioDecoder, readF32) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data->size);
    int frame_count;
    switch (self->format) {
        case WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_f32(&(self->wav), len, (float*)data->root);
        }
        break;
        case OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_float_interleaved(self->ogg, self->info.channels, (float*)data->root, len);
        }
        break;
        case MP3_FORMAT: {
            frame_count = drmp3_read_pcm_frames_f32(&(self->mp3), len, (float*)data->root);
        }
        break;
        case FLAC_FORMAT: {
            frame_count = drflac_read_pcm_frames_f32(self->flac, len, (float*)data->root);
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static META_FUNCTION(AudioDecoder, getChunk) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data->size);
    int frame_count;
    switch (self->format) {
        case WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, (drwav_int16*)data->root);
        }
        break;
        case OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, (short*)data->root, len);
        }
        break;
        case MP3_FORMAT: {
            frame_count = drmp3_read_pcm_frames_s16(&(self->mp3), len, (drmp3_int16*)data->root);
        }
        break;
        case FLAC_FORMAT: {
            frame_count = drflac_read_pcm_frames_s16(self->flac, len, (drflac_int16*)data->root);
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static META_FUNCTION(AudioDecoder, getSampleRate) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.sample_rate);
    return 1;
}

static META_FUNCTION(AudioDecoder, getChannels) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.channels);
    return 1;
}

static META_FUNCTION(AudioDecoder, getBitDepth) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.bit_depth);
    return 1;
}

static META_FUNCTION(AudioDecoder, getFrameCount) {
    CHECK_META(AudioDecoder);
    PUSH_INTEGER(self->info.frame_count);
    return 1;
}

SELENE_API int luaopen_AudioDecoder(lua_State* LUA_STATE_NAME) {
    BEGIN_REG(reg)
        REG_FIELD(AudioDecoder, new),
        REG_FIELD(AudioDecoder, load),
    END_REG()
    BEGIN_REG(index_reg)
        REG_META_FIELD(AudioDecoder, close),
        REG_META_FIELD(AudioDecoder, seek),
        REG_META_FIELD(AudioDecoder, decodeData),
        REG_META_FIELD(AudioDecoder, getChunk),
        REG_META_FIELD(AudioDecoder, getSampleRate),
        REG_META_FIELD(AudioDecoder, getChannels),
        REG_META_FIELD(AudioDecoder, getBitDepth),
    END_REG()
    NEW_META(AudioDecoder, reg, index_reg);
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