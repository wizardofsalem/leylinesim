#include <gtest/gtest.h>
#include <leylib/NodeIdGenerator.h>

TEST(NodeIdGenerator, GeneratesSequentialIdsFromZero) {
  NodeIdGenerator gen;
  EXPECT_EQ(gen.generateId(), 0);
  EXPECT_EQ(gen.generateId(), 1);
  EXPECT_EQ(gen.generateId(), 2);
}

TEST(NodeIdGenerator, ReusesFreedId) {
  NodeIdGenerator gen;
  const uint32_t a = gen.generateId();
  const uint32_t b = gen.generateId();
  (void)a;

  gen.freeId(b);
  EXPECT_EQ(gen.generateId(), b);
}

TEST(NodeIdGenerator, ContinuesAfterReuse) {
  NodeIdGenerator gen;
  gen.generateId();
  const uint32_t one = gen.generateId();
  gen.freeId(one);
  EXPECT_EQ(gen.generateId(), one);
  EXPECT_EQ(gen.generateId(), 2);
}
