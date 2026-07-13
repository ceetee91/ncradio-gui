#include "record.h"
#ifdef HAVE_RECORD

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "record_mp3.h"
#include "record_wav.h"
#include "record_ogg.h"
#include "record_flac.h"
#include "resample.h"

const RecordFormat record_formats[] = {
    RECORD_FMT_WAV,
#ifdef HAVE_LAME
    RECORD_FMT_MP3,
#endif
#ifdef HAVE_VORBIS
    RECORD_FMT_OGG,
#endif
#ifdef HAVE_FLAC
    RECORD_FMT_FLAC,
#endif
};
const int record_format_count = sizeof(record_formats) / sizeof(record_formats[0]);

int record_format_available(RecordFormat fmt)
{
    switch (fmt) {
    case RECORD_FMT_WAV:  return 1;
#ifdef HAVE_LAME
    case RECORD_FMT_MP3:  return 1;
#endif
#ifdef HAVE_VORBIS
    case RECORD_FMT_OGG:  return 1;
#endif
#ifdef HAVE_FLAC
    case RECORD_FMT_FLAC: return 1;
#endif
    default: return 0;
    }
}

const char *record_extension(RecordFormat fmt)
{
    switch (fmt) {
    case RECORD_FMT_WAV:  return ".wav";
    case RECORD_FMT_MP3:  return ".mp3";
    case RECORD_FMT_OGG:  return ".ogg";
    case RECORD_FMT_FLAC: return ".flac";
    default: return "";
    }
}

const char *record_format_name(RecordFormat fmt)
{
    switch (fmt) {
    case RECORD_FMT_WAV:  return "WAV";
    case RECORD_FMT_MP3:  return "MP3";
    case RECORD_FMT_OGG:  return "OGG";
    case RECORD_FMT_FLAC: return "FLAC";
    default: return "?";
    }
}

struct Record {
    RecordFormat fmt;
    void        *impl;

    /* WAV/OGG/FLAC only — MP3 handles resampling/downmix via lame itself */
    int   in_channels;
    int   eff_channels;
    int   in_rate;
    int   eff_rate;
    int   need_chanconv;
    int   need_resample;
#ifdef HAVE_SAMPLERATE
    Resampler *rs;
#endif
    short *chanconv_buf; int chanconv_cap;
    short *resample_buf; int resample_cap;
};

static int ensure_cap(short **buf, int *cap, int need_frames, int channels)
{
    if (need_frames <= *cap) return 1;
    short *nb = realloc(*buf, (size_t)need_frames * (size_t)channels * sizeof(short));
    if (!nb) return 0;
    *buf = nb;
    *cap = need_frames;
    return 1;
}

Record *record_open(RecordFormat fmt, const char *path,
                    int in_rate, int in_channels,
                    int out_rate, int out_channels, double quality,
                    char *errmsg, int errmsg_size)
{
    if (!record_format_available(fmt)) {
        if (errmsg) snprintf(errmsg, errmsg_size, "recording format not compiled in");
        return NULL;
    }

    Record *r = calloc(1, sizeof(*r));
    if (!r) {
        if (errmsg) snprintf(errmsg, errmsg_size, "out of memory");
        return NULL;
    }
    r->fmt = fmt;
    r->in_channels = in_channels;

    if (fmt == RECORD_FMT_MP3) {
#ifdef HAVE_LAME
        r->impl = record_mp3_open(path, in_rate, in_channels, out_rate, out_channels,
                                  (int)quality, errmsg, errmsg_size);
        if (!r->impl) { free(r); return NULL; }
        return r;
#endif
    }

    /* WAV/OGG/FLAC: resample + channel-convert glue lives here */
    r->eff_channels = out_channels > 0 ? out_channels : in_channels;
    r->in_rate      = in_rate;
#ifdef HAVE_SAMPLERATE
    r->eff_rate = out_rate > 0 ? out_rate : in_rate;
#else
    r->eff_rate = in_rate; /* no resampler compiled in: always record at native rate */
#endif
    r->need_chanconv = (r->eff_channels != in_channels);
    r->need_resample = (r->eff_rate != r->in_rate);

#ifdef HAVE_SAMPLERATE
    if (r->need_resample) {
        r->rs = resample_new(r->eff_channels, (double)r->eff_rate / (double)r->in_rate);
        if (!r->rs) {
            if (errmsg) snprintf(errmsg, errmsg_size, "resampler init failed");
            free(r);
            return NULL;
        }
    }
#else
    r->need_resample = 0;
#endif

    switch (fmt) {
    case RECORD_FMT_WAV:
        r->impl = record_wav_open(path, r->eff_rate, r->eff_channels, errmsg, errmsg_size);
        break;
#ifdef HAVE_VORBIS
    case RECORD_FMT_OGG:
        r->impl = record_ogg_open(path, r->eff_rate, r->eff_channels, quality, errmsg, errmsg_size);
        break;
#endif
#ifdef HAVE_FLAC
    case RECORD_FMT_FLAC:
        r->impl = record_flac_open(path, r->eff_rate, r->eff_channels, (int)quality, errmsg, errmsg_size);
        break;
#endif
    default:
        break;
    }

    if (!r->impl) {
#ifdef HAVE_SAMPLERATE
        if (r->rs) resample_delete(r->rs);
#endif
        free(r);
        return NULL;
    }
    return r;
}

/* Channel-convert (if needed) then hand frames+count to the backend feed fn. */
static void feed_backend(Record *r, const short *pcm, int frames)
{
    switch (r->fmt) {
    case RECORD_FMT_WAV:
        record_wav_feed(r->impl, pcm, frames);
        break;
#ifdef HAVE_VORBIS
    case RECORD_FMT_OGG:
        record_ogg_feed(r->impl, pcm, frames);
        break;
#endif
#ifdef HAVE_FLAC
    case RECORD_FMT_FLAC:
        record_flac_feed(r->impl, pcm, frames);
        break;
#endif
    default:
        break;
    }
}

void record_feed(Record *r, const short *pcm, int frames)
{
    if (!r || frames <= 0) return;

    if (r->fmt == RECORD_FMT_MP3) {
#ifdef HAVE_LAME
        record_mp3_feed(r->impl, pcm, frames);
#endif
        return;
    }

    const short *src = pcm;

    if (r->need_chanconv) {
        if (!ensure_cap(&r->chanconv_buf, &r->chanconv_cap, frames, r->eff_channels))
            return;
        if (r->in_channels == 2 && r->eff_channels == 1) {
            for (int i = 0; i < frames; i++)
                r->chanconv_buf[i] = (short)(((int)pcm[i * 2] + (int)pcm[i * 2 + 1]) / 2);
        } else if (r->in_channels == 1 && r->eff_channels == 2) {
            for (int i = 0; i < frames; i++) {
                r->chanconv_buf[i * 2]     = pcm[i];
                r->chanconv_buf[i * 2 + 1] = pcm[i];
            }
        } else {
            memcpy(r->chanconv_buf, pcm, (size_t)frames * (size_t)r->eff_channels * sizeof(short));
        }
        src = r->chanconv_buf;
    }

    int out_frames = frames;

#ifdef HAVE_SAMPLERATE
    if (r->need_resample) {
        int cap = (int)((double)frames * r->eff_rate / r->in_rate) + 64;
        if (!ensure_cap(&r->resample_buf, &r->resample_cap, cap, r->eff_channels))
            return;
        out_frames = resample_process(r->rs, src, frames, r->resample_buf, cap, 0);
        src = r->resample_buf;
    }
#endif

    if (out_frames <= 0) return;
    feed_backend(r, src, out_frames);
}

void record_close(Record *r)
{
    if (!r) return;

    if (r->fmt == RECORD_FMT_MP3) {
#ifdef HAVE_LAME
        record_mp3_close(r->impl);
#endif
        free(r);
        return;
    }

#ifdef HAVE_SAMPLERATE
    if (r->need_resample) {
        int cap = 4096;
        if (ensure_cap(&r->resample_buf, &r->resample_cap, cap, r->eff_channels)) {
            int n = resample_process(r->rs, NULL, 0, r->resample_buf, cap, 1);
            if (n > 0) feed_backend(r, r->resample_buf, n);
        }
        resample_delete(r->rs);
    }
#endif

    switch (r->fmt) {
    case RECORD_FMT_WAV:
        record_wav_close(r->impl);
        break;
#ifdef HAVE_VORBIS
    case RECORD_FMT_OGG:
        record_ogg_close(r->impl);
        break;
#endif
#ifdef HAVE_FLAC
    case RECORD_FMT_FLAC:
        record_flac_close(r->impl);
        break;
#endif
    default:
        break;
    }

    free(r->chanconv_buf);
    free(r->resample_buf);
    free(r);
}

#endif /* HAVE_RECORD */
