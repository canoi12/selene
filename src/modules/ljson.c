#define LUA_JSON_VERSION "0.2.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/*
 * json.decode(str)            -> value          (raises on error: "json:<line>:<col>: message")
 * json.encode(value [, opts]) -> string
 *   opts.indent   number of spaces (or a string) for pretty printing; default compact
 *   opts.sort     sort object keys (default true when indenting)
 *
 * Mapping:
 *   object  <-> table with non sequence keys (empty table encodes as {})
 *   array   <-> sequence table (1..n)
 *   null     -> nil (inside arrays this leaves a hole)
 *   numbers  -> integers when written without fraction/exponent (Lua 5.3+)
 */

#if LUA_VERSION_NUM < 502
#define lua_rawlen lua_objlen
#endif

#ifndef LJSON_MAX_DEPTH
#define LJSON_MAX_DEPTH 256
#endif

/*==============*
 *   Decoder    *
 *==============*/

typedef struct {
    lua_State* L;
    const char* start;
    const char* p;
    const char* end;
    int depth;
} ljson_parser;

static int decode_error(ljson_parser* ps, const char* msg) {
    int line = 1;
    int col = 1;
    for (const char* c = ps->start; c < ps->p && c < ps->end; c++) {
        if (*c == '\n') { line++; col = 1; }
        else col++;
    }
    return luaL_error(ps->L, "json:%d:%d: %s", line, col, msg);
}

static void skip_ws(ljson_parser* ps) {
    while (ps->p < ps->end) {
        char c = *ps->p;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ps->p++;
        else break;
    }
}

static int parse_value(ljson_parser* ps);

static void add_utf8(luaL_Buffer* b, unsigned long cp) {
    if (cp < 0x80) {
        luaL_addchar(b, (char)cp);
    } else if (cp < 0x800) {
        luaL_addchar(b, (char)(0xC0 | (cp >> 6)));
        luaL_addchar(b, (char)(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        luaL_addchar(b, (char)(0xE0 | (cp >> 12)));
        luaL_addchar(b, (char)(0x80 | ((cp >> 6) & 0x3F)));
        luaL_addchar(b, (char)(0x80 | (cp & 0x3F)));
    } else {
        luaL_addchar(b, (char)(0xF0 | (cp >> 18)));
        luaL_addchar(b, (char)(0x80 | ((cp >> 12) & 0x3F)));
        luaL_addchar(b, (char)(0x80 | ((cp >> 6) & 0x3F)));
        luaL_addchar(b, (char)(0x80 | (cp & 0x3F)));
    }
}

static int read_hex4(ljson_parser* ps, unsigned long* out) {
    unsigned long v = 0;
    if (ps->end - ps->p < 4) return 0;
    for (int i = 0; i < 4; i++) {
        char c = ps->p[i];
        v <<= 4;
        if (c >= '0' && c <= '9') v |= (unsigned long)(c - '0');
        else if (c >= 'a' && c <= 'f') v |= (unsigned long)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v |= (unsigned long)(c - 'A' + 10);
        else return 0;
    }
    ps->p += 4;
    *out = v;
    return 1;
}

/* Expects ps->p at the opening quote; pushes the decoded string */
static int parse_string(ljson_parser* ps) {
    luaL_Buffer b;
    ps->p++; /* opening quote */
    /* the buffer keeps a stack slot, so don't touch the stack while building */
    luaL_buffinit(ps->L, &b);
    for (;;) {
        if (ps->p >= ps->end) return decode_error(ps, "unterminated string");
        unsigned char c = (unsigned char)*ps->p;
        if (c == '"') { ps->p++; break; }
        if (c < 0x20) return decode_error(ps, "control character in string");
        if (c != '\\') {
            const char* run = ps->p;
            while (ps->p < ps->end && *ps->p != '"' && *ps->p != '\\' && (unsigned char)*ps->p >= 0x20) ps->p++;
            luaL_addlstring(&b, run, (size_t)(ps->p - run));
            continue;
        }
        ps->p++;
        if (ps->p >= ps->end) return decode_error(ps, "unterminated string");
        char e = *ps->p++;
        switch (e) {
            case '"':  luaL_addchar(&b, '"'); break;
            case '\\': luaL_addchar(&b, '\\'); break;
            case '/':  luaL_addchar(&b, '/'); break;
            case 'b':  luaL_addchar(&b, '\b'); break;
            case 'f':  luaL_addchar(&b, '\f'); break;
            case 'n':  luaL_addchar(&b, '\n'); break;
            case 'r':  luaL_addchar(&b, '\r'); break;
            case 't':  luaL_addchar(&b, '\t'); break;
            case 'u': {
                unsigned long cp;
                if (!read_hex4(ps, &cp)) return decode_error(ps, "invalid \\u escape");
                if (cp >= 0xD800 && cp <= 0xDBFF) {
                    unsigned long lo;
                    if (ps->end - ps->p < 6 || ps->p[0] != '\\' || ps->p[1] != 'u')
                        return decode_error(ps, "missing low surrogate");
                    ps->p += 2;
                    if (!read_hex4(ps, &lo) || lo < 0xDC00 || lo > 0xDFFF)
                        return decode_error(ps, "invalid low surrogate");
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                    return decode_error(ps, "unexpected low surrogate");
                }
                add_utf8(&b, cp);
                break;
            }
            default: return decode_error(ps, "invalid escape sequence");
        }
    }
    luaL_pushresult(&b);
    return 1;
}

static int is_digit(char c) { return c >= '0' && c <= '9'; }

static int parse_number(ljson_parser* ps) {
    const char* s = ps->p;
    int is_float = 0;
    if (ps->p < ps->end && *ps->p == '-') ps->p++;
    if (ps->p >= ps->end || !is_digit(*ps->p)) return decode_error(ps, "invalid number");
    if (*ps->p == '0') ps->p++;
    else while (ps->p < ps->end && is_digit(*ps->p)) ps->p++;
    if (ps->p < ps->end && *ps->p == '.') {
        is_float = 1;
        ps->p++;
        if (ps->p >= ps->end || !is_digit(*ps->p)) return decode_error(ps, "invalid number");
        while (ps->p < ps->end && is_digit(*ps->p)) ps->p++;
    }
    if (ps->p < ps->end && (*ps->p == 'e' || *ps->p == 'E')) {
        is_float = 1;
        ps->p++;
        if (ps->p < ps->end && (*ps->p == '+' || *ps->p == '-')) ps->p++;
        if (ps->p >= ps->end || !is_digit(*ps->p)) return decode_error(ps, "invalid number");
        while (ps->p < ps->end && is_digit(*ps->p)) ps->p++;
    }
    char buf[64];
    size_t len = (size_t)(ps->p - s);
    if (len >= sizeof(buf)) return decode_error(ps, "number too long");
    memcpy(buf, s, len);
    buf[len] = '\0';
#if LUA_VERSION_NUM >= 503
    if (!is_float) {
        char* endp;
        long long v = strtoll(buf, &endp, 10);
        if (*endp == '\0' && !(v == LLONG_MAX || v == LLONG_MIN)) {
            lua_pushinteger(ps->L, (lua_Integer)v);
            return 1;
        }
    }
#else
    (void)is_float;
#endif
    lua_pushnumber(ps->L, (lua_Number)strtod(buf, NULL));
    return 1;
}

static int match_literal(ljson_parser* ps, const char* lit) {
    size_t n = strlen(lit);
    if ((size_t)(ps->end - ps->p) < n || memcmp(ps->p, lit, n) != 0) return 0;
    /* reject "truex", "nullable", ... */
    if ((size_t)(ps->end - ps->p) > n) {
        char c = ps->p[n];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || is_digit(c) || c == '_') return 0;
    }
    ps->p += n;
    return 1;
}

static int parse_object(ljson_parser* ps) {
    ps->p++; /* { */
    lua_newtable(ps->L);
    skip_ws(ps);
    if (ps->p < ps->end && *ps->p == '}') { ps->p++; return 1; }
    for (;;) {
        skip_ws(ps);
        if (ps->p >= ps->end || *ps->p != '"') return decode_error(ps, "expected string key");
        parse_string(ps);
        skip_ws(ps);
        if (ps->p >= ps->end || *ps->p != ':') return decode_error(ps, "expected ':'");
        ps->p++;
        parse_value(ps);
        lua_rawset(ps->L, -3);
        skip_ws(ps);
        if (ps->p < ps->end && *ps->p == ',') { ps->p++; continue; }
        if (ps->p < ps->end && *ps->p == '}') { ps->p++; return 1; }
        return decode_error(ps, "expected ',' or '}'");
    }
}

static int parse_array(ljson_parser* ps) {
    ps->p++; /* [ */
    lua_newtable(ps->L);
    skip_ws(ps);
    if (ps->p < ps->end && *ps->p == ']') { ps->p++; return 1; }
    int i = 1;
    for (;;) {
        parse_value(ps);
        lua_rawseti(ps->L, -2, i++);
        skip_ws(ps);
        if (ps->p < ps->end && *ps->p == ',') { ps->p++; continue; }
        if (ps->p < ps->end && *ps->p == ']') { ps->p++; return 1; }
        return decode_error(ps, "expected ',' or ']'");
    }
}

static int parse_value(ljson_parser* ps) {
    skip_ws(ps);
    if (ps->p >= ps->end) return decode_error(ps, "unexpected end of input");
    if (++ps->depth > LJSON_MAX_DEPTH) return decode_error(ps, "too much nesting");
    luaL_checkstack(ps->L, 4, "json: too much nesting");
    int res;
    char c = *ps->p;
    switch (c) {
        case '{': res = parse_object(ps); break;
        case '[': res = parse_array(ps); break;
        case '"': res = parse_string(ps); break;
        case 't':
            if (!match_literal(ps, "true")) return decode_error(ps, "invalid literal");
            lua_pushboolean(ps->L, 1); res = 1; break;
        case 'f':
            if (!match_literal(ps, "false")) return decode_error(ps, "invalid literal");
            lua_pushboolean(ps->L, 0); res = 1; break;
        case 'n':
            if (!match_literal(ps, "null")) return decode_error(ps, "invalid literal");
            lua_pushnil(ps->L); res = 1; break;
        default:
            if (c == '-' || is_digit(c)) { res = parse_number(ps); break; }
            return decode_error(ps, "unexpected character");
    }
    ps->depth--;
    return res;
}

static int l_json_decode(lua_State* L) {
    size_t len;
    const char* str = luaL_checklstring(L, 1, &len);
    ljson_parser ps;
    ps.L = L;
    ps.start = str;
    ps.p = str;
    ps.end = str + len;
    ps.depth = 0;
    /* skip UTF-8 BOM */
    if (len >= 3 && (unsigned char)str[0] == 0xEF && (unsigned char)str[1] == 0xBB && (unsigned char)str[2] == 0xBF)
        ps.p += 3;
    parse_value(&ps);
    skip_ws(&ps);
    if (ps.p != ps.end) return decode_error(&ps, "trailing characters after value");
    return 1;
}

/*==============*
 *   Encoder    *
 *==============*/

/*
 * Output goes to a malloc'd buffer owned by a userdata (freed by __gc even if
 * encoding raises), so the Lua stack is free to be used while walking tables.
 */
#define LJSON_BUFFER_MT "ljson.buffer"

typedef struct {
    char* data;
    size_t len, cap;
} ljson_strbuf;

typedef struct {
    lua_State* L;
    ljson_strbuf* out;
    const char* indent;   /* NULL = compact */
    size_t indent_len;
    int sort;
    int depth;
} ljson_encoder;

static int strbuf_gc(lua_State* L) {
    ljson_strbuf* sb = (ljson_strbuf*)lua_touserdata(L, 1);
    free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
    return 0;
}

static void out_add(ljson_encoder* en, const char* s, size_t n) {
    ljson_strbuf* sb = en->out;
    if (sb->len + n > sb->cap) {
        size_t cap = sb->cap ? sb->cap : 256;
        while (cap < sb->len + n) cap *= 2;
        char* data = (char*)realloc(sb->data, cap);
        if (!data) luaL_error(en->L, "json: out of memory");
        sb->data = data;
        sb->cap = cap;
    }
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
}

static void out_str(ljson_encoder* en, const char* s) { out_add(en, s, strlen(s)); }
static void out_char(ljson_encoder* en, char c) { out_add(en, &c, 1); }

static void add_newline(ljson_encoder* en) {
    if (!en->indent) return;
    out_char(en, '\n');
    for (int i = 0; i < en->depth; i++) out_add(en, en->indent, en->indent_len);
}

static void encode_string(ljson_encoder* en, const char* s, size_t len) {
    static const char hex[] = "0123456789abcdef";
    out_char(en, '"');
    size_t run = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        const char* esc = NULL;
        char ubuf[7];
        switch (c) {
            case '"':  esc = "\\\""; break;
            case '\\': esc = "\\\\"; break;
            case '\b': esc = "\\b"; break;
            case '\f': esc = "\\f"; break;
            case '\n': esc = "\\n"; break;
            case '\r': esc = "\\r"; break;
            case '\t': esc = "\\t"; break;
            default:
                if (c < 0x20) {
                    memcpy(ubuf, "\\u00", 4);
                    ubuf[4] = hex[c >> 4];
                    ubuf[5] = hex[c & 0xF];
                    ubuf[6] = '\0';
                    esc = ubuf;
                }
        }
        if (esc) {
            out_add(en, s + run, i - run);
            out_str(en, esc);
            run = i + 1;
        }
    }
    out_add(en, s + run, len - run);
    out_char(en, '"');
}

static void encode_number(ljson_encoder* en, int idx) {
    char buf[64];
#if LUA_VERSION_NUM >= 503
    if (lua_isinteger(en->L, idx)) {
        snprintf(buf, sizeof(buf), LUA_INTEGER_FMT, (LUAI_UACINT)lua_tointeger(en->L, idx));
        out_str(en, buf);
        return;
    }
#endif
    double n = (double)lua_tonumber(en->L, idx);
    if (n != n) luaL_error(en->L, "json: cannot encode NaN");
    if (n == HUGE_VAL || n == -HUGE_VAL) luaL_error(en->L, "json: cannot encode infinity");
    /* shortest representation that round trips */
    snprintf(buf, sizeof(buf), "%.17g", n);
    for (int prec = 1; prec < 17; prec++) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%.*g", prec, n);
        if (strtod(tmp, NULL) == n) {
            memcpy(buf, tmp, strlen(tmp) + 1);
            break;
        }
    }
    out_str(en, buf);
}

static void encode_value(ljson_encoder* en, int idx);

/* A table is an array when its keys are exactly 1..n (n > 0) */
static int table_array_len(lua_State* L, int idx) {
    size_t n = lua_rawlen(L, idx);
    if (n == 0) return 0;
    size_t count = 0;
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        lua_pop(L, 1);
        count++;
        if (lua_type(L, -1) != LUA_TNUMBER) {
            lua_pop(L, 1);
            return 0;
        }
    }
    return count == n ? (int)n : 0;
}

/* qsort has no context argument, the names array is set right before sorting */
static const char** s_sort_names;
static int key_compare(const void* a, const void* b) {
    return strcmp(s_sort_names[*(const int*)a], s_sort_names[*(const int*)b]);
}

static void encode_object(ljson_encoder* en, int idx) {
    lua_State* L = en->L;
    int count = 0;
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        lua_pop(L, 1);
        int kt = lua_type(L, -1);
        if (kt != LUA_TSTRING && kt != LUA_TNUMBER)
            luaL_error(L, "json: object keys must be strings or numbers, got %s", luaL_typename(L, -1));
        count++;
    }
    if (count == 0) {
        out_str(en, "{}");
        return;
    }
    /* keys: original keys, names: their string form (kept alive by the table) */
    lua_createtable(L, count, 0);
    int keys = lua_gettop(L);
    lua_createtable(L, count, 0);
    int names_t = lua_gettop(L);
    int k = 0;
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        lua_pop(L, 1);
        k++;
        lua_pushvalue(L, -1);
        lua_rawseti(L, keys, k);
        lua_pushvalue(L, -1);
        lua_tostring(L, -1); /* converts the copy in place */
        lua_rawseti(L, names_t, k);
    }
    /* scratch memory as userdata so it is collected if something raises */
    const char** names = (const char**)lua_newuserdata(L, sizeof(char*) * (size_t)count);
    int* order = (int*)lua_newuserdata(L, sizeof(int) * (size_t)count);
    for (int i = 0; i < count; i++) {
        lua_rawgeti(L, names_t, i + 1);
        names[i] = lua_tostring(L, -1);
        lua_pop(L, 1);
        order[i] = i;
    }
    if (en->sort) {
        s_sort_names = names;
        qsort(order, (size_t)count, sizeof(int), key_compare);
        s_sort_names = NULL;
    }
    out_char(en, '{');
    en->depth++;
    for (int i = 0; i < count; i++) {
        int o = order[i];
        if (i > 0) out_char(en, ',');
        add_newline(en);
        encode_string(en, names[o], strlen(names[o]));
        out_char(en, ':');
        if (en->indent) out_char(en, ' ');
        lua_rawgeti(L, keys, o + 1);
        lua_rawget(L, idx);
        encode_value(en, lua_gettop(L));
        lua_pop(L, 1);
    }
    en->depth--;
    add_newline(en);
    out_char(en, '}');
    lua_settop(L, keys - 1);
}

static void encode_table(ljson_encoder* en, int idx) {
    lua_State* L = en->L;
    if (en->depth >= LJSON_MAX_DEPTH) luaL_error(L, "json: too much nesting (cyclic table?)");
    luaL_checkstack(L, 8, "json: too much nesting");
    int n = table_array_len(L, idx);
    if (n == 0) {
        encode_object(en, idx);
        return;
    }
    out_char(en, '[');
    en->depth++;
    for (int i = 1; i <= n; i++) {
        if (i > 1) out_char(en, ',');
        add_newline(en);
        lua_rawgeti(L, idx, i);
        encode_value(en, lua_gettop(L));
        lua_pop(L, 1);
    }
    en->depth--;
    add_newline(en);
    out_char(en, ']');
}

static void encode_value(ljson_encoder* en, int idx) {
    lua_State* L = en->L;
    switch (lua_type(L, idx)) {
        case LUA_TNIL: out_str(en, "null"); break;
        case LUA_TBOOLEAN: out_str(en, lua_toboolean(L, idx) ? "true" : "false"); break;
        case LUA_TNUMBER: encode_number(en, idx); break;
        case LUA_TSTRING: {
            size_t len;
            const char* s = lua_tolstring(L, idx, &len);
            encode_string(en, s, len);
            break;
        }
        case LUA_TTABLE: encode_table(en, idx); break;
        default:
            luaL_error(L, "json: cannot encode value of type %s", luaL_typename(L, idx));
    }
}

static int l_json_encode(lua_State* L) {
    luaL_checkany(L, 1);
    lua_settop(L, 2);
    ljson_encoder en;
    char spaces[17];
    en.L = L;
    en.indent = NULL;
    en.indent_len = 0;
    en.sort = 0;
    en.depth = 0;
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "indent");          /* index 3, keeps a string indent alive */
        if (lua_type(L, -1) == LUA_TNUMBER) {
            int n = (int)lua_tointeger(L, -1);
            if (n < 0) n = 0;
            if (n > 16) n = 16;
            memset(spaces, ' ', (size_t)n);
            spaces[n] = '\0';
            en.indent = spaces;
            en.indent_len = (size_t)n;
        } else if (lua_type(L, -1) == LUA_TSTRING) {
            en.indent = lua_tolstring(L, -1, &en.indent_len);
        }
        lua_getfield(L, 2, "sort");
        en.sort = lua_isnil(L, -1) ? (en.indent != NULL) : lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    ljson_strbuf* sb = (ljson_strbuf*)lua_newuserdata(L, sizeof(ljson_strbuf));
    sb->data = NULL;
    sb->len = sb->cap = 0;
    luaL_getmetatable(L, LJSON_BUFFER_MT);
    lua_setmetatable(L, -2);
    en.out = sb;
    encode_value(&en, 1);
    lua_pushlstring(L, sb->data ? sb->data : "", sb->len);
    return 1;
}

/*==============*
 *    Module    *
 *==============*/

#ifdef BUILD_AS_DLL
#define LJSON_API __declspec(dllexport)
#endif

#ifndef LJSON_API
#define LJSON_API
#endif

LJSON_API int luaopen_json(lua_State* L) {
    luaL_Reg reg[] = {
        { "encode", l_json_encode },
        { "decode", l_json_decode },
        { NULL, NULL }
    };
    luaL_newmetatable(L, LJSON_BUFFER_MT);
    lua_pushcfunction(L, strbuf_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
    luaL_newlib(L, reg);
    lua_pushstring(L, LUA_JSON_VERSION);
    lua_setfield(L, -2, "_VERSION");
    return 1;
}
