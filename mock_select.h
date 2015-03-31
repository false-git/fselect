#ifndef __MOCK_SELECT_H
#define __MOCK_SELECT_H

#include "select.h"
#include "gmock/gmock.h"

namespace wl {
    class MockSelect: public Select {
    public:
	MOCK_METHOD5(select, int(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout));
    };
}

#endif // __MOCK_SELECT_H
