

#ifndef H_SHADOW_FUNC_H_
#define H_SHADOW_FUNC_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ez_err_t shadow_protocol_report(ez_char_t *devsn, ez_char_t *res_type, ez_uint16_t index, ez_char_t *domain, ez_char_t *key,
                                    ez_void_t *json_value, ez_uint32_t ver, ez_uint32_t *seq);

    ez_err_t shadow_protocol_query_desired(ez_char_t *devsn, ez_char_t *res_type, ez_int32_t index);

    ez_err_t shadow_protocol_set();

    ez_err_t shadow_protocol_set_reply(ez_char_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_int32_t code, ez_int32_t seq);

#ifdef __cplusplus
}
#endif

#endif
