SET(CMAKE_SYSTEM_NAME Linux)
SET(TOOCHAIN_C_COMPILER "gcc")
SET(TOOCHAIN_CXX_COMPILER "g++")

#配置使用的SSL库类型,"mbedtls" || "openssl" || ""
SET(SSL_TYPE "mbedtls")

#配置测试的领域模块
SET(UNITTEST_EZPATCH_ENABLE ON)
SET(UNITTEST_BSPATCH_ENABLE ON)

#配置编译选项,"Debug" || "Release"
SET(CMAKE_BUILD_TYPE "Debug")