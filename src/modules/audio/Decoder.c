#include "audio.h"

#include "selene.h"
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
            dec->bit_depth = 16;
            dec->sample_rate = dec->wav.sampleRate;
            dec->channels = dec->wav.channels;
        }
        break;
        case OGG_FORMAT: {
            stb_vorbis* ogg = stb_vorbis_open_filename(path, NULL, NULL);
            if (!ogg)
                return luaL_error(L, "Failed to load ogg: %s\n", path);
            stb_vorbis_info info;
            info = stb_vorbis_get_info(ogg);
            dec->bit_depth = 16;
            dec->sample_rate = info.sample_rate;
            dec->channels = info.channels;
            dec->ogg = ogg;
        }
        break;
    }
    dec->buffer_len = 4096 * dec->channels * sizeof(int16_t);
    dec->buffer = malloc(dec->buffer_len);
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
    if (self->buffer) {
        free(self->buffer);
        self->buffer = NULL;
        self->buffer_len = 0;
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
            frame_count = stb_vorbis_get_samples_short_interleaved(self->ogg, self->channels, data->data, len);
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
    PUSH_INTEGER(self->sample_rate);
    return 1;
}

static META_FUNCTION(Decoder, GetChannels) {
    CHECK_META(Decoder);
    PUSH_INTEGER(self->channels);
    return 1;
}

static META_FUNCTION(Decoder, GetBitDepth) {
    CHECK_META(Decoder);
    PUSH_INTEGER(self->bit_depth);
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
