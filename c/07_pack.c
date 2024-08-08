#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

typedef float f32_t;
typedef double f64_t;

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
uint64_t pack754(long double f, unsigned bits, unsigned expbits) {
    if (f == 0.0) {
        return 0;
    }
    long long sign = 0;
    long double fnorm = f;
    if (f < 0) {
        sign = 1;
        fnorm = -f;
    }
    int shift = 0;
    while (fnorm >= 2.) {
        fnorm /= 2.;
        shift += 1;
    }
    while (fnorm < 1.) {
        fnorm *= 2.;
        shift -= 1;
    }
    fnorm = fnorm - 1.;
    unsigned significandbits = bits - expbits - 1;
    //calculate the binary form (non-float) of the significand data
    long long significand = fnorm * ((1LL << significandbits) + .5f);
    //get the biased exponent
    long long exp = shift + ((1 << (expbits - 1)) - 1); //shift + bias
    return (sign << (bits -1)) | (exp << (bits - expbits - 1)) | significand;
}

#define unpack754_32(t32) (unpack754((t32), 32, 8))
#define unpack754_64(t32) (unpack754((t32), 64, 11))
long double unpack754(uint64_t t32, unsigned bits, unsigned expbits) {
    if (t32 == 0) {
        return 0.0;
    }
    unsigned significandbits = bits - expbits - 1; //-1 for sign bit
    long double result = (t32 & ((1LL << significandbits) - 1)); //mask
    result /= (1LL << significandbits); //convert bacj to float
    result += 1.0f;

    unsigned bias = (t32 << (expbits - 1)) - 1;
    long long shift = ((t32 >> significandbits) & ((1LL << expbits) - 1)) - bias;
    while (shift > 0) {
        result *= 2.0;
        shift -= 1;
    }
    while (shift < 0) {
        result /= 2.0;
        shift += 1;
    }
    if ((t32 >> (bits - 1)) & 1) {
        result *= -1.0;
    }

    return result;
}

// -- store a 16-bit in into a char buffer (like htons())
// -- return byte write
int packi16(int16_t t32, size_t len, uint8_t buf[len]) {
    if (len < 2) {
        return 0;
    }
    uint16_t t32un = t32;
    buf[0] = t32un >> 8;
    buf[1] = t32un;
    return 2;
}

int packi32(int32_t t32, size_t len, uint8_t buf[len]) {
    if (len < 4) {
        return 0;
    }
    uint32_t t32un = t32;
    buf[0] = t32un >> 24;
    buf[1] = t32un >> 16;
    buf[2] = t32un >> 8;
    buf[3] = t32un;
    return 4;
}

int packi64(int64_t t32, size_t len, uint8_t buf[len]) {
    if (len < 8) {
        return 0;
    }
    uint64_t t32un = t32;
    buf[0] = t32un >> 56;
    buf[1] = t32un >> 48;
    buf[2] = t32un >> 40;
    buf[3] = t32un >> 32;
    buf[4] = t32un >> 24;
    buf[5] = t32un >> 16;
    buf[6] = t32un >> 8;
    buf[7] = t32un;
    return 8;
}

// -- unpack a 16-bit in into a char buffer (like ntohs())
int16_t unpacki16(uint8_t buf[static 1]) {
    uint16_t t16un = ((uint16_t)buf[0] << 8) | buf[1];
    int16_t t16 = t16un;
    if (t16un > 0x7fffu) {
        t16 = -1 - (uint16_t)(0xffffu - t16un);
    }
    return t16;
}

int32_t unpacki32(uint8_t buf[static 1]) {
    uint32_t t32un = (
        ((uint32_t)buf[0] << 24)
        | ((uint32_t)buf[1] << 16)
        | ((uint32_t)buf[2] << 8)
        | buf[3]
    );
    int32_t t32 = t32un;
    if (t32un > 0x7fffffffu) {
        t32 = -1 - (int32_t)(0xffffffffu - t32un);
    }
    return t32;
}

int64_t unpacki64(uint8_t buf[static 1]) {
    uint64_t t64un = (
        ((uint64_t)buf[0] << 56)
        | ((uint64_t)buf[1] << 48)
        | ((uint64_t)buf[2] << 40)
        | ((uint64_t)buf[3] << 32)
        | ((uint64_t)buf[4] << 24)
        | ((uint64_t)buf[5] << 16)
        | ((uint64_t)buf[6] << 8)
        | buf[7]
    );
    int64_t t64 = t64un;
    if (t64un > 0x7fffffffffffffffu) {
        t64 = -1 -(int64_t)(0xffffffffffffffffu - t64un);
    }
    return t64;
}

int32_t pack(size_t len, uint8_t* buf, char* format, ...) {
    int32_t size = 0;
    va_list ap = {0};
    va_start(ap, format);
    for (size_t t32 = 0; format[t32] != '\0'; t32 += 1) {
        if (len < 0) {
            return size;
        }

        switch(format[t32]) {
            case 'h': {//16bits
                int16_t h = (int16_t)va_arg(ap, int);//promoted
                size += packi16(h, len, &buf[size]);
                len -= 2;
            } break;
            case 'l': {//32bits
                int32_t l = va_arg(ap, int32_t);
                size += packi32(l, len, &buf[size]);
                len -= 4;
            } break;
            case 'L': {//64bits
                int64_t l = va_arg(ap, int64_t);
                size += packi64(l, len, &buf[size]);
                len -= 8;
            } break;
            case 'c': {//8bits
                int8_t c = (int8_t)va_arg(ap, int);
                if (len > 0) {
                    buf[size] = (c >> 0) & 0xff;
                    size += 1;
                }
                len -= 1;
            } break;
            case 'f': {//32bits
                f32_t f = (f32_t)va_arg(ap, double);
                uint32_t l = pack754_32(f);
                size += packi32(l, len, &buf[size]);
                len -= 4;
            } break;
            case 'F': {//64bits
                f64_t f = (f64_t)va_arg(ap, double);
                uint64_t l = pack754_64(f);
                size += packi64(l, len, &buf[size]);
                len -= 8;
            } break;
            case 's': {//string
                char* s = va_arg(ap, char*);
                int slen = strlen(s);
                if ((len - 2) >= 0 && (len - 2) >= slen) {
                    size += packi16(slen,len, &buf[size]);
                    len -= 2;
                    memcpy(&buf[size], s, len);
                    size += len;
                }
                len -= 2 + slen;

            } break;
        }

    }
    va_end(ap);
    return size;
}

void unpack(size_t len, uint8_t* buf, char* format, ...) {
    va_list ap = {0};
    va_start(ap, format);
    int pos = 0;
    int32_t maxstrlen = 0;
    for (size_t i = 0; format[i] != '\0'; i += 1) {
        switch(format[i]) {
            case 'h': {//16bits
                int16_t* h = va_arg(ap, int16_t*);
                if (pos+2 > len) {
                    return;
                }
                *h = unpacki16(&buf[pos]);
                pos += 2;
            } break;
            case 'l': {//32bits
                if (pos+4 > len) {
                    return;
                }
                int32_t* l = va_arg(ap, int32_t*);
                *l = unpacki32(&buf[pos]);
                pos += 4;
            } break;
            case 'L': {//64bits
                if (pos+8 > len) {
                    return;
                }
                int64_t* l = va_arg(ap, int64_t*);
                *l = unpacki64(&buf[pos]);
                pos += 8;
            } break;
            case 'c': {//8bits
                if (pos+1 > len) {
                    return;
                }
                int8_t* c = va_arg(ap, int8_t*);
                if (*buf <= 0x7f) {
                    *c = buf[pos];
                } else {
                    *c = -1 - (uint8_t)(0xffu - buf[pos]);
                }
                pos += 1;
            } break;
            case 'f': {//32bits
                if (pos+4 > len) {
                    return;
                }
                f32_t* f = va_arg(ap, f32_t*);
                int32_t pf = unpacki32(&buf[pos]);
                *f = unpack754_32(pf);
                pos += 4;
            } break;
            case 'F': {//64bits
                if (pos+8 > len) {
                    return;
                }
                f64_t* f = va_arg(ap, f64_t*);
                int64_t pf = unpacki64(&buf[pos]);
                printf("pf: %"PRId64"\n", pf);
                *f = unpack754_64(pf);
                printf("Ok4\n");
                pos += 8;
            } break;
            case 's': {//string
                if (pos+2 > len) {
                    return;
                }
                char* s = va_arg(ap, char*);
                int32_t slen = unpacki16(&buf[pos]);
                pos += 2;
                if (pos+slen > len) {
                    return;
                }
                int32_t count = 0;
                if (maxstrlen > 0 && slen > maxstrlen) {
                    count = maxstrlen - 1;
                } else {
                    count = slen;
                }
                memcpy(s, &buf[pos], count);
                s[count] = '\0';
                pos += slen;
            } break;
            default:
                if (isdigit(format[i])) {
                    maxstrlen = maxstrlen * 10 + (format[i] - '0');
                }
        }
        if (!isdigit(format[i])) {
            maxstrlen = 0;
        }
    }
    va_end(ap);
}


#define DEBUG
#ifdef DEBUG
#include <limits.h>
#include <float.h>
#include <assert.h>
#endif

int main() {
    #define BUF_SIZE 1024
    #ifndef DEBUG
        uint8_t buf[BUF_SIZE] = {0};
        char* s = "Great unmitigated Zot! You've found the Runestaff!";
        int32_t packetsize = pack(
            BUF_SIZE,
            buf,
            "chhlsf",
            (int8_t)-5,
            s,
            (f32_t)-3490.6677
        );

        //store packet size in packet for kicks
        packi16(packetsize, BUF_SIZE-packetsize, &buf[packetsize-1]);
        printf("packet is %"PRId32" bytes\n", packetsize);

        int8_t magic = 0;
        int16_t ps2 = 0;
        int16_t monkeycount = 0;
        int32_t altitude = 0;
        f32_t absurdityfactor = 0.0;
        char s2[96] = {0};
        unpack(
            BUF_SIZE,
            buf,
            "chhl96sf",
            &magic,
            &ps2,
            &monkeycount,
            &altitude,
            s2,
            &absurdityfactor
        );
        printf("magic: %c\n", magic);
    #else
        // types are right:
        if (sizeof(f32_t) != 4 || sizeof(f64_t) != 8) {
            char* f32 = (char*)0;
            char* f64 = (char*)0;

            if (sizeof(float) == 4) {
                f32 = "float";
            } else if (sizeof(double) == 4) {
                f32 = "double";
            } else if (sizeof(long double) == 4) {
                f32 = "long double";
            }

            if (sizeof(float) == 8) {
                f64 = "float";
            } else if (sizeof(double) == 8) {
                f64 = "double";
            } else if (sizeof(long double) == 8) {
                f64 = "long double";
            }

            if (f32 == 0 || f64 == 0) {
                printf("I can't find the following size floating point types:%s%s\n\n", f32==NULL?" 32-bit":"", f64==NULL?" 64-bit":"");
                printf("Change the typedefs at the top of this source to the right types.\n");
                return 1;
            }
            printf("Please modify this source so the following typedefs are at the top:\n\n");
            printf("typedef %s float32_t;\n", f32);
            printf("typedef %s float64_t;\n", f64);
            return 1;
        }
        int64_t test64[14] = {0, -0, 1, 2, -1, -2, LLONG_MAX>>1, LLONG_MAX-1, LLONG_MAX, LLONG_MIN+1, LLONG_MIN, 9007199254740991, 9007199254740992, 9007199254740993};
        int32_t test32[14] = {0, -0, 1, 2, -1, -2, INT_MAX>>1, INT_MAX-1, INT_MAX, INT_MIN+1, INT_MIN, 0, 0, 0};
        int16_t test16[14] = {0, -0, 1, 2, -1, -2, SHRT_MAX>>1, SHRT_MAX-1, SHRT_MAX, SHRT_MIN+1, SHRT_MIN, 0, 0, 0};
        uint8_t buf[BUF_SIZE];
        for(int i = 0; i < 14; i += 1) {
            int64_t t64 = test64[i];
            int64_t t64un = 0;
            pack(BUF_SIZE, buf, "L", t64);
            unpack(BUF_SIZE, buf, "L", &t64un);
            if (t64un != t64) {
                printf("64: %"PRId64" != %"PRId64"\n", t64, t64un);
                printf("  before: %016"PRIx64"\n", t64);
                printf("  after:  %016"PRIx64"\n", t64un);
                printf("  buffer: %02hhx %02hhx %02hhx %02hhx"
                       " %02hhx %02hhx %02hhx %02hhx\n",
                       buf[0], buf[1], buf[2], buf[3],
                       buf[4], buf[5], buf[6], buf[7]);
            } else {
                //printf("64: OK: %" PRId64 " == %" PRId64 "\n", t64, t64un);
            }

            int32_t t32 = test32[i];
            int32_t t32un = 0;
            pack(BUF_SIZE, buf, "l", t32);
            unpack(BUF_SIZE, buf, "l", &t32un);
            if (t32un != t32) {
                printf("32: %" PRId32 " != %" PRId32 "\n", t32, t32un);
                printf("  before: %08" PRIx32 "\n", t32);
                printf("  after:  %08" PRIx32 "\n", t32un);
                printf("  buffer: %02hhx %02hhx %02hhx %02hhx\n",
                       buf[0], buf[1], buf[2], buf[3]);
            } else {
                //printf("32: OK: %" PRId32 " == %" PRId32 "\n", t32, t32un);
            }

            int16_t t16 = test16[i];
            int16_t t16un = 0;
            pack(BUF_SIZE, buf, "h", t16);
            unpack(BUF_SIZE, buf, "h", &t16un);
            if (t16un != t16) {
                printf("16: %" PRId16 " != %" PRId16 "\n", t16, t16un);
            } else {
                //printf("16: OK: %" PRId16 " == %" PRId16 "\n", t16, t16un);
            }
        }

        {
            f64_t testf64[7] = { 0.0, 1.0, -1.0, DBL_MIN*2, DBL_MAX/2, DBL_MIN, DBL_MAX };

            for (int i = 0; i < 7; i += 1) {
                f64_t f64 = testf64[i];
                f64_t f64un = 0;
                printf("f64: %f\n", f64);
                pack(BUF_SIZE, buf, "F", f64);
                printf("unpack\n");
                unpack(BUF_SIZE, buf, "F", &f64un);

                if (f64un != f64un) {
                    printf("f64: %f != %f\n", f64, f64un);
                    printf("  before: %016" PRIx64 "\n", *((uint64_t*)&f64));
                    printf("  after:  %016" PRIx64 "\n", *((uint64_t*)&f64un));
                    printf(
                        "  buffer: %02hhx %02hhx %02hhx %02hhx "
                        "%02hhx %02hhx %02hhx %02hhx\n",
                        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]
                    );
                } else {
                    //printf("f64: OK: %f == %f\n", f64, f64un);
                }
            }
        }
    #endif
    return 0;
}
