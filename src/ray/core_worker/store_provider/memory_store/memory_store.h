#ifndef RAY_CORE_WORKER_MEMORY_STORE_H
#define RAY_CORE_WORKER_MEMORY_STORE_H

<<<<<<< HEAD
#include "ray/common/buffer.h"
#include "ray/common/id.h"
#include "ray/common/status.h"
#include "ray/core_worker/common.h"

namespace ray {


class ReferencedRayObject;
class GetOrWaitRequest;
class CoreWorkerMemoryStore;

/// A class that represents an object in memory store.
class ObjectEntry {
 public:
  ObjectEntry(const ObjectID &object_id, const RayObject &object);
  
  std::shared_ptr<RayObject> GetObject() const { return object_; }
  int IncreaseRefcnt() { return ++refcnt_; }
  int DecreaseRefcnt() { return --refcnt_; }
  int Refcnt() { return refcnt_; }

  std::shared_ptr<ReferencedRayObject> CreateReferencedObject(
      std::shared_ptr<CoreWorkerMemoryStore> provider);
 private:
  const ObjectID object_id_;
  std::shared_ptr<RayObject> object_;
  std::atomic<uint16_t> refcnt_;
};

/// This cache currently contains unconsumed objects, which are the objects
/// that have been `put` but haven't been `get`.
class EvictionCache {
 public:
  EvictionCache() {}

  void Add(const ObjectID& key, uint64_t size);

  void Remove(const ObjectID& key);

  uint64_t ChooseObjectsToEvict(uint64_t num_bytes_required,
                               std::vector<ObjectID>* objects_to_evict);

 private:
  /// A doubly-linked list containing the items in the cache.
  typedef std::list<std::pair<ObjectID, uint64_t>> ItemList;
  ItemList item_list_;
  /// A hash table mapping the object ID of an object in the cache to its
  /// location in the doubly linked list item_list_.
  std::unordered_map<ObjectID, ItemList::iterator> item_map_;
};

/// The class provides implementations for local process memory store.
/// An example usage for this is to retrieve the returned objects from direct
/// actor call (see direct_actor_transport.cc).
class CoreWorkerMemoryStore :
    public std::enable_shared_from_this<CoreWorkerMemoryStore> {
 public:
  CoreWorkerMemoryStore(uint64_t max_size);
  ~CoreWorkerMemoryStore() {};

  Status Put(const RayObject &object, const ObjectID &object_id);
  Status Get(const std::vector<ObjectID> &ids, int64_t timeout_ms, 
             std::vector<std::shared_ptr<RayObject>> *results);
  Status Wait(const std::vector<ObjectID> &object_ids,
              int num_objects, int64_t timeout_ms,
              std::vector<bool> *results);
  void Release(const ObjectID &object_id);
  void Delete(const std::vector<ObjectID> &object_ids);
 private:
  Status DeleteObjectImpl(const ObjectID &object_id);
  Status GetOrWait(const std::vector<ObjectID> &ids, int64_t timeout_ms,
                   std::vector<std::shared_ptr<RayObject>> *results,
                   bool is_get);

  const uint64_t max_size_;

  std::mutex lock_;

  uint64_t total_size_;

  std::unordered_map<ObjectID, std::unique_ptr<ObjectEntry>> objects_;
  std::unordered_map<ObjectID, std::vector<std::shared_ptr<GetOrWaitRequest>>> object_get_requests_;

  EvictionCache cache_;

  friend class ObjectEntry;
=======
#include "ray/common/id.h"
#include "ray/common/status.h"
#include "ray/core_worker/common.h"
#include "ray/core_worker/store_provider/store_provider.h"

namespace ray {

class GetRequest;
class CoreWorkerMemoryStore;

/// The class provides implementations for local process memory store.
/// An example usage for this is to retrieve the returned objects from direct
/// actor call (see direct_actor_transport.cc).
class CoreWorkerMemoryStore {
 public:
  CoreWorkerMemoryStore();
  ~CoreWorkerMemoryStore(){};

  /// Put an object with specified ID into object store.
  ///
  /// \param[in] object_id Object ID specified by user.
  /// \param[in] object The ray object.
  /// \return Status.
  Status Put(const ObjectID &object_id, const RayObject &object);

  /// Get a list of objects from the object store.
  ///
  /// \param[in] object_ids IDs of the objects to get. Duplicates are allowed.
  /// \param[in] timeout_ms Timeout in milliseconds, wait infinitely if it's negative.
  /// \param[in] remove_after_get When to remove the objects from store after `Get`
  /// finishes.
  /// \param[out] results Result list of objects data.
  /// \return Status.
  Status Get(const std::vector<ObjectID> &object_ids, int64_t timeout_ms,
             bool remove_after_get, std::vector<std::shared_ptr<RayObject>> *results);

  /// Delete a list of objects from the object store.
  ///
  /// \param[in] object_ids IDs of the objects to delete.
  /// \return Void.
  void Delete(const std::vector<ObjectID> &object_ids);

 private:
  /// Map from object ID to `RayObject`.
  std::unordered_map<ObjectID, std::shared_ptr<RayObject>> objects_;

  /// Map from object ID to its get requests.
  std::unordered_map<ObjectID, std::vector<std::shared_ptr<GetRequest>>>
      object_get_requests_;

  /// Protect the two maps above.
  std::mutex lock_;
>>>>>>> 384cbfb21140aad820b3c72c4624edc3cf08beb2
};

}  // namespace ray

#endif  // RAY_CORE_WORKER_MEMORY_STORE_H
