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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef _BASE_SAMPLE_H
#define _BASE_SAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

	int base_sample_start();

	int base_sample_stop();

	int base_sample_query_user_id();

	int base_sample_set_operation_code();

#ifdef __cplusplus
}
#endif

#endif