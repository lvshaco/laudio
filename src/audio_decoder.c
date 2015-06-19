#include "audio_decoder.h"
#include "audio_resource.h"

// alut (wav/raw)
static int
alut_init() {
    return 0;
}

static void
alut_fini() {
}

static int
alut_decode(struct audio_resource *res, ALuint *buffer) {
    return 0;
}

static const struct audio_decoder_ops g_alut = {
    alut_init,
    alut_fini,
    alut_decode,
};

// mpg123 (mp3)
static int 
mpg123_init() {
    return 0;
}

static void
mpg123_fini() {
}

static int
mpg123_decode(struct audio_resource *res, ALuint *buffer) {
    return 0;
}

static const struct audio_decoder_ops g_mpg123 = {
    mpg123_init,
    mpg123_fini,
    mpg123_decode,
};

const struct audio_decoder_ops * g_audio_decoders[] = {
    &g_alut,
    &g_mpg123,
    NULL,
};
