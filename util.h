#ifndef util_h
#define util_h

#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>
#include <thread>

#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <cstddef>
#include <ostream>
#include <limits>
#include <cerrno>
#include <vector>
#include <sstream>


#ifdef __APPLE__
	#define _darwin_
#endif

#ifdef __FreeBSD__
	#define _darwin_
#endif

#ifdef __linux__
	#define _linux_
#endif

#define _unix_
#define _x86_64_

#define UNUSED(x) (void)x

template <typename T>
inline T Max(const T& x, const T& y) {
	return std::max<T>(x, y);
}

template <typename T>
inline T Min(const T& x, const T& y) {
	return std::min<T>(x, y);
}

template <typename T>
inline T Max() {
	return std::numeric_limits<T>::max();
}

template <typename T>
inline void Zero(T& t) {
	memset(&t, 0, sizeof(T));
}

#define PLATFORM_DATA_ALIGN sizeof(void*)

#define ReinterpretCast reinterpret_cast

#define ythrow throw

inline void ThrowBadAlloc() {
    throw std::bad_alloc();
}

inline bool UncaughtException() {
    return std::uncaught_exception();
}

#define VERIFY(a,b)
#define YASSERT(x) assert(x)

class TNonCopyable {
private:  // emphasize the following members are private
	TNonCopyable(const TNonCopyable&);
	const TNonCopyable& operator=(const TNonCopyable&);
protected:
	TNonCopyable() {
	}

	~TNonCopyable() {
	}
};

typedef std::string Stroka;

typedef unsigned short ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

inline ui64 MicroSeconds() {
	timeval t;
	gettimeofday(&t, NULL);
	return ui64(t.tv_sec) * 1000000 + t.tv_usec;
}

inline struct tm* GmTimeR(time_t*,struct tm*) {
	abort();
	return NULL;
}

#define DECLARE_PODTYPE(x)

#include "base.h"

class TTempBuf {
	char* _data;

public:
	TTempBuf(size_t size)
		: _data(new char[size]) {}
	~TTempBuf() { delete[] _data; }
	void* Data() { return _data; }
};

template<typename T>
inline void DoSwap(T& x, T& y) {
	std::swap<T>(x, y);
}

#ifndef SOCKET
typedef int SOCKET;
#endif

#define INVALID_SOCKET -1
#define closesocket close

inline void ShutDown(SOCKET s, int mode) {
    if (shutdown(s, mode)) {
        ythrow std::runtime_error("shutdown socket error");
    }
}

inline void SetNonBlock(SOCKET fd, bool value = true) {
#if defined(FIONBIO)
	int nb = value;

	if (ioctl(fd, FIONBIO, &nb) < 0) {
		ythrow std::runtime_error("ioctl failed");
	}
#else
	int flags = fcntl(fd, F_GETFL, 0);
	if (value)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
#endif
}

class TSocketHolder: public TNonCopyable {
public:
    inline TSocketHolder()
        : Fd_(INVALID_SOCKET)
    {
    }

    inline TSocketHolder(SOCKET fd)
        : Fd_(fd)
    {
    }

    inline ~TSocketHolder() throw () {
        Close();
    }

    inline SOCKET Release() throw () {
        SOCKET ret = Fd_; Fd_ = INVALID_SOCKET; return ret;
    }

    inline void Close() throw () {
        if (Fd_ != INVALID_SOCKET) {
            closesocket(Fd_);
            Fd_ = INVALID_SOCKET;
        }
    }

    inline void ShutDown(int mode) const {
        ::ShutDown(Fd_, mode);
    }

    inline void Swap(TSocketHolder& r) throw () {
        DoSwap(Fd_, r.Fd_);
    }

    inline bool Closed() const throw () {
        return Fd_ == INVALID_SOCKET;
    }

    inline operator SOCKET () const throw () {
        return Fd_;
    }

private:
    SOCKET Fd_;
};

inline int LastSystemError() {
	return errno;
}

class yexception : public std::exception {
protected:
	std::string _msg;

public:
	virtual const char* what() const throw() {
		return _msg.c_str();
	}

	yexception& operator<<(const std::string s) {
		std::ostringstream oss;
		oss << s;
		_msg += oss.str();
		return *this;
	}

	virtual ~yexception() throw() {}
};

class TSystemError : public yexception {
public:
	TSystemError() {}

	TSystemError(int status) {
		char buf[100];
		strerror_r(status, buf, 100);
		_msg += buf;
		_msg += " ";
	}
};

inline std::string CurrentExceptionMessage() {
    try {
        throw;
    } catch (const std::exception& e) {
        return e.what();
    } catch (...) {
    }

    return "unknown error";
}

typedef std::ostream TOutputStream;

typedef int TFakeMutex;

namespace NBitOps {
	namespace NPrivate {
		template <unsigned N, class T>
		struct TClp2Helper {
			static inline T Calc(T t) throw () {
				const T prev = TClp2Helper<N / 2, T>::Calc(t);

				return prev | (prev >> N);
			}
		};

		template <class T>
		struct TClp2Helper<0u, T> {
			static inline T Calc(T t) throw () {
				return t - 1;
			}
		};
	}
}

template <class T>
static inline T FastClp2(T t) throw () {
	return 1 + NBitOps::NPrivate::TClp2Helper<sizeof(T) * 4, T>::Calc(t);
}

#define yvector std::vector
typedef std::string TStringBuf;
#define STRINGBUF std::string

#define EXPECT_FALSE(x) x

typedef std::mutex TAtomic;

template<typename T>
using TGuard = std::lock_guard<T>;

template <void (*func)(void*), void* p>
inline void apply() {
	func(p);
}

#endif
