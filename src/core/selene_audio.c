#ifndef SELENE_NO_AUDIO
#include "selene_audio.h"

/**
 * Load audio data from a file (ogg, wav, mp3 and flac)
 */
static MODULE_FUNCTION(audio, load_data) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);
    AudioDecoder dec;
    int res = s_AudioDecoder_init(L, path, (int)len, &dec);
    if (res <= 0) {
        return luaL_error(L, "failed to load audio data: %s", path);
    }
    SDL_AudioSpec dec_spec;
    dec_spec.freq = dec.info.sample_rate;
#if defined(SELENE_USE_SDL3)
    if (dec.info.bit_depth == 16) dec_spec.format = SDL_AUDIO_S16;
    else if (dec.info.bit_depth == 32) dec_spec.format = SDL_AUDIO_F32;
    else dec_spec.format = SELENE_AUDIO_FORMAT;
#else
    if (dec.info.bit_depth == 16) dec_spec.format = AUDIO_S16;
    else if (dec.info.bit_depth == 32) dec_spec.format = AUDIO_F32;
    else dec_spec.format = SELENE_AUDIO_FORMAT;
#endif
    dec_spec.channels = dec.info.channels;
    SDL_AudioSpec spec;
    // fprintf(stderr, "format: %d\n", format);
    spec.freq = SELENE_AUDIO_SAMPLE_RATE;
    spec.format = SELENE_AUDIO_FORMAT;
    spec.channels = SELENE_AUDIO_CHANNELS;
    int samples = SELENE_AUDIO_SAMPLES;
    if (lua_type(L, arg) == LUA_TTABLE) {
        lua_getfield(L, arg, "sample_rate");
        spec.freq = (int)luaL_optinteger(L, -1, spec.freq);
        lua_pop(L, 1);
        lua_getfield(L, arg, "format");
        spec.format = (int)luaL_optinteger(L, -1, spec.format);
        lua_pop(L, 1);
        lua_getfield(L, arg, "channels");
        spec.channels = (int)luaL_optinteger(L, -1, spec.channels);
        lua_getfield(L, arg, "samples");
        samples = (int)luaL_optinteger(L, -1, samples);
        lua_pop(L, 1);
    }
    short aux_data[16384];
    int frame_count = s_AudioDecoder_read_s16(&dec, 4096, aux_data);
    if (frame_count < 0) {
        return luaL_error(L, "Failed to read audio decoder");
    }
#if defined(SELENE_USE_SDL3)
    SDL_AudioStream* stream = SDL_CreateAudioStream(&dec_spec, &spec);
#else
    SDL_AudioStream* stream = SDL_NewAudioStream(
            dec_spec.format, dec.info.channels, dec.info.sample_rate,
            spec.format, spec.channels, spec.freq);
#endif

    while (frame_count != 0) {
        // short* buffer = (short*)malloc(frame_count * channels * sizeof(short));
        short* buffer = (short*)aux_data;
#if defined(SELENE_USE_SDL3)
        res = SDL_PutAudioStreamData(stream, buffer, frame_count * spec.channels * sizeof(short));
#else
        res = SDL_AudioStreamPut(stream, buffer, (int)(frame_count * spec.channels * sizeof(short)));
#endif
        if (res < 0) {
            return luaL_error(L, "Failed to put audio stream: %s", SDL_GetError());
        }
        frame_count = s_AudioDecoder_read_s16(&dec, samples, buffer);
        // free(buffer);
    }
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetAudioStreamAvailable(stream);
#else
    size_t size = SDL_AudioStreamAvailable(stream);
#endif
    NEW_UDATA(AudioData, data);
    data->size = (int)size;
    data->data = malloc(size);
    data->info.sample_rate = spec.freq;
    data->info.channels = spec.channels;
    data->info.format = spec.format;
#if defined(SELENE_USE_SDL3)
    int read = SDL_GetAudioStreamData(stream, data->data, size);
#else
    int read = SDL_AudioStreamGet(stream, data->data, (int)size);
#endif
    if (read < 0) {
        return luaL_error(L, "Failed to read audio stream: %s", SDL_GetError());
    }
#if defined(SELENE_USE_SDL3)
    SDL_DestroyAudioStream(stream);
#else
    SDL_FreeAudioStream(stream);
#endif
    return 1;
}

/**
 * Load an audio decoder from a file (ogg, wav, mp3 and flac)
 */
static MODULE_FUNCTION(audio, load_decoder) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);

    AudioDecoder out;
    int res = s_AudioDecoder_init(L, path, (int)len, &out);
    if (res <= 0) {
        return res;
    }
    NEW_UDATA(AudioDecoder, dec);
    memcpy(dec, &out, sizeof(AudioDecoder));

    return 1;
}

extern int l_AudioSystem_create(lua_State* L);
extern int l_AudioSystem_meta(lua_State* L);
extern int l_AudioDecoder_meta(lua_State* L);

int luaopen_audio(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(audio, load_data),
        REG_FIELD(audio, load_decoder),
    END_REG()
    luaL_newlib(L, reg);
    luaL_newmetatable(L, "AudioData");
    lua_setfield(L, -2, "AudioData");
    l_AudioDecoder_meta(L);
    lua_setfield(L, -2, "AudioDecoder");
    l_AudioSystem_meta(L);
    lua_setfield(L, -2, "AudioSystem");
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
