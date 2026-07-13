#pragma once
#ifdef HAVE_SAMPLERATE

/* Thin streaming wrapper around libsamplerate for the WAV/OGG/FLAC
 * recording backends (MP3 keeps using lame's own internal resampler). */
typedef struct Resampler Resampler;

Resampler *resample_new(int channels, double ratio);

/* Feeds in_frames of interleaved shorts; writes up to out_cap resampled
 * interleaved frames to out; returns frames actually written.
 * end_of_input=1 drains the internal filter with a zero-length input
 * (used once at record_close() to flush remaining samples). */
int resample_process(Resampler *rs, const short *in, int in_frames,
                     short *out, int out_cap, int end_of_input);

void resample_delete(Resampler *rs);

#endif /* HAVE_SAMPLERATE */
