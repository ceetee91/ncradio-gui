#pragma once
#ifdef HAVE_LAME

/*
 * MP3 backend (libmp3lame). Opaque handle returned as void * so record.c's
 * generic dispatcher doesn't need to know lame's types.
 *   in_rate/in_channels  — format of PCM passed to record_mp3_feed()
 *   out_rate/out_channels — output format; lame resamples/downmixes
 *                           internally when these differ from the input
 *   bitrate               — kbps
 */
void *record_mp3_open(const char *path,
                      int in_rate, int in_channels,
                      int out_rate, int out_channels, int bitrate,
                      char *errmsg, int errmsg_size);
void record_mp3_feed(void *impl, const short *pcm, int frames);
void record_mp3_close(void *impl);

#endif /* HAVE_LAME */
