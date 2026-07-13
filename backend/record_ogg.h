#pragma once
#ifdef HAVE_VORBIS

/* OGG Vorbis backend (libvorbisenc/libvorbis/libogg).
 *   quality — VBR quality, -1.0..10.0 (divided by 10 internally for
 *             vorbis_encode_init_vbr's -0.1..1.0 range) */
void *record_ogg_open(const char *path, int rate, int channels, double quality,
                      char *errmsg, int errmsg_size);
void record_ogg_feed(void *impl, const short *pcm, int frames);
void record_ogg_close(void *impl);

#endif /* HAVE_VORBIS */
