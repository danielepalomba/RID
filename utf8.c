#include "utf8.h"

/** @copydoc utf8_byte_length */
int utf8_byte_length(uint8_t lead){
    if(lead < 0x80) return 1;       /* 0xxxxxxx */
    if((lead & 0xE0) == 0xC0) return 2; /* 110xxxxx */
    if((lead & 0xF0) == 0xE0) return 3; /* 1110xxxx */
    if((lead & 0xF8) == 0xF0) return 4; /* 11110xxx */
    return 1;
}

/** @copydoc utf8_encode */
int utf8_encode(uint32_t codepoint, char *out){
    if(codepoint < 0x80){
        out[0] = (char)codepoint;
        return 1;
    }else if(codepoint < 0x800){
        out[0] = (char)(0xC0 | (codepoint >> 6));
        out[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    }else if(codepoint < 0x10000){
        out[0] = (char)(0xE0 | (codepoint >> 12));
        out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }else if(codepoint <= 0x10FFFF){
        out[0] = (char)(0xF0 | (codepoint >> 18));
        out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }
    /* Out-of-range codepoint → replacement character U+FFFD */
    out[0] = (char)0xEF;
    out[1] = (char)0xBF;
    out[2] = (char)0xBD;
    return 3;
}

/** @copydoc utf8_decode */
int utf8_decode(const char *s, uint32_t *cp){
    uint8_t lead = (uint8_t)s[0];
    int len = utf8_byte_length(lead);

    if(len == 1){
        *cp = lead;
        return 1;
    }

    uint32_t result;
    switch(len){
        case 2:
            result = lead & 0x1F;
            break;
        case 3:
            result = lead & 0x0F;
            break;
        case 4:
            result = lead & 0x07;
            break;
        default:
            *cp = lead;
            return 1;
    }

    for(int i = 1; i < len; i++){
        uint8_t cont = (uint8_t)s[i];
        if((cont & 0xC0) != 0x80){
            /* Invalid continuation byte — fall back to raw lead byte */
            *cp = lead;
            return 1;
        }
        result = (result << 6) | (cont & 0x3F);
    }

    *cp = result;
    return len;
}

/** @copydoc utf8_codepoint_count */
size_t utf8_codepoint_count(const char *s, size_t byte_len){
    size_t count = 0;
    size_t i = 0;
    while(i < byte_len){
        int clen = utf8_byte_length((uint8_t)s[i]);
        if(i + (size_t)clen > byte_len)
            break;
        count++;
        i += (size_t)clen;
    }
    return count;
}

/** @copydoc utf8_byte_offset */
size_t utf8_byte_offset(const char *s, size_t byte_len, int cp_index){
    size_t offset = 0;
    int idx = 0;
    while(offset < byte_len && idx < cp_index){
        int clen = utf8_byte_length((uint8_t)s[offset]);
        if(offset + (size_t)clen > byte_len)
            break;
        offset += (size_t)clen;
        idx++;
    }
    return offset;
}

/** @copydoc utf8_cp_index */
int utf8_cp_index(const char *s, size_t byte_offset){
    size_t pos = 0;
    int idx = 0;
    while(pos < byte_offset){
        int clen = utf8_byte_length((uint8_t)s[pos]);
        pos += (size_t)clen;
        idx++;
    }
    return idx;
}
