#ifndef __audio_decoder_h__
#define __audio_decoder_h__

#include "openal/al.h"

struct audio_resource;
struct audio_decoder_ops {
    int  (*init)();
    void (*fini)();
    int  (*decode)(struct audio_resource *res, ALuint *buffer);
};

extern const struct audio_decoder_ops g_audio_decoders[];

#endif
