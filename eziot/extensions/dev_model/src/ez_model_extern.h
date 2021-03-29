#ifndef H_EZ_MODEL_EXTERN_H_
#define H_EZ_MODEL_EXTERN_H_

#include "ez_model_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int ez_model_extern_init();

    void ez_model_extern_deinit();

    int ez_get_list_size();

    int ez_reg_domain(const ez_domain_reg *domain_reg);

    int ez_dereg_domain(const char *domain);

    ez_domain_reg *ez_get_reg_domain(const char *domain);

    int ez_set_data_route_cb(ez_model_default_cb *ez_data_router);

#ifdef __cplusplus
}
#endif

#endif
