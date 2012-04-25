#pragma once
#include "pollerimpl.h"

#include <sys/socket.h>
#include <poll.h>

#include "util.h"
#include "ptr.h"

class IPollerFace {
public:
    struct TChange {
        SOCKET Fd;
        void* Data;
        ui16 Flags;
    };

    struct TEvent {
        void* Data;
        int Status;
        ui16 Filter;
    };

    typedef yvector<TEvent> TEvents;

    virtual ~IPollerFace() {
    }

    inline void Set(void* ptr, SOCKET fd, ui16 flags) {
        const TChange c = {
              fd
            , ptr
            , flags
        };

        Set(c);
    }

    virtual void Set(const TChange& change) = 0;
    virtual void Wait(TEvents& events, TInstant deadLine) = 0;

    static TAutoPtr<IPollerFace> Default();
    static TAutoPtr<IPollerFace> Construct(const TStringBuf& name);
};
