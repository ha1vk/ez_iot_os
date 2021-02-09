#ifndef H_JSON_PARSER_H_
#define H_JSON_PARSER_H_

#define JSON_PARSER_INTERFACE \
	extern mkernel_internal_error json_parse_devinfo( const char* dev_config_info, dev_basic_info* dev_info); \
	extern mkernel_internal_error json_parse_das_server_info( const char* jsonstring, das_info* das_server_info); \
	extern mkernel_internal_error json_parse_stun_server_info( const char* jsonstring, stun_info* stun_server_info);


#endif