#include <gtest/gtest.h>

// Sample test to verify Google Test framework is working
TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

// Test basic arithmetic
TEST(SampleTest, ArithmeticTest) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_NE(2 + 2, 5);
    EXPECT_LT(2, 3);
    EXPECT_GT(3, 2);
}
