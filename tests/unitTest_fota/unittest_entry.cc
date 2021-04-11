#include <stdio.h>
#include <stdlib.h>
#include "gtest.h"
#include "localcfg.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  EXPECT_TRUE(getCfgSingleton().load());
  EXPECT_TRUE(getCfgSingleton().bFormatValid());

  return RUN_ALL_TESTS();
}