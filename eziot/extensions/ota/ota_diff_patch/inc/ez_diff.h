/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/
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
     * @param patch_path 
     * @param level zip level(1-9), The higher the level is, the more memory will be needed during merging.
     * @return 0 success !0 failed 
     */
    EZDIFF_API int ez_diff(const char *old_path, const char *new_path, const char *patch_path, int level);

#ifdef __cplusplus
}
#endif