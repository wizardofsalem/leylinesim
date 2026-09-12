#include <leylib/LeyID.h>
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
  const LeyID a = gen.generateId();
  const LeyID b = gen.generateId();
  (void)a;

  gen.freeId(b);
  EXPECT_EQ(gen.generateId(), b);
}

TEST(NodeIdGenerator, ContinuesAfterReuse) {
  NodeIdGenerator gen;
  gen.generateId();
  const LeyID one = gen.generateId();
  gen.freeId(one);
  EXPECT_EQ(gen.generateId(), one);
  EXPECT_EQ(gen.generateId(), 2);
}
