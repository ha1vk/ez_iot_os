#include "ez_iot_core.h"
#include "ezlog.h"
#include "ezcloud_access.h"
#include "ezos_kv.h"

// ez cloud info
// =================================================
#define EZ_IOT_CLOUD_ENTRY_HOST "test15.ys7.com"
#define EZ_IOT_CLOUD_ENTRY_PORT 8666
// =================================================

// ez iot device info
// =================================================
#define EZ_IOT_DEV_AUTH_MODE 1
#define EZ_IOT_DEV_UUID "4LYV8SK7UKLBOUOVS6HXVX:AFDH8DRTL728"
#define EZ_IOT_DEV_LICENSE "3tYndbcr4hNYdkfn27ZKAG"
#define EZ_IOT_DEV_PRODUCT_KEY "4LYV8SK7UKLBOUOVS6HXVX"
#define EZ_IOT_DEV_NAME "AFDH8DRTL728"
#define EZ_IOT_DEV_DISPLAY_NAME "IOT_UTEST_DEV"
#define EZ_IOT_DEV_ID ""
#define EZ_IOT_DEV_FWVER "V1.2.0 build 201212"
// =================================================
static EZ_INT m_event_id = -1;
static EZ_INT m_last_err = 0;

static ez_int32_t ez_event_notice_func(ez_event_e event_type, void *data, EZ_INT len);

static ez_server_info_t m_lbs_addr = {EZ_IOT_CLOUD_ENTRY_HOST, EZ_IOT_CLOUD_ENTRY_PORT};

static ez_dev_info_t m_dev_info = {
    .auth_mode = EZ_IOT_DEV_AUTH_MODE,
    .dev_subserial = EZ_IOT_DEV_UUID,
    .dev_verification_code = EZ_IOT_DEV_LICENSE,
    .dev_firmwareversion = EZ_IOT_DEV_FWVER,
    .dev_type = EZ_IOT_DEV_PRODUCT_KEY,
    .dev_typedisplay = EZ_IOT_DEV_DISPLAY_NAME,
    .dev_mac = EZ_IOT_DEV_ID,
};

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = ezos_kv_init,
    .ezos_kv_raw_set = ezos_kv_raw_set,
    .ezos_kv_raw_get = ezos_kv_raw_get,
    .ezos_kv_del = ezos_kv_del,
    .ezos_kv_del_by_prefix = ezos_kv_del_by_prefix,
    .ezos_kv_print = ezos_kv_print,
    .ezos_kv_deinit = ezos_kv_deinit,
};

EZ_INT ez_cloud_init()
{
    ez_byte_t devid[32] = {0};

    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(4);

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func);

    return 0;
}

const ez_char_t *ez_cloud_get_sn()
{
    return EZ_IOT_DEV_UUID;
}

const ez_char_t *ez_cloud_get_ver()
{
    return EZ_IOT_DEV_FWVER;
}

const ez_char_t *ez_cloud_get_type()
{
    return EZ_IOT_DEV_PRODUCT_KEY;
}

EZ_INT ez_cloud_start()
{
    return ez_iot_core_start();
}

void ez_cloud_deint()
{
    ez_iot_core_stop();

}


static ez_int32_t ez_event_notice_func(ez_event_e event_type, void *data, EZ_INT len)
{
  switch (event_type)
  {
  case EZ_EVENT_ONLINE:
    m_event_id = EZ_EVENT_ONLINE;
    break;
  case EZ_EVENT_DEVID_UPDATE:
    /* save devid */
    break;

  default:
    break;
  }

  return 0;
}
EZ_INT dev_event_waitfor(EZ_INT event_id, EZ_INT time_ms)
{
  EZ_INT ret = -1;
  EZ_INT index = 0;
  m_event_id = -1;
  m_last_err = 0;

  do
  {
    if (event_id == m_event_id)
    {
      m_last_err = 0;
      ret = 0;
      break;
    }

    ezos_delay_ms(300);
    index += 300;
  } while (index < time_ms);

  return ret;
}

