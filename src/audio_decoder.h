#ifndef __sound_decoder_h__
#define __sound_decoder_h__

struct sound_resource;
struct sound_decoder_ops {
    int  (*init)();
    void (*fini)();
    int  (*decode)(struct sound_resource *res);
};

extern struct sound_decoder_ops g_sound_decoders *[];

#endif
