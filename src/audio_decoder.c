#include "sound_decoder.h"
#include "sound_resource.h"

// alut (wav/raw)
static int
alut_init() {
}

static void
alut_fini() {
}

static int
alut_decode(struct sound_resource *res) {
}

static const struct sound_decoder g_alut = {
};

// mpg123 (mp3)
static int 
mpg123_init() {
}

static void
mpg123_fini() {
}

static int
mpg123_decode(struct sound_resource *res) {
}

static const struct sound_decoder g_mpg123 = {
};

extern struct sound_decoder_ops g_sound_decoders *[] = {
    &g_alut,
    &g_mpg123,
    NULL,
};
