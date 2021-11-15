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

#include "sdk_kernel_def.h"
#include <stdarg.h>
#include <stdlib.h>

ezdev_sdk_kernel g_ezdev_sdk_kernel;

ezdev_sdk_kernel* get_ezdev_sdk_kernel()
{
	return &g_ezdev_sdk_kernel;
}