#include "fselect.h"
#include <unistd.h>
#include <sys/select.h>

#ifdef FSELECT_THREAD_SAFE
#include <mutex>
#define LOCK std::lock_guard<std::recursive_mutex> lock(d->mutex)
#else
#define LOCK
#endif

namespace wl {

    class fselect_private {
    public:
	fselect_private();
	~fselect_private();
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	fd_set readfdresults;
	fd_set writefdresults;
	fd_set exceptfdresults;
	int nfds;
	int pipe_fds[2];
#ifdef FSELECT_THREAD_SAFE
	mutable std::recursive_mutex mutex;
#endif
	void fix_nfds();
    };

    fselect_private::fselect_private() {
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

    fselect_private::~fselect_private() {
	if (pipe_fds[0] != -1) {
	    close(pipe_fds[0]);
	}
	if (pipe_fds[1] != -1) {
	    close(pipe_fds[1]);
	}
    }

    fselect::fselect(): d(std::unique_ptr<fselect_private>(new fselect_private)) {
    }

    fselect::~fselect() = default;

    bool fselect::is_valid() const {
	LOCK;
	return d->pipe_fds[0] != -1;
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
	    r = d->readfds;
	    w = d->writefds;
	    e = d->exceptfds;
	}
	int result = ::select(d->nfds + 1, &r, &w, &e, 0);
	{
	    LOCK;
	    d->readfdresults = r;
	    d->writefdresults = w;
	    d->exceptfdresults = e;
	}
	if (result > 0 && read_isready(d->pipe_fds[0])) {
	    char buf[256];
	    read(d->pipe_fds[0], buf, sizeof(buf));
	    result--;
	    is_stop = true;
	}
	return result;
    }

    void fselect::stop() {
	LOCK;
	if (d->pipe_fds[1] != -1) {
	    static char buf[1] = {'1'};
	    write(d->pipe_fds[1], buf, sizeof(buf));
	}
    }

    void fselect::read_unwatch(int fd) {
	LOCK;
	if (fd == -1) {
	    FD_ZERO(&d->readfds);
	    if (d->pipe_fds[0] != -1) {
		FD_SET(d->pipe_fds[0], &d->readfds);
	    }
	} else if (fd != d->pipe_fds[0]) {
	    FD_CLR(fd, &d->readfds);
	}
	d->fix_nfds();
    }
    bool fselect::read_iswatch(int fd) const {
	LOCK;
	return FD_ISSET(fd, &d->readfds);
    }
    bool fselect::read_isready(int fd) const {
	LOCK;
	return FD_ISSET(fd, &d->readfdresults);
    }
    void fselect::read_watch(int fd) {
	LOCK;
	FD_SET(fd, &d->readfds);
	if (fd > d->nfds) {
	    d->nfds = fd;
	}
    }

    void fselect::write_unwatch(int fd) {
	LOCK;
	if (fd == -1) {
	    FD_ZERO(&d->readfds);
	} else {
	    FD_CLR(fd, &d->writefds);
	}
	d->fix_nfds();
    }
    bool fselect::write_iswatch(int fd) const {
	LOCK;
	return FD_ISSET(fd, &d->writefds);
    }
    bool fselect::write_isready(int fd) const {
	LOCK;
	return FD_ISSET(fd, &d->writefdresults);
    }
    void fselect::write_watch(int fd) {
	LOCK;
	FD_SET(fd, &d->writefds);
	if (fd > d->nfds) {
	    d->nfds = fd;
	}
    }

    void fselect::except_unwatch(int fd) {
	LOCK;
	if (fd == -1) {
	    FD_ZERO(&d->readfds);
	} else {
	    FD_CLR(fd, &d->exceptfds);
	}
	d->fix_nfds();
    }
    bool fselect::except_iswatch(int fd) const {
	LOCK;
	return FD_ISSET(fd, &d->exceptfds);
    }
    bool fselect::except_isready(int fd) const {
	LOCK;
	return FD_ISSET(fd, &d->exceptfdresults);
    }
    void fselect::except_watch(int fd) {
	LOCK;
	FD_SET(fd, &d->exceptfds);
	if (fd > d->nfds) {
	    d->nfds = fd;
	}
    }

    void fselect_private::fix_nfds() {
	int _nfds = -1;
	for (int i = nfds; i >= 0; i--) {
	    if (FD_ISSET(i, &readfds)) {
		_nfds = i;
		break;
	    }
	    if (FD_ISSET(i, &writefds)) {
		_nfds = i;
		break;
	    }
	    if (FD_ISSET(i, &exceptfds)) {
		_nfds = i;
		break;
	    }
	}
	nfds = _nfds;
    }

}
