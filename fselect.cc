#include "fselect.h"
#include <unistd.h>

#ifdef FSELECT_THREAD_SAFE
#define LOCK std::lock_guard<std::recursive_mutex> lock(mutex)
#else
#define LOCK
#endif

namespace wl {

    fselect::fselect() {
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	FD_ZERO(&readfdresults);
	FD_ZERO(&writefdresults);
	FD_ZERO(&exceptfdresults);
	nfds = -1;
	pipe_fds[0] = -1;
	pipe_fds[1] = -1;
	int result = pipe(pipe_fds);
	if (result < 0) {
	    // error
	    nfds = -1;
	} else {
	    FD_SET(pipe_fds[0], &readfds);
	    nfds = pipe_fds[0];
	}
    }

    fselect::~fselect() {
	if (pipe_fds[0] != -1) {
	    close(pipe_fds[0]);
	}
	if (pipe_fds[1] != -1) {
	    close(pipe_fds[1]);
	}
    }

    bool fselect::is_valid() const {
	LOCK;
	return pipe_fds[0] != -1;
    }

    int fselect::select(bool &is_stop) {
	if (!is_valid()) {
	    is_stop = true;
	    return -1;
	}
	is_stop = false;
	fd_set r, w, e;
	{
	    LOCK;
	    r = readfds;
	    w = writefds;
	    e = exceptfds;
	}
	int result = ::select(nfds + 1, &r, &w, &e, 0);
	{
	    LOCK;
	    readfdresults = r;
	    writefdresults = w;
	    exceptfdresults = e;
	}
	if (result > 0 && read_isready(pipe_fds[0])) {
	    char buf[256];
	    read(pipe_fds[0], buf, sizeof(buf));
	    result--;
	    is_stop = true;
	}
	return result;
    }

    void fselect::stop() {
	LOCK;
	if (pipe_fds[1] != -1) {
	    static char buf[1] = {'1'};
	    write(pipe_fds[1], buf, sizeof(buf));
	}
    }

    void fselect::read_unwatch(int fd) {
	LOCK;
	if (fd == -1) {
	    FD_ZERO(&readfds);
	    if (pipe_fds[0] != -1) {
		FD_SET(pipe_fds[0], &readfds);
	    }
	} else if (fd != pipe_fds[0]) {
	    FD_CLR(fd, &readfds);
	}
	fix_nfds();
    }
    bool fselect::read_iswatch(int fd) const {
	LOCK;
	return FD_ISSET(fd, &readfds);
    }
    bool fselect::read_isready(int fd) const {
	LOCK;
	return FD_ISSET(fd, &readfdresults);
    }
    void fselect::read_watch(int fd) {
	LOCK;
	FD_SET(fd, &readfds);
	if (fd > nfds) {
	    nfds = fd;
	}
    }

    void fselect::write_unwatch(int fd) {
	LOCK;
	if (fd == -1) {
	    FD_ZERO(&readfds);
	} else {
	    FD_CLR(fd, &writefds);
	}
	fix_nfds();
    }
    bool fselect::write_iswatch(int fd) const {
	LOCK;
	return FD_ISSET(fd, &writefds);
    }
    bool fselect::write_isready(int fd) const {
	LOCK;
	return FD_ISSET(fd, &writefdresults);
    }
    void fselect::write_watch(int fd) {
	LOCK;
	FD_SET(fd, &writefds);
	if (fd > nfds) {
	    nfds = fd;
	}
    }

    void fselect::except_unwatch(int fd) {
	LOCK;
	if (fd == -1) {
	    FD_ZERO(&readfds);
	} else {
	    FD_CLR(fd, &exceptfds);
	}
	fix_nfds();
    }
    bool fselect::except_iswatch(int fd) const {
	LOCK;
	return FD_ISSET(fd, &exceptfds);
    }
    bool fselect::except_isready(int fd) const {
	LOCK;
	return FD_ISSET(fd, &exceptfdresults);
    }
    void fselect::except_watch(int fd) {
	LOCK;
	FD_SET(fd, &exceptfds);
	if (fd > nfds) {
	    nfds = fd;
	}
    }

    void fselect::fix_nfds() {
	int _nfds = -1;
	for (int i = nfds; i >= 0; i--) {
	    if (read_iswatch(i)) {
		_nfds = i;
		break;
	    }
	    if (write_iswatch(i)) {
		_nfds = i;
		break;
	    }
	    if (except_iswatch(i)) {
		_nfds = i;
		break;
	    }
	}
	nfds = _nfds;
    }

}
