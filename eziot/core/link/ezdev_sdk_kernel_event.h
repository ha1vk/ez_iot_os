#ifndef H_EZDEV_SDK_KERNEL_EVENT_H_
#define H_EZDEV_SDK_KERNEL_EVENT_H_

#define EZDEV_SDK_KERNEL_EVENT_INTERFACE	\
	extern mkernel_internal_error broadcast_user_start();	\
	extern mkernel_internal_error broadcast_user_stop();	\
	extern mkernel_internal_error broadcast_user_event(sdk_kernel_event_type event_type, void* ctx, EZDEV_SDK_UINT32 ctx_size); \
	extern mkernel_internal_error broadcast_runtime_err(err_tag_e err_tag, ezdev_sdk_kernel_error err_code, void* err_ctx, EZDEV_SDK_UINT32 ctx_size); \
	extern mkernel_internal_error broadcast_user_event_reconnect_success();\
	extern mkernel_internal_error das_keepalive_interval_changed_event_cb(int interval);\

#endif