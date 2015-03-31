#include "fselect.h"
#include "mock_select.h"
#include "gtest/gtest.h"

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::Invoke;

// 0(stdin)を監視し、0が読み込み可能になるケース
TEST(FSelectTest, select_01) {
    wl::MockSelect *mock = new wl::MockSelect;
    EXPECT_CALL(*mock, select(4, testing::_, testing::_, testing::_, 0))
	.Times(AtLeast(1))
    	.WillOnce(Invoke([](int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout) {
		    FD_ZERO(readfds);
		    FD_ZERO(writefds);
		    FD_ZERO(errorfds);
		    FD_SET(0, readfds);
		    return 1;
		}));

    wl::fselect fselect(mock);
    bool stop;
    fselect.read_watch(0);
    int result = fselect.select(stop);
    ASSERT_EQ(1, result);
    ASSERT_EQ(true, fselect.read_isready(0));
}

// 0(stdin)を監視し、selectがエラーを返すケース
TEST(FSelectTest, select_02) {
    wl::MockSelect *mock = new wl::MockSelect;
    EXPECT_CALL(*mock, select(4, testing::_, testing::_, testing::_, 0))
	.Times(AtLeast(1))
    	.WillOnce(Return(-1));

    wl::fselect fselect(mock);
    bool stop;
    fselect.read_watch(0);
    int result = fselect.select(stop);
    ASSERT_EQ(-1, result);
}

// 0(stdin)を監視し、途中でstop()が呼ばれるケース
TEST(FSelectTest, select_03) {
    wl::MockSelect *mock = new wl::MockSelect;
    EXPECT_CALL(*mock, select(4, testing::_, testing::_, testing::_, 0))
	.Times(AtLeast(1))
    	.WillOnce(Invoke([](int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout) {
		    FD_ZERO(readfds);
		    FD_ZERO(writefds);
		    FD_ZERO(errorfds);
		    FD_SET(3, readfds);
		    write(4, "1", 1);
		    return 1;
		}));

    wl::fselect fselect(mock);
    bool stop;
    fselect.read_watch(0);
    int result = fselect.select(stop);
    ASSERT_EQ(0, result);
    ASSERT_EQ(true, stop);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
