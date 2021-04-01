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