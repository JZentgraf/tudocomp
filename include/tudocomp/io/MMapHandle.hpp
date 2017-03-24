#pragma once

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <tudocomp/util/View.hpp>

namespace tdc {namespace io {
    inline size_t pagesize() {
        return sysconf(_SC_PAGESIZE);
    }

    /// A handle for a memory map.
    ///
    /// Can either be a file mapping or an anonymous mapping.
    class MMap {
    public:
        enum class State {
            Unmapped,
            Shared,
            Private
        };

        enum class Mode {
            Read,
            ReadWrite
        };
    private:
        uint8_t* m_ptr   = nullptr;
        size_t   m_size  = 0;
        int      m_fd    = -1;

        State    m_state = State::Unmapped;
        Mode     m_mode  = Mode::Read;

    public:
        inline static bool is_offset_valid(size_t offset) {
            return (offset % pagesize()) == 0;
        }

        inline static size_t next_valid_offset(size_t offset) {
            auto ps = pagesize();
            auto diff = offset % ps;
            auto ok = offset - diff;
            DCHECK(is_offset_valid(ok));
            DCHECK(ok <= offset);
            return ok;
        }

        inline MMap() { /* field default values are fine already */ }

        inline MMap(const std::string& path,
             Mode mode,
             size_t size,
             size_t offset = 0)
        {
            // Open file for memory map
            m_fd = open(path.c_str(), O_RDONLY);
            CHECK(m_fd != -1) << "Error at opening file";

            // Map into memory

            int mmap_prot;
            int mmap_flags;

            if (mode == Mode::ReadWrite) {
                mmap_prot = PROT_READ | PROT_WRITE;
                mmap_flags = MAP_PRIVATE;
                m_state = State::Private;
            } else {
                mmap_prot = PROT_READ;
                mmap_flags = MAP_SHARED;
                m_state = State::Shared;
            }
            m_mode = mode;
            m_size = size;

            DCHECK(is_offset_valid(offset));

            void* ptr = mmap(NULL,
                             m_size,
                             mmap_prot,
                             mmap_flags,
                             m_fd,
                             offset);
            CHECK(ptr != MAP_FAILED) << "Error at mapping file into memory";

            m_ptr = (uint8_t*) ptr;
        }

        inline MMap(size_t size)
        {
            int mmap_prot = PROT_READ | PROT_WRITE;;
            int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;

            m_size = size;

            void* ptr = mmap(NULL,
                             m_size,
                             mmap_prot,
                             mmap_flags,
                             -1,
                             0);
            CHECK(ptr != MAP_FAILED) << "Error at creating anon. memory map";

            m_ptr = (uint8_t*) ptr;
            m_fd = -1;

            m_mode = Mode::ReadWrite;
            m_state = State::Private;
        }

        inline void remap(size_t new_size) {
            DCHECK(m_mode == Mode::ReadWrite);
            DCHECK(m_state == State::Private);
            DCHECK(m_fd == -1);

            auto p = mremap(m_ptr, m_size, new_size, MREMAP_MAYMOVE);
            CHECK(p != MAP_FAILED) << "Error at remapping memory";

            m_ptr = (uint8_t*) p;
            m_size =  new_size;
        }

        View view() const {
            return View(m_ptr, m_size);
        }

        GenericView<uint8_t> view() {
            const auto err = "Attempting to get a mutable view into a read-only mapping. Call the const overload of view() instead"_v;

            DCHECK(m_state == State::Private) << err;
            DCHECK(m_mode == Mode::ReadWrite) << err;
            return GenericView<uint8_t>(m_ptr, m_size);
        }

        inline MMap(const MMap& other) = delete;
        inline MMap& operator=(const MMap& other) = delete;
    private:
        inline void move_from(MMap&& other) {
            m_ptr   = other.m_ptr;
            m_size  = other.m_size;
            m_fd    = other.m_fd;

            m_state = other.m_state;
            m_mode  = other.m_mode;

            other.m_state = State::Unmapped;
            other.m_fd = -1;
        }
    public:
        inline MMap(MMap&& other) {
            move_from(std::move(other));
        }

        inline MMap& operator=(MMap&& other) {
            move_from(std::move(other));
            return *this;
        }

        ~MMap() {
            if (m_state != State::Unmapped) {
                DCHECK(m_ptr != nullptr);

                int rc = munmap(m_ptr, m_size);
                CHECK(rc == 0) << "Error at unmapping";

                if (m_fd != -1) {
                    close(m_fd);
                    m_fd = -1;
                }
            } else {
                DCHECK(m_fd == -1);
            }
        }
    };
}}
