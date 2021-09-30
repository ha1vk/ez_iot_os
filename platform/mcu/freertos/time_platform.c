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

#include "ez_sdk_log.h"

#include "time_interface.h"


int ez_get_clock_time(ez_timespec *clock)
{
	
}

time_t ez_get_time_stamp(time_t *time)
{

}

int ez_set_time_stamp(time_t *time)
{

}

int ez_get_rtc_time(ez_rtc_time_t *rtc)
{

}

int ez_set_rtc_time(ez_rtc_time_t *rtc)
{

}


void ez_delay_ms(unsigned int time_ms)
{
	usleep((int)(time_ms * 1000));
}
