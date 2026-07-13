#pragma once
#ifdef HAVE_FLAC

/* FLAC backend (libFLAC stream encoder).
 *   level — compression level, 0 (fastest/largest) .. 8 (slowest/smallest) */
void *record_flac_open(const char *path, int rate, int channels, int level,
                       char *errmsg, int errmsg_size);
void record_flac_feed(void *impl, const short *pcm, int frames);
void record_flac_close(void *impl);

#endif /* HAVE_FLAC */
