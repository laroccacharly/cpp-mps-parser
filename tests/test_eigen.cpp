#include <gtest/gtest.h>
#include <Eigen/Dense>

// Basic test to ensure Eigen and GTest are linked and working
TEST(EigenTest, BasicAssertion) {
    Eigen::MatrixXd m(2, 2);
    m(0, 0) = 3;
    m(1, 0) = 2.5;
    m(0, 1) = -1;
    m(1, 1) = m(1, 0) + m(0, 1);
    ASSERT_EQ(m(1, 1), 1.5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 