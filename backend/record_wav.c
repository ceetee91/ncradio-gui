#include "record_wav.h"
#include "record.h"
#ifdef HAVE_RECORD

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    FILE *fp;
    int   channels;
    int   rate;
    unsigned int data_bytes;
} WavCtx;

static void put_u32(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v);
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

static void put_u16(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v);
    p[1] = (unsigned char)(v >> 8);
}

void *record_wav_open(const char *path, int rate, int channels,
                      char *errmsg, int errmsg_size)
{
    WavCtx *w = calloc(1, sizeof(*w));
    if (!w) {
        if (errmsg) snprintf(errmsg, errmsg_size, "out of memory");
        return NULL;
    }
    w->channels = channels;
    w->rate     = rate;

    w->fp = fopen(path, "wb");
    if (!w->fp) {
        if (errmsg) snprintf(errmsg, errmsg_size, "cannot open '%s'", path);
        free(w);
        return NULL;
    }

    /* 44-byte canonical PCM header; sizes patched in at close(). */
    unsigned char hdr[44] = {0};
    unsigned int byte_rate   = (unsigned int)rate * (unsigned int)channels * 2u;
    unsigned int block_align = (unsigned int)channels * 2u;

    memcpy(hdr, "RIFF", 4);
    put_u32(hdr + 4, 36); /* placeholder: 36 + data_bytes */
    memcpy(hdr + 8, "WAVE", 4);
    memcpy(hdr + 12, "fmt ", 4);
    put_u32(hdr + 16, 16);            /* fmt chunk size */
    put_u16(hdr + 20, 1);             /* PCM */
    put_u16(hdr + 22, (unsigned)channels);
    put_u32(hdr + 24, (unsigned)rate);
    put_u32(hdr + 28, byte_rate);
    put_u16(hdr + 32, (unsigned)block_align);
    put_u16(hdr + 34, 16);            /* bits per sample */
    memcpy(hdr + 36, "data", 4);
    put_u32(hdr + 40, 0);             /* placeholder: data_bytes */

    if (fwrite(hdr, 1, sizeof(hdr), w->fp) != sizeof(hdr)) {
        if (errmsg) snprintf(errmsg, errmsg_size, "cannot write to '%s'", path);
        fclose(w->fp);
        free(w);
        return NULL;
    }

    return w;
}

void record_wav_feed(void *impl, const short *pcm, int frames)
{
    WavCtx *w = impl;
    size_t n = (size_t)frames * (size_t)w->channels;
    fwrite(pcm, sizeof(short), n, w->fp);
    w->data_bytes += (unsigned int)(n * sizeof(short));
}

void record_wav_close(void *impl)
{
    WavCtx *w = impl;
    if (!w) return;

    unsigned char buf[4];
    fseek(w->fp, 4, SEEK_SET);
    put_u32(buf, 36 + w->data_bytes);
    fwrite(buf, 1, 4, w->fp);

    fseek(w->fp, 40, SEEK_SET);
    put_u32(buf, w->data_bytes);
    fwrite(buf, 1, 4, w->fp);

    fclose(w->fp);
    free(w);
}

#endif /* HAVE_RECORD */
