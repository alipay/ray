/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_ray_spi_impl_RedisClient */

#ifndef _Included_org_ray_spi_impl_RedisClient
#define _Included_org_ray_spi_impl_RedisClient
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_ray_spi_impl_RedisClient
 * Method:    connect
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_org_ray_spi_impl_RedisClient_connect
  (JNIEnv *, jclass, jstring, jint);

/*
 * Class:     org_ray_spi_impl_RedisClient
 * Method:    disconnect
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_ray_spi_impl_RedisClient_disconnect
  (JNIEnv *, jclass, jint);

/*
 * Class:     org_ray_spi_impl_RedisClient
 * Method:    execute_command
 * Signature: (Ljava/lang/String;I[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_ray_spi_impl_RedisClient_execute_1command
  (JNIEnv *, jclass, jint, jstring, jint, jbyteArray);

#ifdef __cplusplus
}
#endif
#endif
