#ifndef _UNITTEST_LOCALCFG_H_
#define _UNITTEST_LOCALCFG_H_

#include <stddef.h>
#include <string>

#define dir_res_name "resources"
#define file_config_name "config"

#define dir_cache_name "cache"
#define file_devinfo_name "devinfo"
#define file_masterkey_name "masterkey"
#define file_devid_name "devid"

bool makeCache();
bool clearCache();
bool deleteFile(const char *filename);
bool isFileExist(const char *filename);
bool setFileValue(const char *filename, const char *buf, size_t size);
bool getFileValue(const char *filename, char *buf, size_t *size);

typedef struct
{
    int keyRef;
    const char *keyName;
    int cjsonType;
} cfgKey_t;

class CCfgBase
{
public:
    virtual ~CCfgBase(){};
    bool load();
    bool save();
    virtual bool bFormatValid() = 0;
    virtual int getItemInt(int key) = 0;
    virtual char *getItemString(int key) = 0;

protected:
    void *m_json_ptr;
    std::string m_file_path;
};

class CCfgLocal : public CCfgBase
{
public:
    enum
    {
        version,
        old_file,
        new_file,
        patch_file,
        buf_max,
        root_cfg_key_max
    } root_cfg_key_e;

    CCfgLocal(const std::string &file_path);
    ~CCfgLocal();
    virtual bool bFormatValid();
    virtual int getItemInt(int key);
    virtual char *getItemString(int key);

private:
    CCfgLocal(){};
    static cfgKey_t keyMaps[];
};

CCfgLocal& getCfgSingleton();

#endif