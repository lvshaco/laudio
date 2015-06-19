#ifndef __audio_error_h__
#define __audio_error_h__

#define __alerrorcase(err) case err: return #err;

static inline const char *
__alstrerror(int err) {
    switch(err) {
    __alerrorcase(AL_INVALID_NAME);
    __alerrorcase(AL_INVALID_ENUM);
    __alerrorcase(AL_INVALID_VALUE);
    __alerrorcase(AL_INVALID_OPERATION);
    __alerrorcase(AL_OUT_OF_MEMORY);
    default:
        return "unknow";
    }
}

#define __alcheckerror__() \
    int e = alGetError(); \
    if (e != AL_NO_ERROR) { \
        fprintf(stderr, "AL error: %s in %s:%d\n", __alstrerror(e), __FILE__, __LINE__); \
    }

#define __alcheckerror() { __alcheckerror__(); }
#define __alcheckerrorret(ret) { __alcheckerror__(); ret = e; } 

#endif
