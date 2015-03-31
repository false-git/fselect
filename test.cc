#include "fselect.h"
#include "select.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <future>
#include <iostream>

int main(int argc, char *argv[]) {
    wl::fselect fselect;
    fselect.read_watch(0);
    auto th1 = std::thread([&] {
	    for (int i = 0; i < 3; i++) {
		sleep(5);
		fselect.stop();
	    }
	});
    bool stop;
    int count = 0;
    int result;
    while((result = fselect.select(stop)) >= 0) {
	if (fselect.read_isready(0)) {
	    char buf[256];
	    ssize_t len = read(0, buf, sizeof(buf));
	    write(1, buf, len);
	} else if (stop) {
	    std::cout << "stop called" << std::endl;
	    if (++count > 2) {
		break;
	    }
	} else {
	    std::cout << "something wrong: "
		      << result << "," << stop << std::endl;
	}
    }
    th1.join();
    return 0;
}
