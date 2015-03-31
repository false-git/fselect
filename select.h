#ifndef __SELECT_H
#define __SELECT_H

struct fd_set;
struct timeval;

namespace wl {
    class Select {
    public:
	virtual ~Select() = default;
	virtual int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);
    };
}

#endif // __SELECT_H
