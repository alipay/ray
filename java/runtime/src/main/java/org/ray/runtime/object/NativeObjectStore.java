package org.ray.runtime.object;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import org.ray.api.id.BaseId;
import org.ray.api.id.ObjectId;
import org.ray.runtime.context.WorkerContext;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Object store methods for cluster mode. This is a wrapper class for core worker object interface.
 */
public class NativeObjectStore extends ObjectStore {

  private static final Logger LOGGER = LoggerFactory.getLogger(NativeObjectStore.class);

  /**
   * The native pointer of core worker.
   */
  private final long nativeCoreWorkerPointer;

  public NativeObjectStore(WorkerContext workerContext, long nativeCoreWorkerPointer) {
    super(workerContext);
    this.nativeCoreWorkerPointer = nativeCoreWorkerPointer;
  }

  @Override
  public ObjectId putRaw(NativeRayObject obj, List<ObjectId> containedObjectIds) {
    return new ObjectId(nativePut(nativeCoreWorkerPointer, obj, toBinaryList(containedObjectIds)));
  }

  @Override
  public void putRaw(NativeRayObject obj, ObjectId objectId, List<ObjectId> containedObjectIds) {
    nativePut(nativeCoreWorkerPointer, objectId.getBytes(), obj, toBinaryList(containedObjectIds));
  }

  @Override
  public List<NativeRayObject> getRaw(List<ObjectId> objectIds, long timeoutMs) {
    return nativeGet(nativeCoreWorkerPointer, toBinaryList(objectIds), timeoutMs);
  }

  @Override
  public List<Boolean> wait(List<ObjectId> objectIds, int numObjects, long timeoutMs) {
    return nativeWait(nativeCoreWorkerPointer, toBinaryList(objectIds), numObjects, timeoutMs);
  }

  @Override
  public void delete(List<ObjectId> objectIds, boolean localOnly, boolean deleteCreatingTasks) {
    nativeDelete(nativeCoreWorkerPointer, toBinaryList(objectIds), localOnly, deleteCreatingTasks);
  }

  @Override
  public void addLocalReference(ObjectId objectId) {
    nativeAddLocalReference(nativeCoreWorkerPointer, objectId.getBytes());
  }

  @Override
  public void removeLocalReference(ObjectId objectId) {
    nativeRemoveLocalReference(nativeCoreWorkerPointer, objectId.getBytes());
  }

  public Map<ObjectId, long[]> getAllReferenceCounts() {
    Map<ObjectId, long[]> referenceCounts = new HashMap<>();
    for (Map.Entry<byte[], long[]> entry :
        nativeGetAllReferenceCounts(nativeCoreWorkerPointer).entrySet()) {
      referenceCounts.put(new ObjectId(entry.getKey()), entry.getValue());
    }
    return referenceCounts;
  }

  private static List<byte[]> toBinaryList(List<ObjectId> ids) {
    return ids.stream().map(BaseId::getBytes).collect(Collectors.toList());
  }

  private static native byte[] nativePut(long nativeCoreWorkerPointer, NativeRayObject obj,
                                         List<byte[]> containedObjectIds);

  private static native void nativePut(long nativeCoreWorkerPointer, byte[] objectId,
      NativeRayObject obj, List<byte[]> containedObjectIds);

  private static native List<NativeRayObject> nativeGet(long nativeCoreWorkerPointer,
      List<byte[]> ids, long timeoutMs);

  private static native List<Boolean> nativeWait(long nativeCoreWorkerPointer,
      List<byte[]> objectIds, int numObjects, long timeoutMs);

  private static native void nativeDelete(long nativeCoreWorkerPointer, List<byte[]> objectIds,
      boolean localOnly, boolean deleteCreatingTasks);

  private static native void nativeAddLocalReference(long nativeCoreWorkerPointer, byte[] objectId);

  private static native void nativeRemoveLocalReference(long nativeCoreWorkerPointer,
      byte[] objectId);

  private static native Map<byte[], long[]> nativeGetAllReferenceCounts(
      long nativeCoreWorkerPointer);
}
