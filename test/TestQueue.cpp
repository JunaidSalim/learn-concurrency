#include <gtest/gtest.h>
#include <memory>
#include "threadSafeQueue.h"

class QueueTest : public ::testing::Test {
 protected:
  threadSafeQueue<int> q;
};

TEST_F(QueueTest, TestPush) {
  EXPECT_TRUE(q.empty());
  q.push(1);
  EXPECT_FALSE(q.empty());
}

TEST_F(QueueTest, TestWaitAndPop) {
  q.push(1);
  EXPECT_FALSE(q.empty());
  q.wait_and_pop();
  EXPECT_TRUE(q.empty());
}

TEST_F(QueueTest, TestWaitAndPopReturnsValue) {
  q.push(5);
  std::shared_ptr<int> p = q.wait_and_pop();
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(*p, 5);
}

TEST_F(QueueTest, TestWaitAndPopByReference) {
  q.push(7);
  int v = 0;
  q.wait_and_pop(v);
  EXPECT_EQ(v, 7);
  EXPECT_TRUE(q.empty());
}

TEST_F(QueueTest, TestTryPopByReference) {
  q.push(3);
  int v = 0;
  EXPECT_TRUE(q.try_pop(v));
  EXPECT_EQ(v, 3);
  EXPECT_TRUE(q.empty());
}

TEST_F(QueueTest, TestTryPopReturnsPointer) {
  q.push(4);
  std::shared_ptr<int> p = q.try_pop();
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(*p, 4);
}

TEST_F(QueueTest, TestTryPopOnEmpty) {
  int v = 0;
  EXPECT_FALSE(q.try_pop(v));
  EXPECT_EQ(q.try_pop(), nullptr);
}

TEST_F(QueueTest, TestFifoOrder) {
  q.push(1);
  q.push(2);
  q.push(3);
  int v = 0;
  q.try_pop(v);
  EXPECT_EQ(v, 1);
  q.try_pop(v);
  EXPECT_EQ(v, 2);
  q.try_pop(v);
  EXPECT_EQ(v, 3);
}

TEST_F(QueueTest, TestCopyConstructor) {
  q.push(1);
  q.push(2);
  threadSafeQueue<int> copy(q);
  int v = 0;
  copy.try_pop(v);
  EXPECT_EQ(v, 1);
  EXPECT_FALSE(q.empty());
}