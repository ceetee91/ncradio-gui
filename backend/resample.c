#include "resample.h"
#ifdef HAVE_SAMPLERATE

#include <stdlib.h>
#include <string.h>
#include <samplerate.h>

struct Resampler {
    SRC_STATE *state;
    int        channels;
    double     ratio;
    float     *in_f;  int in_cap;
    float     *out_f; int out_cap;
};

static int ensure_f(float **buf, int *cap, int need_frames, int channels)
{
    if (need_frames <= *cap) return 1;
    float *nb = realloc(*buf, (size_t)need_frames * (size_t)channels * sizeof(float));
    if (!nb) return 0;
    *buf = nb;
    *cap = need_frames;
    return 1;
}

Resampler *resample_new(int channels, double ratio)
{
    Resampler *rs = calloc(1, sizeof(*rs));
    if (!rs) return NULL;

    int err = 0;
    rs->state = src_new(SRC_SINC_MEDIUM_QUALITY, channels, &err);
    if (!rs->state) {
        free(rs);
        return NULL;
    }
    rs->channels = channels;
    rs->ratio    = ratio;
    return rs;
}

int resample_process(Resampler *rs, const short *in, int in_frames,
                     short *out, int out_cap, int end_of_input)
{
    if (!ensure_f(&rs->out_f, &rs->out_cap, out_cap, rs->channels))
        return 0;

    SRC_DATA data;
    memset(&data, 0, sizeof(data));

    if (in_frames > 0) {
        if (!ensure_f(&rs->in_f, &rs->in_cap, in_frames, rs->channels))
            return 0;
        src_short_to_float_array(in, rs->in_f, in_frames * rs->channels);
        data.data_in      = rs->in_f;
        data.input_frames = in_frames;
    }

    data.data_out      = rs->out_f;
    data.output_frames = out_cap;
    data.src_ratio      = rs->ratio;
    data.end_of_input   = end_of_input;

    if (src_process(rs->state, &data) != 0)
        return 0;

    src_float_to_short_array(rs->out_f, out, (int)data.output_frames_gen * rs->channels);
    return (int)data.output_frames_gen;
}

void resample_delete(Resampler *rs)
{
    if (!rs) return;
    src_delete(rs->state);
    free(rs->in_f);
    free(rs->out_f);
    free(rs);
}

#endif /* HAVE_SAMPLERATE */
