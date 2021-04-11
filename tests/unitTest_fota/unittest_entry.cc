/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/

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