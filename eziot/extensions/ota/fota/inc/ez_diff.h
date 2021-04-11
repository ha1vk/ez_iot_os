/**
 * @file ez_diff.h
 * @xurongjun (xurongjun@hikvision.com)
 * @brief 实现差分包生成
 * @version 0.1
 * @date 2019-05-29
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#if defined(__linux__)
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)

#ifdef EZDIFF_EXPORTS
#define EZDIFF_API __declspec(dllexport)
#else
#define EZDIFF_API __declspec(dllimport)
#endif

#define CALLBACK __stdcall

#else
#define EZDIFF_API

#define CALLBACK
#define __stdcall

#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief generate diff packet 
     * 
     * @param old_path original package path
     * @param new_path new package path
     * @param patch_path 生成的差分包路径
     * @param level zip level(1-9), The higher the level is, the more memory will be needed during merging.
     * @return 0 success !0 failed 
     */
    EZDIFF_API int ez_diff(const char *old_path, const char *new_path, const char *patch_path, int level);

#ifdef __cplusplus
}
#endif