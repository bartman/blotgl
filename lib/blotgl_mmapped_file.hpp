#pragma once
#include <string>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <algorithm>

namespace BlotGL {

class MmappedFile final {
private:
    int m_fd = -1;
    void* m_data = nullptr;
    size_t m_length = 0;

public:
    MmappedFile(const std::string& filename, size_t max_size = 0) {
        m_fd = open(filename.c_str(), O_RDONLY);
        if (m_fd == -1) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        struct stat st;
        if (fstat(m_fd, &st) == -1) {
            close(m_fd);
            throw std::runtime_error("Failed to stat file: " + filename);
        }

        size_t file_size = static_cast<size_t>(st.st_size);
        m_length = (max_size == 0) ? file_size : std::min(file_size, max_size);

        if (m_length == 0) {
            close(m_fd);
            return; // Nothing to map
        }

        m_data = mmap(nullptr, m_length, PROT_READ, MAP_PRIVATE, m_fd, 0);
        if (m_data == MAP_FAILED) {
            close(m_fd);
            throw std::runtime_error("Failed to mmap file: " + filename);
        }
    }

    ~MmappedFile() {
        if (m_data != nullptr && m_data != MAP_FAILED) {
            munmap(m_data, m_length);
        }
        if (m_fd != -1) {
            close(m_fd);
        }
    }

    template<typename T = void>
    const T* data() const { return (const T*)m_data; }
    size_t size() const { return m_length; }
    std::string str() const { return std::string(data<char>(), data<char>() + size()); }

    // Prevent copying
    MmappedFile(const MmappedFile&) = delete;
    MmappedFile& operator=(const MmappedFile&) = delete;

    // Allow moving if needed
    MmappedFile(MmappedFile&& other) noexcept
        : m_fd(other.m_fd), m_data(other.m_data), m_length(other.m_length) {
        other.m_fd = -1;
        other.m_data = nullptr;
        other.m_length = 0;
    }

    MmappedFile& operator=(MmappedFile&& other) noexcept {
        if (this != &other) {
            if (m_data != nullptr) munmap(m_data, m_length);
            if (m_fd != -1) close(m_fd);

            m_fd = other.m_fd;
            m_data = other.m_data;
            m_length = other.m_length;

            other.m_fd = -1;
            other.m_data = nullptr;
            other.m_length = 0;
        }
        return *this;
    }
};

}
