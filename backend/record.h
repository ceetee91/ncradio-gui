#pragma once
#ifdef HAVE_RECORD

typedef enum {
    RECORD_FMT_WAV = 0,
    RECORD_FMT_MP3,
    RECORD_FMT_OGG,
    RECORD_FMT_FLAC,
    RECORD_FMT_COUNT
} RecordFormat;

typedef struct Record Record;

/*
 * Open a new recording in the given format.
 *   in_rate/in_channels  — format of PCM that will be passed to record_feed()
 *   out_rate/out_channels — desired output format (0/0 = match input)
 *   quality              — MP3: kbps; OGG: VBR quality -1.0..10.0;
 *                           FLAC: compression level 0-8; WAV: ignored
 * WAV/OGG/FLAC resample and channel-convert internally when the requested
 * output differs from the input (resampling silently falls back to the
 * input rate when built without libsamplerate). MP3 keeps using lame's
 * own internal resampling/downmix, unchanged.
 * Returns NULL on error, filling errmsg.
 */
Record *record_open(RecordFormat fmt, const char *path,
                    int in_rate, int in_channels,
                    int out_rate, int out_channels, double quality,
                    char *errmsg, int errmsg_size);

/* Feed interleaved S16_LE frames (samples-per-channel = frames). */
void record_feed(Record *r, const short *pcm, int frames);

/* Flush, finalize, close file, free r. */
void record_close(Record *r);

const char *record_extension(RecordFormat fmt);    /* ".wav" ".mp3" ".ogg" ".flac" */
const char *record_format_name(RecordFormat fmt);  /* "WAV" "MP3" "OGG" "FLAC" */
int         record_format_available(RecordFormat fmt);

/* Compiled-in formats, WAV always first — for building the settings cycle. */
extern const RecordFormat record_formats[];
extern const int          record_format_count;

#endif /* HAVE_RECORD */
