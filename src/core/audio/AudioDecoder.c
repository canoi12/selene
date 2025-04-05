#include "selene_audio.h"

int s_AudioDecoder_init(lua_State* L, const char* path, int len, AudioDecoder* out) {
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
    out->format = (char)format;
#if defined(OS_ANDROID)
    char* data = NULL;
    size_t data_size;
    SDL_RWops* fp = SDL_RWFromFile(path, "rb");
    data_size = SDL_RWsize(fp);
    data = (char*)malloc(data_size);
    SDL_RWread(fp, data, 1, data_size);
    SDL_RWclose(fp);
#endif
    switch(format) {
        case SELENE_WAV_FORMAT: {
#if !defined(OS_ANDROID)
            int success = (int)drwav_init_file(&(out->wav), path, NULL);
#else
            int success = drwav_init_memory(&(out->wav), (const void*)data, data_size, NULL);
#endif
            if (!success)
                return luaL_error(L, "Failed to load wav: %s\n", path);
            out->info.bit_depth = out->wav.bitsPerSample;
            out->info.sample_rate = (int)out->wav.sampleRate;
            out->info.channels = out->wav.channels;
            out->info.size = (int)out->wav.dataChunkDataSize;
            out->info.frame_count = (int)out->wav.totalPCMFrameCount;
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
            out->info.sample_rate = (int)info.sample_rate;
            out->info.channels = info.channels;
            out->ogg = ogg;
        }
            break;
        case SELENE_MP3_FORMAT: {
#if !defined(OS_ANDROID)
            int success = (int)drmp3_init_file(&(out->mp3), path, NULL);
#else
            int success = drmp3_init_memory(&(out->mp3), data, data_size, NULL);
#endif
            if (!success)
                return luaL_error(L, "Failed to load mp3: %s\n", path);
            // out->info.bit_depth = out->mp3.bit;
            out->info.sample_rate = (int)out->mp3.sampleRate;
            out->info.channels = (int)out->mp3.channels;
            out->info.size = (int)out->mp3.dataSize;
            out->info.frame_count = (int)out->mp3.pcmFramesRemainingInMP3Frame;
        }
            break;
        case SELENE_FLAC_FORMAT: {
#if !defined(OS_ANDROID)
            out->flac = drflac_open_file(path, NULL);
#else
            out->flac = drflac_open_memory(data, data_size, NULL);
#endif
            if (out->flac == NULL)
                return luaL_error(L, "Failed to load flac: %s\n", path);
            out->info.bit_depth = out->flac->bitsPerSample;
            out->info.sample_rate = (int)out->flac->sampleRate;
            out->info.channels = out->flac->channels;
            out->info.size = out->flac->maxBlockSizeInPCMFrames;
            out->info.frame_count = (int)out->flac->totalPCMFrameCount;
        }
            break;
        default: break;
    }
#if defined(OS_ANDROID)
    out->audio_data = data;
#endif
    return 1;
}

static MODULE_FUNCTION(AudioDecoder, close) {
    CHECK_META(AudioDecoder);
    s_AudioDecoder_close(self);
    return 0;
}

int s_AudioDecoder_close(AudioDecoder* self) {
    switch (self->format) {
        case SELENE_WAV_FORMAT:
            drwav_uninit(&(self->wav));
            break;
        case SELENE_OGG_FORMAT:
            stb_vorbis_close(self->ogg);
            break;
        case SELENE_MP3_FORMAT:
            drmp3_uninit(&(self->mp3));
            break;
        case SELENE_FLAC_FORMAT:
            drflac_close(self->flac);
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
    s_AudioDecoder_seek(self, index);
    return 0;
}

int s_AudioDecoder_seek(AudioDecoder* self, int index) {
    switch (self->format) {
        case SELENE_WAV_FORMAT:
            drwav_seek_to_pcm_frame(&(self->wav), index);
            break;
        case SELENE_OGG_FORMAT:
            stb_vorbis_seek(self->ogg, index);
            break;
        case SELENE_MP3_FORMAT:
            drmp3_seek_to_pcm_frame(&(self->mp3), index);
            break;
        case SELENE_FLAC_FORMAT:
            drflac_seek_to_pcm_frame(self->flac, index);
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

static MODULE_FUNCTION(AudioDecoder, read_s16) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data[0]);
    int frame_count;
    frame_count = s_AudioDecoder_read_s16(self, len, (short*)(&data[1]));
    PUSH_INTEGER(frame_count);
    return 1;
}

int s_AudioDecoder_read_s16(AudioDecoder* self, int len, short* data) {
    switch (self->format) {
        case SELENE_WAV_FORMAT:
            return (int)drwav_read_pcm_frames_s16(&(self->wav), len, data);
        case SELENE_OGG_FORMAT:
            return (int)stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, data, len);
        case SELENE_MP3_FORMAT:
            return (int)drmp3_read_pcm_frames_s16(&(self->mp3), len, data);
        case SELENE_FLAC_FORMAT:
            return (int)drflac_read_pcm_frames_s16(self->flac, len, data);
        default:
            return 0;
    }
}

static MODULE_FUNCTION(AudioDecoder, read_f32) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data[0]);
    int frame_count;
    frame_count = s_AudioDecoder_read_f32(self, len, (float*)(&data[1]));
    PUSH_INTEGER(frame_count);
    return 1;
}

int s_AudioDecoder_read_f32(AudioDecoder* self, int len, float* data) {
    switch (self->format) {
        case SELENE_WAV_FORMAT:
            return (int)drwav_read_pcm_frames_f32(&(self->wav), len, data);
        case SELENE_OGG_FORMAT:
            return (int)stb_vorbis_get_samples_float_interleaved(self->ogg, self->info.channels, data, len);
        case SELENE_MP3_FORMAT:
            return (int)drmp3_read_pcm_frames_f32(&(self->mp3), len, data);
        case SELENE_FLAC_FORMAT:
            return (int)drflac_read_pcm_frames_f32(self->flac, len, data);
        default:
            return 0;
    }
}

static MODULE_FUNCTION(AudioDecoder, get_chunk) {
    CHECK_META(AudioDecoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data[0]);
    size_t frame_count;
    switch (self->format) {
        case SELENE_WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, (drwav_int16*)(&data[1]));
        }
            break;
        case SELENE_OGG_FORMAT: {
            frame_count = (size_t)stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, (short*)(&data[1]), len);
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

int l_AudioDecoder_meta(lua_State* LUA_STATE_NAME) {
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
    return 1;
}