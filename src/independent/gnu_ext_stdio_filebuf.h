// Compatibility shim: provides __gnu_cxx::stdio_filebuf<CharT> for
// clang/libc++ (macOS). GCC's original wraps an existing file descriptor in a
// std::basic_streambuf without taking ownership of the fd; this shim replicates
// that contract using read()/write() system calls.

#pragma once
#include <streambuf>
#include <ios>
#include <unistd.h>

namespace __gnu_cxx {

template<typename CharT, typename Traits = std::char_traits<CharT>>
class stdio_filebuf : public std::basic_streambuf<CharT, Traits>
{
public:
    using char_type   = CharT;
    using traits_type = Traits;
    using int_type    = typename Traits::int_type;

    stdio_filebuf() : _fd(-1), _open(false), _mode(std::ios_base::in) {}

    stdio_filebuf(int fd, std::ios_base::openmode mode)
        : _fd(fd), _open(fd >= 0), _mode(mode)
    {
        if (_open) {
            if (mode & std::ios_base::in)
                this->setg(_rbuf, _rbuf + BUFSZ, _rbuf + BUFSZ);
            if (mode & std::ios_base::out)
                this->setp(_wbuf, _wbuf + BUFSZ);
        }
    }

    ~stdio_filebuf()
    {
        // Flush but do NOT close the fd — the caller owns it.
        if (_open && (_mode & std::ios_base::out))
            this->sync();
    }

    bool is_open() const { return _open; }

protected:
    int_type underflow() override
    {
        if (!(_mode & std::ios_base::in) || _fd < 0)
            return traits_type::eof();
        ssize_t n = ::read(_fd, _rbuf, BUFSZ);
        if (n <= 0) return traits_type::eof();
        this->setg(_rbuf, _rbuf, _rbuf + n);
        return traits_type::to_int_type(*this->gptr());
    }

    int sync() override
    {
        if (!(_mode & std::ios_base::out) || _fd < 0)
            return 0;
        std::ptrdiff_t n = this->pptr() - this->pbase();
        if (n > 0 && ::write(_fd, this->pbase(), static_cast<size_t>(n)) < 0)
            return -1;
        this->setp(_wbuf, _wbuf + BUFSZ);
        return 0;
    }

    int_type overflow(int_type c) override
    {
        if (this->sync() < 0) return traits_type::eof();
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *this->pptr() = traits_type::to_char_type(c);
            this->pbump(1);
        }
        return traits_type::not_eof(c);
    }

private:
    int                     _fd;
    bool                    _open;
    std::ios_base::openmode _mode;
    static constexpr std::size_t BUFSZ = 8192;
    char_type _rbuf[BUFSZ];
    char_type _wbuf[BUFSZ];
};

} // namespace __gnu_cxx
