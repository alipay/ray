// Copyright 2017 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RAY_GCS_PLACEMENT_GROUP_MANAGER_H
#define RAY_GCS_PLACEMENT_GROUP_MANAGER_H

#include <ray/common/id.h>
#include <ray/common/bundle_spec.h>
#include <ray/gcs/accessor.h>
#include <ray/protobuf/gcs.pb.h>
#include <ray/rpc/client_call.h>
#include <ray/rpc/gcs_server/gcs_rpc_server.h>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "ray/gcs/pubsub/gcs_pub_sub.h"
#include "gcs_placement_group_scheduler.h"

namespace ray {
namespace gcs {


/// GcsPlacementGroup just wraps `PlacementGroupTableData` and provides some convenient interfaces to access
/// the fields inside `PlacementGroupTableData`.
/// This class is not thread-safe.
class GcsPlacementGroup {
 public:
  /// Create a GcsPlacementGroup by placement_group_table_data.
  ///
  /// \param placement_group_table_data Data of the placement_group (see gcs.proto).
  explicit GcsPlacementGroup(rpc::PlacementGroupTableData placement_group_table_data)
      : placement_group_table_data_(std::move(placement_group_table_data)) {}

  /// Create a GcsPlacementGroup by CreatePlacementGroupRequest.
  ///
  /// \param request Contains the placement group creation task specification.
  explicit GcsPlacementGroup(const ray::rpc::CreatePlacementGroupRequest &request) {
    const auto &placement_group_spec = request.placement_group_spec();
    placement_group_table_data_.set_placement_group_id(placement_group_spec.placement_group_id());
    placement_group_table_data_.set_max_placement_group_restart(placement_group_spec.max_placement_group_restart());
    placement_group_table_data_.set_num_restarts(0);

    placement_group_table_data_.set_state(rpc::PlacementGroupTableData::PENDING);
    // TODO(AlisaWu) : fix the proto.
     // placement_group_table_data_.set_bundles(placement_group_spec.bundles());
    placement_group_table_data_.set_strategy(placement_group_spec.strategy());
  }
  
  /// Get the immutable PlacementGroupTableData of this placement group.
  const rpc::PlacementGroupTableData &GetPlacementGroupTableData();

  /// Update the `Address` of this placement_group (see gcs.proto).
  // void UpdateAddress(const rpc::Address &address);
  /// Get the `Address` of this placement_group.
  // const rpc::Address &GetAddress() const;

  /// Update the state of this placement_group.
  void UpdateState(rpc::PlacementGroupTableData::PlacementGroupState state);
  /// Get the state of this gcs placement_group.
  rpc::PlacementGroupTableData::PlacementGroupState GetState() const;

  /// Get the id of this placement_group.
  PlacementGroupID GetPlacementGroupID() const;
  /// Get the name of this placement_group.
  std::string GetName() const;

  /// Get the bundles of this placement_group
  std::vector<BundleSpecification> GetBundles() const;
  
 /// Get the Strategy
  rpc::PlacementStrategy GetStrategy() const;

  // const rpc::PlacementGroupTableData &GcsPlacementGroup::GetPlacementGroupTableData() const;
 private:
  /// The placement_group meta data which contains the task specification as well as the state of
  /// the gcs placement_group and so on (see gcs.proto).
  rpc::PlacementGroupTableData placement_group_table_data_;
};

using RegisterPlacementGroupCallback = std::function<void(std::shared_ptr<GcsPlacementGroup>)>;
/// GcsPlacementGroupManager is responsible for managing the lifecycle of all placement group.
/// This class is not thread-safe.
class GcsPlacementGroupManager {
 public:
  /// Create a GcsPlacementGroupManager
  ///
  /// \param scheduler Used to schedule placement group creation tasks.
  /// \param placement_group_info_accessor Used to flush placement group data to storage.
  /// \param gcs_pub_sub Used to publish gcs message.
  explicit GcsPlacementGroupManager(std::shared_ptr<GcsPlacementGroupSchedulerInterface> scheduler,
                  gcs::PlacementGroupInfoAccessor &placement_group_info_accessor,
                  std::shared_ptr<gcs::GcsPubSub> gcs_pub_sub);
  ~GcsPlacementGroupManager() = default;

  /// Register placement_group asynchronously.
  ///
  /// \param request Contains the meta info to create the placement_group.
  /// \param callback Will be invoked after the placement_group is created successfully or be
  /// invoked immediately if the placement_group is already registered to `registered_placement_groups_` and
  /// its state is `ALIVE`.
  /// \return Status::Invalid if this is a named placement_group and an placement_group with the specified
  /// name already exists. The callback will not be called in this case.
  Status RegisterPlacementGroup(const rpc::CreatePlacementGroupRequest &request,
                       RegisterPlacementGroupCallback callback);

  /// Get the placement_group ID for the named placement_group. Returns nil if the placement_group was not found.
  /// \param name The name of the detached placement_group to look up.
  /// \returns PlacementGroupID The ID of the placement_group. Nil if the placement_group was not found.
  PlacementGroupID GetPlacementGroupIDByName(const std::string &name);


  /// Schedule placement_groups in the `pending_placement_groups_` queue.
  /// This method should be called when new nodes are registered or resources
  /// change.
  void SchedulePendingPlacementGroups();

  /// Handle placement_group creation task failure. This should be called when scheduling
  /// an placement_group creation task is infeasible.
  ///
  /// \param placement_group The placement_group whose creation task is infeasible.
  void OnPlacementGroupCreationFailed(std::shared_ptr<GcsPlacementGroup> placement_group);

  /// Handle placement_group creation task success. This should be called when the placement_group
  /// creation task has been scheduled successfully.
  ///
  /// \param placement_group The placement_group that has been created.
  void OnPlacementGroupCreationSuccess(std::shared_ptr<GcsPlacementGroup> placement_group);

  void HandleCreatePlacementGroup(const rpc::CreatePlacementGroupRequest &request, rpc::CreatePlacementGroupReply *reply,
                                  rpc::SendReplyCallback send_reply_callback);

  /// Add a placement group node.
  ///
  /// \param placement_group The info of the placement group to be added.
  void AddPlacementGroup(std::shared_ptr<rpc::PlacementGroupTableData> placement_group); 


  Status ScheduleBundles(const rpc::CreatePlacementGroupRequest &request);

 private:

  /// Callbacks of placement_group registration requests that are not yet flushed.
  /// This map is used to filter duplicated messages from a Driver/Worker caused by some
  /// network problems.
  absl::flat_hash_map<PlacementGroupID, std::vector<RegisterPlacementGroupCallback>>
      placement_group_to_register_callbacks_;
  /// All registered placement_groups (pending placement_groups are also included).
  absl::flat_hash_map<PlacementGroupID, std::shared_ptr<GcsPlacementGroup>> registered_placement_groups_;
  /// Maps detached placement_group names to their placement_group ID for lookups by name.
  absl::flat_hash_map<std::string, PlacementGroupID> named_placement_groups_;
  /// The pending placement_groups which will not be scheduled until there's a resource change.
  std::vector<std::shared_ptr<GcsPlacementGroup>> pending_placement_groups_;
  /// To record the Bundle im which node. 
  absl::flat_hash_map<BundleID,ClientID>schedule_bundle_map_;
  /// The scheduler to schedule all registered placement_groups.
  std::shared_ptr<gcs::GcsPlacementGroupSchedulerInterface> gcs_placement_group_scheduler_;
  /// Placement Group info accessor.
  gcs::PlacementGroupInfoAccessor &placement_group_info_accessor_;
  /// A publisher for publishing gcs messages.
  std::shared_ptr<gcs::GcsPubSub> gcs_pub_sub_;

};

} // namespace gcs
} // namespace gcs


#endif