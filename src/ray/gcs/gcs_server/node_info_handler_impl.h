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

#ifndef RAY_GCS_NODE_INFO_HANDLER_IMPL_H
#define RAY_GCS_NODE_INFO_HANDLER_IMPL_H

#include "gcs_node_manager.h"
#include "ray/gcs/pubsub/gcs_pub_sub.h"
#include "ray/gcs/redis_gcs_client.h"
#include "ray/rpc/gcs_server/gcs_rpc_server.h"

namespace ray {

namespace rpc {

/// This implementation class of `NodeInfoHandler`.
class DefaultNodeInfoHandler : public rpc::NodeInfoHandler {
 public:
  explicit DefaultNodeInfoHandler(gcs::RedisGcsClient &gcs_client,
                                  gcs::GcsNodeManager &gcs_node_manager,
                                  const std::shared_ptr<gcs::RedisClient> &redis_client)
      : gcs_client_(gcs_client),
        gcs_node_manager_(gcs_node_manager),
        node_pub_(redis_client),
        node_resource_pub_(redis_client) {}

  void HandleRegisterNode(const RegisterNodeRequest &request, RegisterNodeReply *reply,
                          SendReplyCallback send_reply_callback) override;

  void HandleUnregisterNode(const UnregisterNodeRequest &request,
                            UnregisterNodeReply *reply,
                            SendReplyCallback send_reply_callback) override;

  void HandleGetAllNodeInfo(const GetAllNodeInfoRequest &request,
                            GetAllNodeInfoReply *reply,
                            SendReplyCallback send_reply_callback) override;

  void HandleReportHeartbeat(const ReportHeartbeatRequest &request,
                             ReportHeartbeatReply *reply,
                             SendReplyCallback send_reply_callback) override;

  void HandleGetResources(const GetResourcesRequest &request, GetResourcesReply *reply,
                          SendReplyCallback send_reply_callback) override;

  void HandleUpdateResources(const UpdateResourcesRequest &request,
                             UpdateResourcesReply *reply,
                             SendReplyCallback send_reply_callback) override;

  void HandleDeleteResources(const DeleteResourcesRequest &request,
                             DeleteResourcesReply *reply,
                             SendReplyCallback send_reply_callback) override;

 private:
  void UnregisterNode(const ClientID &node_id, rpc::GcsNodeInfo &node_info,
                      rpc::UnregisterNodeReply *reply,
                      const SendReplyCallback &send_reply_callback);

  void DeleteResources(const ClientID &node_id,
                       const std::vector<std::string> &resource_names,
                       const gcs::NodeInfoAccessor::ResourceMap &delete_resources,
                       DeleteResourcesReply *reply,
                       const SendReplyCallback &send_reply_callback);

  gcs::RedisGcsClient &gcs_client_;
  gcs::GcsNodeManager &gcs_node_manager_;
  gcs::GcsNodeTablePubSub node_pub_;
  gcs::GcsNodeResourceTablePubSub node_resource_pub_;

  /// Mutex to protect the nodes_cache_ field.
  absl::Mutex node_mutex_;
  /// A mapping from node id to node info.
  std::unordered_map<ClientID, rpc::GcsNodeInfo> nodes_cache_ GUARDED_BY(node_mutex_);
  /// Mutex to protect the nodes_cache_ field.
  absl::Mutex node_resource_mutex_;
  /// A mapping from node id to node resource.
  std::unordered_map<ClientID, gcs::NodeInfoAccessor::ResourceMap> nodes_resource_cache_
      GUARDED_BY(node_resource_mutex_);
};

}  // namespace rpc
}  // namespace ray

#endif  // RAY_GCS_NODE_INFO_HANDLER_IMPL_H
