#include "record_flac.h"
#ifdef HAVE_FLAC

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <FLAC/stream_encoder.h>

typedef struct {
    FLAC__StreamEncoder *enc;
    int channels;
    FLAC__int32 *scratch;
    int scratch_cap;
} FlacCtx;

void *record_flac_open(const char *path, int rate, int channels, int level,
                       char *errmsg, int errmsg_size)
{
    FlacCtx *f = calloc(1, sizeof(*f));
    if (!f) {
        if (errmsg) snprintf(errmsg, errmsg_size, "out of memory");
        return NULL;
    }
    f->channels = channels;

    f->enc = FLAC__stream_encoder_new();
    if (!f->enc) {
        if (errmsg) snprintf(errmsg, errmsg_size, "FLAC__stream_encoder_new failed");
        free(f);
        return NULL;
    }

    if (level < 0) level = 0;
    if (level > 8) level = 8;

    FLAC__stream_encoder_set_channels(f->enc, (unsigned)channels);
    FLAC__stream_encoder_set_bits_per_sample(f->enc, 16);
    FLAC__stream_encoder_set_sample_rate(f->enc, (unsigned)rate);
    FLAC__stream_encoder_set_compression_level(f->enc, (unsigned)level);

    if (FLAC__stream_encoder_init_file(f->enc, path, NULL, NULL) !=
        FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        if (errmsg) snprintf(errmsg, errmsg_size, "cannot open '%s'", path);
        FLAC__stream_encoder_delete(f->enc);
        free(f);
        return NULL;
    }

    return f;
}

void record_flac_feed(void *impl, const short *pcm, int frames)
{
    FlacCtx *f = impl;
    int need = frames * f->channels;
    if (need > f->scratch_cap) {
        FLAC__int32 *nb = realloc(f->scratch, (size_t)need * sizeof(FLAC__int32));
        if (!nb) return;
        f->scratch     = nb;
        f->scratch_cap = need;
    }
    for (int i = 0; i < need; i++)
        f->scratch[i] = pcm[i];

    FLAC__stream_encoder_process_interleaved(f->enc, f->scratch, frames);
}

void record_flac_close(void *impl)
{
    FlacCtx *f = impl;
    if (!f) return;
    FLAC__stream_encoder_finish(f->enc);
    FLAC__stream_encoder_delete(f->enc);
    free(f->scratch);
    free(f);
}

#endif /* HAVE_FLAC */
