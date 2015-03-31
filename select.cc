#include <sys/select.h>
#include "select.h"

namespace wl {

    int Select::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout) {
	return ::select(nfds, readfds, writefds, errorfds, timeout);
    }
}

