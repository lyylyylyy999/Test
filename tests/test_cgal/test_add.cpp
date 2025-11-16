#include <gtest/gtest.h>
#include "add.hpp"

// 测试add函数的基本功能
TEST(AddTest, BasicAddition) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_EQ(add(-1, 1), 0);
    EXPECT_EQ(add(0, 0), 0);
    EXPECT_EQ(add(-5, -3), -8);
}

// 测试add函数的大数相加
TEST(AddTest, LargeNumbers) {
    EXPECT_EQ(add(1000000, 2000000), 3000000);
    EXPECT_EQ(add(-1000000, 1000000), 0);
}

// // 测试add函数边界值
// TEST(AddTest, BoundaryValues) {
//     EXPECT_EQ(add(INT_MAX, 0), INT_MAX);
//     EXPECT_EQ(add(INT_MIN, 0), INT_MIN);
// }