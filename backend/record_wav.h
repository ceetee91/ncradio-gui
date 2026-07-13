#pragma once

/* WAV backend — hand-rolled RIFF/WAVE writer, no external library. */
void *record_wav_open(const char *path, int rate, int channels,
                      char *errmsg, int errmsg_size);
void record_wav_feed(void *impl, const short *pcm, int frames);
void record_wav_close(void *impl);
