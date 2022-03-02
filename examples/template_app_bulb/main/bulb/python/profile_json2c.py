import json
from string import Template

str_pro_up_funcs=""
					
str_pro_up_body = Template("{\n\tez_int32_t rv = -1;\n\n" + \
					"\tdo\n" + "\t{\n" + \
					"\t\tif (NULL == p_stru_key_value || NULL == p_stru_key_value->value)\n" + \
					"\t\t{\n" + \
					"\t\t\tbreak;\n" + \
					"\t\t}\n" + \
					"\t\t//todo:获取实际值上传\n" + \
					"\t\tp_stru_key_value->type = $profile_key_type;\n" + \
					"\t\t//完善此部分...\n" + \
					"\t\tprintf(\"\\n to_do DEBUG in line (%d) and function (%s)): \\n \", __LINE__, __func__);\n"+ \
					"\t\trv = EZ_BASE_ERR_SUCC;\n\t}while(0);\n" + \
					"\treturn rv;\n}")
					
str_event_up_funcs=""
str_pro_handle_funcs=""

str_pro_set_body = Template("{\n\tif (NULL == p_stru_key_value || $profile_key_type != p_stru_key_value->type)\n" + \
					"\t{\n" + "\t\treturn -1;\n\t}\n" + \
					"\t//todo:应用层业务处理\n" + \
					"\tprintf(\"\\n to_do DEBUG in line (%d) and function (%s)): \\n \", __LINE__, __func__);\n"+ \
					"\t//完善此部分...\n" + \
					"\treturn EZ_BASE_ERR_SUCC;\n}")
					
str_atc_handle_funcs=""
str_event_handle_funcs=""
str_pro_cmd=""
str_action_cmd=""
str_event_cmd=""
file_in=open('profile.json',encoding='utf-8')
ret=file_in.read()
total=json.loads(ret)
array_resources=total['resources']
obj_resources=array_resources[0]
resources_type = obj_resources['resourceCategory']
resources_id_array = obj_resources['localIndex']
resources_id = resources_id_array[0]
array_domains=obj_resources['domains']
array_domains.sort(key = lambda x:x["identifier"])
for i in range(len(array_domains)):
    data=array_domains[i]
    domain=data['identifier']
    if(data['props']!=[]):
        array_property=data['props']
        array_property.sort(key = lambda x:x["identifier"])
        for i in range(len(array_property)):
          key=array_property[i]
          schema=key['schema']
          type=""
          if(schema['type']=='boolean'):
           type="EZ_TSL_DATA_TYPE_BOOL"
          elif(schema['type']=='integer'):
           type="EZ_TSL_DATA_TYPE_INT"
          elif(schema['type']=='number'):
           type="EZ_TSL_DATA_TYPE_DOUBLE"
          elif(schema['type']=='string'):
           type="EZ_TSL_DATA_TYPE_STRING"
          elif(schema['type']=='array'):
           type="EZ_TSL_DATA_TYPE_ARRAY"
          elif(schema['type']=='object'):
           type="EZ_TSL_DATA_TYPE_OBJECT"
          str_pro_handle_funcs=str_pro_handle_funcs+"static ez_int32_t property_" + key['identifier'].lower() + "_set(ez_tsl_value_t *p_stru_key_value)\n" + str_pro_set_body.substitute(profile_key_type = type) + "\n\n"
          str_pro_up_funcs=str_pro_up_funcs + "static ez_int32_t property_" + key['identifier'].lower() + "_up(ez_tsl_value_t *p_stru_key_value)\n" + \
                            str_pro_up_body.substitute(profile_key = key['identifier'].lower(), profile_key_type = type) + "\n\n"
          str_pro_cmd=str_pro_cmd+"\t{"+ "\"" + key['identifier'] + "\"," + \
                       "\"" + domain + "\"," + \
                       "\"" + resources_type + "\"," + \
                       "\"" +  resources_id + "\"," +  "property_" + key['identifier'].lower() + "_set," + "property_" + key['identifier'].lower() + "_up},\n"
		  
    if(data['actions']!=[]):
        
        action=data['actions']
        action.sort(key = lambda x:x["identifier"])
        for i in range(len(action)):
          key=action[i]
          str_atc_handle_funcs=str_atc_handle_funcs+"static void action_"+key['identifier'].lower()+"(char *data,char *value)\n{\n\n\n"+"\n}\n"
          str_action_cmd=str_action_cmd+"    {"+"\""+data['identifier']+"\""+","+"\""+key['identifier'].lower()+"\""+","+"action_"+key['identifier'].lower() + "},\n" 
		  
    if(data['events']!=[]):
        event=data['events']
        event.sort(key = lambda x:x["identifier"])
        for i in range(len(event)):
          key=event[i]
          str_event_handle_funcs=str_event_handle_funcs+"static void fn_event_"+key['identifier']+"(char *data,char *value)\n{\n\n\n"+"\n}\n"
          str_event_up_funcs=str_event_up_funcs+"void fn_event_"+key['identifier']+"_sync (char *data)\n{\n    char szIdentifier[32]="+"\""+key['identifier']+"\";\n\n    event_up_handler_entry(szIdentifier,data);\n}\n"
          str_event_cmd=str_event_cmd+"    {"+"\""+data['identifier']+"\""+","+"\""+key['identifier']+"\""+","+"fn_event_"+key['identifier']+"},\n"
		  
str_pro_cmd=str_pro_cmd+"    {NULL,NULL,NULL,NULL,NULL,NULL}"
str_action_cmd=str_action_cmd+"    {NULL,NULL,NULL}"
str_event_cmd=str_event_cmd+"    {NULL,NULL,NULL}"

#模版文件
file_in=open('template_header.txt',encoding='utf-8')
template_header=file_in.read()

file_in=open('template_fixed.txt',encoding='utf-8')
template_fixed=file_in.read()

file_out=open('bulb_protocol_tsl_parse.c',"w")

file_out.write(template_header
              +"\n"+str_pro_up_funcs
              +"\n"+str_event_up_funcs
              +"\n//用户可在这些函数中让设备执行//"
              +"\n"+str_pro_handle_funcs
              +"\n"+str_atc_handle_funcs
              +"\n"+str_event_handle_funcs
              +"\n//功能点数组用来找功能点//"
              +"\nproperty_cmd_t property_cmd[]=\n{\n"+str_pro_cmd+"\n};\n"
              +"\naction_cmd_t action_cmd[]=\n{\n"+str_action_cmd+"\n};\n\n"
			  +template_fixed)
file_in.close()
file_out.close()
