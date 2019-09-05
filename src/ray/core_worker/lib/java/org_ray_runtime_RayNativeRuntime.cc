#include "ray/core_worker/lib/java/org_ray_runtime_RayNativeRuntime.h"
#include <jni.h>
#include <sstream>
#include "ray/common/id.h"
#include "ray/core_worker/core_worker.h"
#include "ray/core_worker/lib/java/jni_utils.h"

thread_local JNIEnv *local_env = nullptr;
jobject java_task_executor = nullptr;

inline ray::gcs::GcsClientOptions ToGcsClientOptions(JNIEnv *env,
                                                     jobject gcs_client_options) {
  std::string ip = JavaStringToNativeString(
      env, (jstring)env->GetObjectField(gcs_client_options, java_gcs_client_options_ip));
  int port = env->GetIntField(gcs_client_options, java_gcs_client_options_port);
  std::string password = JavaStringToNativeString(
      env,
      (jstring)env->GetObjectField(gcs_client_options, java_gcs_client_options_password));
  return ray::gcs::GcsClientOptions(ip, port, password, /*is_test_client=*/false);
}

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     org_ray_runtime_RayNativeRuntime
 * Method:    nativeInitCoreWorkerProcess
 * Signature:
 * (ILjava/util/Map;Ljava/lang/String;Ljava/lang/String;[BLorg/ray/runtime/gcs/GcsClientOptions;)J
 */
JNIEXPORT jlong JNICALL Java_org_ray_runtime_RayNativeRuntime_nativeInitCoreWorkerProcess(
    JNIEnv *env, jclass, jint workerMode, jobject staticWorkerInfo, jstring storeSocket,
    jstring rayletSocket, jbyteArray jobId, jobject gcsClientOptions) {
  auto static_worker_info = JavaStringMapToNativeStringMap(env, staticWorkerInfo);
  auto native_store_socket = JavaStringToNativeString(env, storeSocket);
  auto native_raylet_socket = JavaStringToNativeString(env, rayletSocket);
  auto job_id = JavaByteArrayToId<ray::JobID>(env, jobId);
  auto gcs_client_options = ToGcsClientOptions(env, gcsClientOptions);

  auto executor_func = [](const ray::RayFunction &ray_function,
                          const std::vector<std::shared_ptr<ray::RayObject>> &args,
                          int num_returns,
                          std::vector<std::shared_ptr<ray::RayObject>> *results) {
    JNIEnv *env = local_env;
    if (!env) {
      // Attach the native thread to JVM.
      auto status =
          jvm->AttachCurrentThreadAsDaemon(reinterpret_cast<void **>(&env), NULL);
      // auto status = jvm->GetEnv(reinterpret_cast<void **>(&env), CURRENT_JNI_VERSION);
      RAY_CHECK(status == JNI_OK) << "Failed to get JNIEnv. Return code: " << status;
      local_env = env;
    }
    RAY_CHECK(env);
    RAY_CHECK(java_task_executor);
    // convert RayFunction
    jobject ray_function_array_list =
        NativeStringVectorToJavaStringList(env, ray_function.function_descriptor);
    // convert args
    // TODO (kfstorm): Avoid copying binary data from Java to C++
    jobject args_array_list = NativeVectorToJavaList<std::shared_ptr<ray::RayObject>>(
        env, args, NativeRayObjectToJavaNativeRayObject);

    // invoke Java method
    jobject java_return_objects =
        env->CallObjectMethod(java_task_executor, java_task_executor_execute,
                              ray_function_array_list, args_array_list);
    std::vector<std::shared_ptr<ray::RayObject>> return_objects;
    JavaListToNativeVector<std::shared_ptr<ray::RayObject>>(
        env, java_return_objects, &return_objects,
        [](JNIEnv *env, jobject java_native_ray_object) {
          return JavaNativeRayObjectToNativeRayObject(env, java_native_ray_object);
        });
    for (auto &obj : return_objects) {
      results->push_back(obj);
    }
    return ray::Status::OK();
  };

  try {
    auto core_worker_process = new ray::CoreWorkerProcess(
        static_cast<ray::WorkerType>(workerMode), ::Language::JAVA, static_worker_info,
        native_store_socket, native_raylet_socket, job_id, gcs_client_options,
        executor_func);
    return reinterpret_cast<jlong>(core_worker_process);
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Failed to construct core worker: " << e.what();
    THROW_EXCEPTION_AND_RETURN_IF_NOT_OK(env, ray::Status::Invalid(oss.str()), 0);
    return 0;  // To make compiler no complain
  }
}

/*
 * Class:     org_ray_runtime_RayNativeRuntime
 * Method:    nativeRunTaskExecutor
 * Signature: (JLorg/ray/runtime/task/TaskExecutor;)V
 */
JNIEXPORT void JNICALL Java_org_ray_runtime_RayNativeRuntime_nativeRunTaskExecutor(
    JNIEnv *env, jclass o, jlong nativeCoreWorkerProcessPointer,
    jobject javaTaskExecutor) {
  java_task_executor = javaTaskExecutor;
  auto core_worker =
      reinterpret_cast<ray::CoreWorkerProcess *>(nativeCoreWorkerProcessPointer);
  core_worker->Execution().Run();
  java_task_executor = nullptr;
}

/*
 * Class:     org_ray_runtime_RayNativeRuntime
 * Method:    nativeDestroyCoreWorkerProcess
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_org_ray_runtime_RayNativeRuntime_nativeDestroyCoreWorkerProcess(
    JNIEnv *env, jclass o, jlong nativeCoreWorkerProcessPointer) {
  delete reinterpret_cast<ray::CoreWorkerProcess *>(nativeCoreWorkerProcessPointer);
}

/*
 * Class:     org_ray_runtime_RayNativeRuntime
 * Method:    nativeSetup
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_ray_runtime_RayNativeRuntime_nativeSetup(JNIEnv *env,
                                                                         jclass,
                                                                         jstring logDir) {
  std::string log_dir = JavaStringToNativeString(env, logDir);
  ray::RayLog::StartRayLog("java_worker", ray::RayLogLevel::INFO, log_dir);
  // TODO (kfstorm): If we add InstallFailureSignalHandler here, Java test may crash.
}

/*
 * Class:     org_ray_runtime_RayNativeRuntime
 * Method:    nativeShutdownHook
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_ray_runtime_RayNativeRuntime_nativeShutdownHook(JNIEnv *,
                                                                                jclass) {
  ray::RayLog::ShutDownRayLog();
}

/*
 * Class:     org_ray_runtime_RayNativeRuntime
 * Method:    nativeSetCoreWorker
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_org_ray_runtime_RayNativeRuntime_nativeSetCoreWorker(
    JNIEnv *env, jclass, jlong nativeCoreWorkerProcessPointer, jbyteArray workerId) {
  const auto worker_id = JavaByteArrayToId<ray::WorkerID>(env, workerId);
  reinterpret_cast<ray::CoreWorkerProcess *>(nativeCoreWorkerProcessPointer)
      ->SetCoreWorker(worker_id);
}

#ifdef __cplusplus
}
#endif
