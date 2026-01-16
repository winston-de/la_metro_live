// Created by to Justin DeWitt

#include "justinjson.h"
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

enum {
    READ = 1,
    WRITE = 2,
    FIELD = 4,
    NOTEMPTY = 8,
    OWNS_TEXT = 16
};

static struct json_ctx {
    u16 state;
    u16 depth;
    i32 index;
    u64 bitset; // depth max 64
    union {
        FILE *fp;
        char *text;
    };
} ctx = { 0 };

bool json_read(char *str, i32 index) {
    if(ctx.state != 0) return false;
    ctx.text = str;
    ctx.index = index;
    ctx.state = READ;
    ctx.depth = 1;
    return true;
}

bool json_read_file(char *path) {
    if(ctx.state != 0) return false;
    FILE *fp = fopen(path, "rb");
    if(!fp) {
        printf("failed to read file %s: %d\n", path, errno);
        return false;
    }
    fseek(fp, 0, SEEK_END);
    i32 len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    ctx.text = malloc(len + 1);
    fread(ctx.text, 1, len, fp);
    ctx.text[len] = 0;
    ctx.index = 0;
    ctx.state = READ | OWNS_TEXT;
    ctx.depth = 1;
    return true;
}

void json_reset() {
    if(ctx.state & OWNS_TEXT) free(ctx.text);
    else if(ctx.state & WRITE) fclose(ctx.fp);
    memset(&ctx, 0, sizeof(ctx));
}

bool skip_ws() {
    while(1) {
        switch(ctx.text[ctx.index]) {
            case 0: return false;
            case ' ': case '\r': case '\n': case '\t': break;
            default: return true;
        }
        ctx.index++;
    }
}

bool json_write(char *path) {
    if(ctx.state != 0) return false;
    ctx.fp = fopen(path, "wb");
    if(!ctx.fp) return false;
    ctx.state = WRITE;
    ctx.depth = 1;
    ctx.index = 0;
    skip_ws();
    return true;
}

void indent() {
    if((ctx.state & NOTEMPTY) && (ctx.bitset & 1))
        fprintf(ctx.fp, ",\n");
    if(ctx.bitset & 1)
        for(i32 i = 1; i < ctx.depth; i++)
            fputs("  ", ctx.fp);
}

bool json_obj() {
    if(ctx.state & READ) {
        if(ctx.text[ctx.index] != '{') return false;
        ctx.index++;
        skip_ws();
    } else if(ctx.state & WRITE) {
        indent();
        fputs("{\n", ctx.fp);
    }
    ctx.state &= ~(FIELD | NOTEMPTY);
    ctx.depth++;
    ctx.bitset <<= 1;
    return true;
}

bool json_arr() {
    if(ctx.state & READ) {
        if(ctx.text[ctx.index] != '[') return false;
        ctx.index++;
        skip_ws();
    } else if(ctx.state & WRITE) {
        indent();
        fputs("[\n", ctx.fp);
    }
    ctx.state &= ~(FIELD | NOTEMPTY);
    ctx.depth++;
    ctx.bitset = (ctx.bitset << 1) | 1;
    return true;
}

bool json_read_str(char *name, i32 max_len) {
    if(ctx.text[ctx.index] != '"') return false;
    ctx.index++;
    i32 i = 0;
    while(ctx.text[ctx.index++] != '"') {
        if(--max_len < 1) continue;
        *name++ = ctx.text[ctx.index - 1];
    }
    *name = 0;
    return true;
}

bool _check_end(i32 save) {
    skip_ws();
    char c = ctx.text[ctx.index];
    if(c == ',') {
        ctx.index++;
        skip_ws();
        ctx.state &= ~FIELD;
        return true;
    }
    if(c == ((ctx.bitset & 1) ? ']' : '}'))
        return true;
    return ctx.index = save, false;
}

bool _json_write_val(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    indent();
    vfprintf(ctx.fp, fmt, args);
    ctx.state |= NOTEMPTY;
    ctx.state &= ~FIELD;
    va_end(args);
    return true;
}

bool _json_read_val(char *fmt, void *ptr) {
    i32 save = ctx.index, n = 0;
    i32 m = sscanf(&ctx.text[ctx.index], fmt, ptr, &n);
    if(m != 1) return false;
    ctx.index += n;
    return _check_end(save);
}

bool json_str(char *str, i32 max_len) {
    i32 save = ctx.index;
    if(ctx.state & READ) {
        if(!json_read_str(str, max_len))
            return false;
        return _check_end(save);
    } else if(ctx.state & WRITE) {
        if(max_len == -1)
            return _json_write_val("\"%s\"", str);
        else
            return _json_write_val("\"%.*s\"", max_len, str);
    }
    return true;
}

bool json_decimal(double *d) {
    if(ctx.state & READ)
        return _json_read_val("%lf%n", d);
    else if(ctx.state & WRITE)
        return _json_write_val("%f", *d);
    return false;
}

bool json_int(i32 *i) {
    if(ctx.state & READ)
        return _json_read_val("%d%n", i);
    else if(ctx.state & WRITE)
        return _json_write_val("%d", *i);
    return false;
}

bool json_bool(bool *b) {
    i32 save = ctx.index;
    if(ctx.state & READ) {
        if(!strncmp(&ctx.text[ctx.index], "true", 4)) {
            ctx.index += 4;
            *b = true;
        } else if(!strncmp(&ctx.text[ctx.index], "false", 5)) {
            ctx.index += 5;
            *b = false;
        } else return false;
        return _check_end(save);
    } else if(ctx.state & WRITE) {
        return _json_write_val("%s", *b ? "true" : "false");
    }
    return true;
}

bool json_skip(bool all) {
    i32 save = ctx.depth;
    bool inquotes = false;
    for(char c, p = 0; (c = ctx.text[ctx.index++]); p = c) {
        if(c == '"' && p != '\\') inquotes ^= 1;
        if(c == '"' || inquotes) continue;
        if(c == ',' && save == ctx.depth) {
            skip_ws();
            if(!all) return ctx.state &= ~FIELD, true;
        }
        if(c == '{' || c == '[') {
            ctx.depth++;
            ctx.bitset = (ctx.bitset << 1) | (c == '[');
        } else if(c == '}' || c == ']') {
            // if a '{' was closed with a ']' or vis versa
            if((ctx.bitset & 1) != (c == ']'))
                return false;
            if(ctx.depth == save)
                return ctx.index--, ctx.state &= ~FIELD, true;
            ctx.depth--;
            ctx.bitset >>= 1;
        }
    }
    return false;
}

bool json_field(char *name, i32 max_len) {
    i32 save = ctx.index;
    if(ctx.bitset & 1) return false;
    if((ctx.state & FIELD) && !json_skip(false)) return false;
    if(ctx.state & READ) {
        if(!json_read_str(name, max_len)) goto FAIL;
        skip_ws();
        if(ctx.text[ctx.index++] != ':') goto FAIL;
        skip_ws();
    } else if(ctx.state & WRITE) {
        if(ctx.state & NOTEMPTY)
            fprintf(ctx.fp, ",\n");
        for(i32 i = 1; i < ctx.depth; i++)
            fputs("  ", ctx.fp);
        if(max_len == -1)
            fprintf(ctx.fp, "\"%s\": ", name);
        else fprintf(ctx.fp, "\"%*.s\": ", (int)max_len, name);
        ctx.state |= NOTEMPTY;
    }
    ctx.state |= FIELD;
    return true;
FAIL: return ctx.index = save, false;
}

bool json_end() {
    char c = (ctx.bitset & 1) ? ']' : '}';
    if((ctx.state & READ) && ctx.text[ctx.index] != c) return false;
    ctx.depth--;
    ctx.bitset >>= 1;
    if(ctx.state & READ) {
        if(ctx.depth == 0)
            return ctx.state = 0, free(ctx.text), true;
        ctx.index++;
        if(!_check_end(ctx.index))
            return false;
    } else if(ctx.state & WRITE) {
        if(ctx.depth == 0)
            return ctx.state = 0, fclose(ctx.fp), true;
        fputc('\n', ctx.fp);
        for(i32 i = 1; i < ctx.depth; i++)
            fputs("  ", ctx.fp);
        fprintf(ctx.fp, "%c", c);
        ctx.state |= NOTEMPTY;
        ctx.state &= ~FIELD;
    }
    return true;
}