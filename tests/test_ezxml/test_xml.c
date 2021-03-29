#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ezxml.h"

int main()
{
  ezxml_t req;
  ezxml_t rsp;
  ezxml_t req1;
  char *strrsp = NULL;
  char *strreq = NULL;
  char sCode[64]={0};
  FILE *pf = NULL;
  do
  {
    pf = fopen("testxml.xml", "r");
    if (NULL == pf)
    {
      printf("fopen error\n");
      break;
    }
    req  = ezxml_parse_fp(pf);
    if(NULL == req)
    {
      printf("ezxml_parse_fp error\n");
      break;
    }
    ezxml_t code = ezxml_child(req,"Code");
    if(code)
    {    
      strncpy(sCode, ezxml_txt(code), sizeof(sCode) -1);
      printf("sCode:%s\n", sCode);
    }
    ezxml_t base =  ezxml_child(req,"base");
    if(base)
    {
        ezxml_t res=ezxml_child(base,"rsp");
        if(res)
        {
            char* sbase = ezxml_txt(res);
            printf("sbase:%s\n", sbase);
        }
       
    }
    char userid[64]={0};
    char name[64]={0};
    ezxml_t User = ezxml_child(req, "User");
    if(NULL!=User)
    {
        if(NULL!=ezxml_attr(User,"Id"))
        {
            strncpy(userid, ezxml_attr(User,"Id"), sizeof(userid) -1);
            printf("Id:%s\n",userid);
        }
        if(NULL!=ezxml_attr(User,"Name"))
        {
            strncpy(name, ezxml_attr(User,"Name"), sizeof(name) -1);
            printf("Name:%s\n",name);
        }
    }
    strreq = ezxml_toxml(req);
    if (NULL == strreq)
    {
      printf("ezxml_toxml strreq error\n");
      break;
    }
    int len = strlen(strreq);
    printf("strreq:%s ,len:%d \n", strreq, len);
    rsp = ezxml_new("Response");
    if (NULL == rsp)
    {
      printf("ezxml_new error\n");
      break;
    }
    req1 = ezxml_parse_str(strreq, len);
    if(NULL == req1)
    {
      printf("ezxml_parse_strerror\n");
      break;
    }
    ezxml_t result = ezxml_add_child(rsp, "Result", 1);
    char buf[32] = {0};
    sprintf(buf, "%d", 10);
    ezxml_set_txt(result, buf);

    strrsp = ezxml_toxml(rsp);
    if (NULL == strrsp)
    {
       printf("ezxml_toxml error\n");
        break;
    }
    printf("strrsp:%s\n", strrsp);
  } while (0);

  if(req)
  {
    ezxml_free(req);
  }
  if(req1)
  {
    ezxml_free(req1);
  }
  if(rsp)
  {
    ezxml_free(rsp);
  }
  if(strrsp)
  {
    free(strrsp);
    strrsp = NULL;
  }
  if(strreq)
  {
    free(strreq);
    strreq = NULL;
  }
  if(pf)
  {
     fclose(pf);
  }
  return 0;
}