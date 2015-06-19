#ifndef __audio_resource_h__
#define __audio_resource_h__

#include <stdio.h>

#define AUDIO_RESOURCE_TFILE 1
#define AUDIO_RESOURCE_TBUFFER 2

struct audio_resource {
    int type;
    union {
        struct {
            FILE *fp;
        };
        struct {
            void *buffer;
            size_t sz;
        };
    };
};

#endif
