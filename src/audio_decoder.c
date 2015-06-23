#include "audio_decoder.h"
#include "audio_resource.h"
#include "audio_error.h"

#include "AL/al.h"
#include "AL/alut.h"
#include "libmpg123/mpg123.h"

#include <assert.h>

static int
albuffer_init(ALuint *buffer, ALenum format, const ALvoid *data, ALsizei sz, ALsizei freq) {
    alGenBuffers(1, buffer);
    int err;
    __alcheckerrorret(err);
    if (err != AL_NO_ERROR)
        return 1;
    alBufferData(*buffer, format, data, sz, freq);
    __alcheckerror();
    return 0;
}

// alut (wav/raw)
#define __alutlogerr() \
    fprintf(stderr, "alut error: %s in %s:%d\n", \
        alutGetErrorString(alutGetError()), __FILE__, __LINE__);

static int
alut_init() {
    return 0;
}

static void
alut_fini() {
}

static int
alut_decode(struct audio_resource *res, ALuint *buffer) {
    if (res->type != AUDIO_RESOURCE_TFILE)
        return 1;
    *buffer = alutCreateBufferFromFile(res->name);
    if (*buffer == AL_NONE) {
        __alutlogerr();
        return 1;
    }
    return 0;
}

// mpg123 (mp3)
static mpg123_handle *__handle;

#define __mpglogerr() \
    fprintf(stderr, "mpg123 error: %s in %s:%d\n", \
            mpg123_strerror(__handle), __FILE__, __LINE__);

static int 
mpg_init() {
    int err;
    mpg123_init();
    __handle = mpg123_new(NULL,&err); 
    if (__handle == NULL) {
        fprintf(stderr, "%s\n", mpg123_plain_strerror(err));
        mpg123_exit();
        return 1;
    }
    if (mpg123_format(__handle, 
                44100, 
                MPG123_MONO | MPG123_STEREO,
                MPG123_ENC_UNSIGNED_8 | MPG123_ENC_SIGNED_16) != MPG123_OK) {
        __mpglogerr();
        mpg123_delete(__handle);
        __handle = NULL;
        mpg123_exit();
        return 1;
    }
    return 0;
}

static void
mpg_fini() {
    if (__handle) {
        mpg123_delete(__handle);
        __handle = NULL;
    }
    mpg123_exit();
}

static int
mpg_decode(struct audio_resource *res, ALuint *buffer) {
    if (res->type != AUDIO_RESOURCE_TFILE)
        return 1;

    if (mpg123_open(__handle, res->name) != MPG123_OK) {
        __mpglogerr();
        return 1;
    }

    long rate = 0;
    int channels = 0;
    int encoding = 0;
    if (mpg123_getformat(__handle, &rate, &channels, &encoding) != MPG123_OK) {
        __mpglogerr();
        goto err;
    }
    // can not get decoded data length
    //off_t len = mpg123_length(__handle);
    //if (len == MPG123_ERR) {
        //__mpglogerr();
        //goto err;
    //}
    ALsizei freq = rate;
    ALenum format = AL_NONE;
    if (encoding == MPG123_ENC_UNSIGNED_8)
        format = channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
    else
        format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    //size_t sz = len;
    //if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16)
        //sz *= 2; 
    size_t sz = 2048;
    char *data = malloc(sz);
    size_t done = 0;
    size_t diff = 0;
    while (mpg123_read(__handle, (unsigned char*)data+diff, sz-diff, &done) == MPG123_OK) {
        diff += done;
        done = 0;
        sz = sz * 2;
        data = realloc(data, sz);
    }
    diff += done;
    if (albuffer_init(buffer, format, data, diff, freq)) {
        free(data);
        goto err;
    }
    free(data);
    mpg123_close(__handle);
    return 0;
err:
    mpg123_close(__handle);
    return 1;
}

// decoder table
const struct audio_decoder_ops g_audio_decoders[] = {
    { alut_init, alut_fini, alut_decode },
    { mpg_init, mpg_fini, mpg_decode },
    { NULL, NULL, NULL },
};
