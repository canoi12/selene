#include "audio.h"
#include "lua_helper.h"

static MODULE_FUNCTION(Decoder, New) {
    INIT_ARG();
    CHECK_STRING(path);
    return 1;
}

static MODULE_FUNCTION(Decoder, Load) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);
    char* p = path + len;
    while (*p != '.')
        p--;
    int format = UNKNOWN_FORMAT;
    if (!strcmp(p, ".ogg"))
        format = OGG_FORMAT;
    else if (!strcmp(p ,".wav"))
        format = WAV_FORMAT;
    else
        return luaL_error(L, "Unsupported audio format: %s\n", path);

    NEW_UDATA(Decoder, dec);
    dec->format = format;
    switch (format) {
        case WAV_FORMAT: {
            int success = drwav_init_file(&(dec->wav), path, NULL);
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
            stb_vorbis* ogg = stb_vorbis_open_filename(path, NULL, NULL);
            if (!ogg)
                return luaL_error(L, "Failed to load ogg: %s\n", path);
            stb_vorbis_info info;
            info = stb_vorbis_get_info(ogg);
            dec->info.sample_rate = info.sample_rate;
            dec->info.channels = info.channels;
            dec->ogg = ogg;
        }
        break;
    }
    return 1;
}

static META_FUNCTION(Decoder, Close) {
    CHECK_META(Decoder);
    switch (self->format) {
        case WAV_FORMAT: {
           drwav_uninit(&(self->wav));
        }
        break;
        case OGG_FORMAT: {
            stb_vorbis_close(self->ogg);
        }
        break;
    }
    return 0;
}

static META_FUNCTION(Decoder, Clone) {
    return 1;
}

static META_FUNCTION(Decoder, Seek) {
    CHECK_META(Decoder);
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
    }
    return 0;
}

static META_FUNCTION(Decoder, DecodeData) {
    CHECK_META(Decoder);
    CHECK_UDATA(Data, data);
    return 1;
}

static META_FUNCTION(Decoder, ReadS16) {
    CHECK_META(Decoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data->size);
    int frame_count;
    switch (self->format) {
        case WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, data->data);
        }
        break;
        case OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, data->data, len);
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static META_FUNCTION(Decoder, ReadF32) {
    CHECK_META(Decoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data->size);
    int frame_count;
    switch (self->format) {
        case WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_f32(&(self->wav), len, data->data);
        }
        break;
        case OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_float_interleaved(self->ogg, self->info.channels, data->data, len);
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static META_FUNCTION(Decoder, GetChunk) {
    CHECK_META(Decoder);
    CHECK_UDATA(Data, data);
    OPT_INTEGER(len, data->size);
    int frame_count;
    switch (self->format) {
        case WAV_FORMAT: {
            frame_count = drwav_read_pcm_frames_s16(&(self->wav), len, data->data);
        }
        break;
        case OGG_FORMAT: {
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->info.channels, data->data, len);
        }
        break;
        default:
            return luaL_error(L, "Invalid audio format in decoder\n");
    }
    PUSH_INTEGER(frame_count);
    return 1;
}

static META_FUNCTION(Decoder, GetSampleRate) {
    CHECK_META(Decoder);
    PUSH_INTEGER(self->info.sample_rate);
    return 1;
}

static META_FUNCTION(Decoder, GetChannels) {
    CHECK_META(Decoder);
    PUSH_INTEGER(self->info.channels);
    return 1;
}

static META_FUNCTION(Decoder, GetBitDepth) {
    CHECK_META(Decoder);
    PUSH_INTEGER(self->info.bit_depth);
    return 1;
}

static META_FUNCTION(Decoder, GetFrameCount) {
    CHECK_META(Decoder);
    PUSH_INTEGER(self->info.frame_count);
    return 1;
}

BEGIN_META(Decoder) {
    BEGIN_REG()
        REG_FIELD(Decoder, New),
        REG_FIELD(Decoder, Load),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Decoder, Close),
        REG_META_FIELD(Decoder, Seek),
        REG_META_FIELD(Decoder, DecodeData),
        REG_META_FIELD(Decoder, GetChunk),
        REG_META_FIELD(Decoder, GetSampleRate),
        REG_META_FIELD(Decoder, GetChannels),
        REG_META_FIELD(Decoder, GetBitDepth),
    END_REG()
    NEW_META(Decoder, _reg, _index_reg);
    return 1;
}

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
