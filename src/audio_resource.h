#ifndef __sound_resource_h__
#define __sound_resource_h__

#define SOUND_RESOURCE_TFILE 1
#define SOUND_RESOURCE_TBUFFER 2

struct sound_resource {
    const char *name;
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
