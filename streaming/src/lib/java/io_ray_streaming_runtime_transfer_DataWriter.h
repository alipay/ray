/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class io_ray_streaming_runtime_transfer_DataWriter */

#ifndef _Included_io_ray_streaming_runtime_transfer_DataWriter
#define _Included_io_ray_streaming_runtime_transfer_DataWriter
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     io_ray_streaming_runtime_transfer_DataWriter
 * Method:    createWriterNative
 * Signature: (Lio/ray/streaming/runtime/transfer/ChannelCreationParametersBuilder;[[B[JJ[BZ)J
 */
JNIEXPORT jlong JNICALL Java_io_ray_streaming_runtime_transfer_DataWriter_createWriterNative
  (JNIEnv *, jclass, jobject, jobjectArray, jlongArray, jlong, jbyteArray, jboolean);

/*
 * Class:     io_ray_streaming_runtime_transfer_DataWriter
 * Method:    writeMessageNative
 * Signature: (JJJI)J
 */
JNIEXPORT jlong JNICALL Java_io_ray_streaming_runtime_transfer_DataWriter_writeMessageNative
  (JNIEnv *, jobject, jlong, jlong, jlong, jint);

/*
 * Class:     io_ray_streaming_runtime_transfer_DataWriter
 * Method:    stopWriterNative
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_io_ray_streaming_runtime_transfer_DataWriter_stopWriterNative
  (JNIEnv *, jobject, jlong);

/*
 * Class:     io_ray_streaming_runtime_transfer_DataWriter
 * Method:    closeWriterNative
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_io_ray_streaming_runtime_transfer_DataWriter_closeWriterNative
  (JNIEnv *, jobject, jlong);

/*
 * Class:     io_ray_streaming_runtime_transfer_DataWriter
 * Method:    onWriterMessageNative
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_io_ray_streaming_runtime_transfer_DataWriter_onWriterMessageNative
  (JNIEnv *, jobject, jlong, jbyteArray);

/*
 * Class:     io_ray_streaming_runtime_transfer_DataWriter
 * Method:    onWriterMessageSyncNative
 * Signature: (J[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_io_ray_streaming_runtime_transfer_DataWriter_onWriterMessageSyncNative
  (JNIEnv *, jobject, jlong, jbyteArray);

#ifdef __cplusplus
}
#endif
#endif
