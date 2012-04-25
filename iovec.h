#pragma once

#include <cassert>

class TContIOVector {
		struct TPart {
			inline TPart(const void* Buf, size_t Len) throw ()
				: buf(Buf)
				, len(Len)
			{
			}
			/*
			inline TPart(const TStringBuf& s) throw ()
				: buf(~s)
				, len(+s)
			{
			}
			*/

			inline TPart() throw ()
				: buf(0)
				, len(0)
			{
			}

			inline ~TPart() throw () {
			}

			static inline TPart CrLf() throw () {
				return TPart("\r\n", 2);
			}

			const void* buf;
			size_t      len;
		};

    public:
        inline TContIOVector(TPart* parts, size_t count)
            : Parts_(parts)
            , Count_(count)
        {
        }

        inline void Proceed(size_t len) throw () {
            while (Count_) {
                if (len < Parts_->len) {
                    Parts_->len -= len;
                    Parts_->buf = (const char*)Parts_->buf + len;

                    return;
                } else {
                    len -= Parts_->len;
                    --Count_;
                    ++Parts_;
                }
            }

            if (len)
                YASSERT(0 && "shit happen");
        }

        inline const TPart* Parts() const throw () {
            return Parts_;
        }

        inline size_t Count() const throw () {
            return Count_;
        }

        static inline size_t Bytes(const TPart* parts, size_t count) throw () {
            size_t ret = 0;

            for (size_t i = 0; i < count; ++i) {
                ret += parts[i].len;
            }

            return ret;
        }

        inline size_t Bytes() const throw () {
            return Bytes(Parts_, Count_);
        }

        inline bool Complete() const throw () {
            return !Count();
        }

    private:
        TPart* Parts_;
        size_t Count_;
};
