#ifndef H_ASE_SUPPORT_H_
#define H_ASE_SUPPORT_H_


#define ASE_SUPPORT_INTERFACE	\
	extern void buf_padding(unsigned char* buf, EZDEV_SDK_INT32 padding_len ,EZDEV_SDK_INT32 len); \
	extern EZDEV_SDK_UINT32 calculate_padding_len(EZDEV_SDK_UINT32 len); \
	extern mkernel_internal_error aes_cbc_128_dec_padding(const unsigned char aes_key[16], \
													  const unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, \
													  unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len); \
	extern mkernel_internal_error aes_cbc_128_enc_padding(const unsigned char aes_key[16], \
													  unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, EZDEV_SDK_UINT32 input_length_padding, \
													  unsigned char *output_buf, EZDEV_SDK_UINT32 *output_length); \
    extern mkernel_internal_error aes_gcm_128_enc_padding(const unsigned char gcm_key[16], \
                                                        unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, \
                                                        unsigned char *output_buf, EZDEV_SDK_UINT32 *output_length, \
                                                        unsigned char* output_tag_buf, EZDEV_SDK_UINT32 tag_buf_len); \
    extern mkernel_internal_error aes_gcm_128_dec_padding(const unsigned char gcm_key[16], \
                                                        const unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, \
                                                        unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len, \
                                                        unsigned char* input_tag_buf, EZDEV_SDK_UINT32 tag_buf_len); \


#endif