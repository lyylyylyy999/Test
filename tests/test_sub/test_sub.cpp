#include <gtest/gtest.h>
#include "sub.hpp"

// 测试sub函数的基本功能
TEST(subTest, Basicsubition) {
    EXPECT_EQ(sub(2, 3), -1);
    EXPECT_EQ(sub(-1, 1), -2);
    EXPECT_EQ(sub(0, 0), 0);
    EXPECT_EQ(sub(-5, -3), -2);
}

// 测试sub函数的大数相加
TEST(subTest, LargeNumbers) {
    EXPECT_EQ(sub(1000000, 2000000),-1000000);
    EXPECT_EQ(sub(1000000, 1000000), 0);
}