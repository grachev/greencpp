#pragma once

#include <new>

#include "util.h"
#include "align.h"

template <class T>
void Destroyer(void* ptr) {
    UNUSED(ptr);
    ((T*)ptr)->~T();
}

template <class T> T* SingletonInt();

template <class T>
void Destroyer() {
	T* ptr = SingletonInt<T>();
	Destroyer<T>(ptr);
}

template <class T>
struct TSingletonTraits {
    static const size_t Priority = 65536;
};

template <class T>
T* SingletonInt() {
    static T* volatile ret;
    static char tBuf[sizeof(T) + PLATFORM_DATA_ALIGN];

    if (!ret) {
        static TAtomic lock;
        TGuard<TAtomic> guard(lock);

        if (!ret) {
            char* buf = AlignUp((char*)tBuf);
            T* res = new (buf) T;

            try {
                //AtExit(Destroyer<T>, res, TSingletonTraits<T>::Priority);
            	std::atexit(Destroyer<T>);
            } catch (...) {
                Destroyer<T>(res);

                throw;
            }

            ret = res;
        }
    }

    return (T*)ret;
}

template <class T>
T* Singleton() {
    static T* ret;
    if (EXPECT_FALSE(!ret))
        ret = SingletonInt<T>();
    return ret;
}

template<class X>
const X& Default() {
    return *Singleton<X>();
}

