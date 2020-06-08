#include <SDL.h>
#include <physfs.h>
#include <memory>
#include <cassert>

class RwopsIo {
    std::shared_ptr<size_t> references = std::make_shared<size_t>(1);
    mutable Sint64 position = 0;
    SDL_RWops* inner;

    Sint64 reseek() {
        if (*references < 2) {
            return position;
        } else {
            return SDL_RWseek(inner, position, RW_SEEK_SET);
        }
    }

    Sint64 retell() const {
        if (*references < 2) return -1;

        Sint64 pos = SDL_RWtell(inner);
        if (pos == -1) {
            return pos;
        } else {
            position = pos;
            return position;
        }
    }

public:
    RwopsIo(SDL_RWops* src) : inner(src) {
        assert(src);
    }

    RwopsIo(const RwopsIo& src) : references(src.references), position(src.position), inner(src.inner) {
        if ((*references)++ == 1) {
            retell();
            src.retell();
        }
    }

    ~RwopsIo() {
        *references -= 1;
        size_t refs = *references;
        if (refs == 0) {
            SDL_FreeRW(inner);
        }
    }

    Sint64 seek(Sint64 offset, int whence) {
        if (whence == RW_SEEK_CUR) reseek();
        Sint64 pos = SDL_RWseek(inner, offset, whence);
        if (pos == -1 || *references < 2) {
            return pos;
        } else {
            position = pos;
            return position;
        }
    }

    Sint64 size() {
        return SDL_RWsize(inner);
    }

    size_t read(void* ptr, size_t size, size_t maxnum) {
        reseek();
        size_t read = SDL_RWread(inner, ptr, size, maxnum);
        retell();
        return read;
    }

    size_t write(const void* ptr, size_t size, size_t num) {
        reseek();
        size_t written = SDL_RWwrite(inner, ptr, size, num);
        retell();
        return written;
    }
};

static PHYSFS_sint64 read(struct PHYSFS_Io* io, void* buf, PHYSFS_uint64 len) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    return static_cast<PHYSFS_sint64>(opaque->read(buf, 1, len));
}

static PHYSFS_sint64 write(struct PHYSFS_Io* io, const void* buf, PHYSFS_uint64 len) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    return static_cast<PHYSFS_sint64>(opaque->write(buf, 1, len));
}

static int seek(struct PHYSFS_Io* io, PHYSFS_uint64 offset) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    Sint64 new_offset = opaque->seek(offset, RW_SEEK_SET);
    if (new_offset == -1) {
        return 0;
    } else {
        return 1;
    }
}

static PHYSFS_sint64 tell(struct PHYSFS_Io* io) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    return opaque->seek(0, RW_SEEK_CUR);
}

static PHYSFS_sint64 length(struct PHYSFS_Io* io) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    Sint64 length = opaque->size();
    if (length < 0) {
        return -1;
    } else {
        return length;
    }
}

static struct PHYSFS_Io* make_physfs_io(RwopsIo* rwops);

static PHYSFS_Io* duplicate(struct PHYSFS_Io* io) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    RwopsIo* clone = new RwopsIo(*opaque);
    return make_physfs_io(clone);
}

static int flush(struct PHYSFS_Io* io) {
    return 0;
}

static void destroy(struct PHYSFS_Io* io) {
    RwopsIo* opaque = static_cast<RwopsIo*>(io->opaque);
    delete opaque;
    delete io;
}

static struct PHYSFS_Io* make_physfs_io(RwopsIo* rwops) {
    struct PHYSFS_Io* vtable = new struct PHYSFS_Io;
    vtable->version = 0;
    vtable->opaque = rwops;
    vtable->read = read;
    vtable->write = write;
    vtable->seek = seek;
    vtable->tell = tell;
    vtable->length = length;
    vtable->duplicate = duplicate;
    vtable->flush = flush;
    vtable->destroy = destroy;
    return vtable;
}

struct PHYSFS_Io* make_rwopsio(SDL_RWops* inner) {
    if (!inner) return NULL;

    RwopsIo* opaque = new RwopsIo(inner);
    return make_physfs_io(opaque);
}
