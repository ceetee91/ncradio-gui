#include "record_ogg.h"
#ifdef HAVE_VORBIS

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <vorbis/vorbisenc.h>
#include <ogg/ogg.h>

typedef struct {
    FILE *fp;
    int   channels;
    ogg_stream_state os;
    vorbis_info      vi;
    vorbis_comment   vc;
    vorbis_dsp_state vd;
    vorbis_block     vb;
} OggCtx;

static void write_pages(OggCtx *o, int flush)
{
    ogg_page og;
    int (*fn)(ogg_stream_state *, ogg_page *) = flush ? ogg_stream_flush : ogg_stream_pageout;
    while (fn(&o->os, &og)) {
        fwrite(og.header, 1, og.header_len, o->fp);
        fwrite(og.body, 1, og.body_len, o->fp);
    }
}

void *record_ogg_open(const char *path, int rate, int channels, double quality,
                      char *errmsg, int errmsg_size)
{
    OggCtx *o = calloc(1, sizeof(*o));
    if (!o) {
        if (errmsg) snprintf(errmsg, errmsg_size, "out of memory");
        return NULL;
    }
    o->channels = channels;

    vorbis_info_init(&o->vi);
    double q = quality / 10.0;
    if (q < -0.1) q = -0.1;
    if (q > 1.0) q = 1.0;
    if (vorbis_encode_init_vbr(&o->vi, channels, rate, (float)q) != 0) {
        if (errmsg) snprintf(errmsg, errmsg_size, "vorbis_encode_init_vbr failed");
        vorbis_info_clear(&o->vi);
        free(o);
        return NULL;
    }

    vorbis_comment_init(&o->vc);
    vorbis_analysis_init(&o->vd, &o->vi);
    vorbis_block_init(&o->vd, &o->vb);

    srand((unsigned int)time(NULL));
    ogg_stream_init(&o->os, rand());

    o->fp = fopen(path, "wb");
    if (!o->fp) {
        if (errmsg) snprintf(errmsg, errmsg_size, "cannot open '%s'", path);
        ogg_stream_clear(&o->os);
        vorbis_block_clear(&o->vb);
        vorbis_dsp_clear(&o->vd);
        vorbis_comment_clear(&o->vc);
        vorbis_info_clear(&o->vi);
        free(o);
        return NULL;
    }

    ogg_packet header, header_comm, header_code;
    vorbis_analysis_headerout(&o->vd, &o->vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&o->os, &header);
    ogg_stream_packetin(&o->os, &header_comm);
    ogg_stream_packetin(&o->os, &header_code);
    write_pages(o, 1);

    return o;
}

void record_ogg_feed(void *impl, const short *pcm, int frames)
{
    OggCtx *o = impl;
    float **buffer = vorbis_analysis_buffer(&o->vd, frames);
    for (int ch = 0; ch < o->channels; ch++)
        for (int i = 0; i < frames; i++)
            buffer[ch][i] = (float)pcm[i * o->channels + ch] / 32768.0f;

    vorbis_analysis_wrote(&o->vd, frames);

    while (vorbis_analysis_blockout(&o->vd, &o->vb) == 1) {
        vorbis_analysis(&o->vb, NULL);
        vorbis_bitrate_addblock(&o->vb);

        ogg_packet op;
        while (vorbis_bitrate_flushpacket(&o->vd, &op)) {
            ogg_stream_packetin(&o->os, &op);
            write_pages(o, 0);
        }
    }
}

void record_ogg_close(void *impl)
{
    OggCtx *o = impl;
    if (!o) return;

    vorbis_analysis_wrote(&o->vd, 0);

    ogg_packet op;
    while (vorbis_analysis_blockout(&o->vd, &o->vb) == 1) {
        vorbis_analysis(&o->vb, NULL);
        vorbis_bitrate_addblock(&o->vb);
        while (vorbis_bitrate_flushpacket(&o->vd, &op)) {
            ogg_stream_packetin(&o->os, &op);
            write_pages(o, 0);
        }
    }
    write_pages(o, 1);

    ogg_stream_clear(&o->os);
    vorbis_block_clear(&o->vb);
    vorbis_dsp_clear(&o->vd);
    vorbis_comment_clear(&o->vc);
    vorbis_info_clear(&o->vi);
    fclose(o->fp);
    free(o);
}

#endif /* HAVE_VORBIS */
