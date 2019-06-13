#ifndef RAY_RPC_NODE_MANAGER_SERVER_H
#define RAY_RPC_NODE_MANAGER_SERVER_H

#include "ray/rpc/grpc_server.h"
#include "ray/rpc/server_call.h"

#include "src/ray/protobuf/node_manager.grpc.pb.h"
#include "src/ray/protobuf/node_manager.pb.h"

namespace ray {
namespace rpc {

/// Interface of the `NodeManagerService`, see `src/ray/protobuf/node_manager.proto`.
class NodeManagerServiceHandler {
 public:
  /// Handle a `ForwardTask` request.
  /// The implementation can handle this request asynchronously. When hanling is done, the
  /// `done_callback` should be called.
  ///
  /// \param[in] request The request message.
  /// \param[out] reply The reply message.
  /// \param[in] done_callback The callback to be called when the request is done.
  virtual void HandleForwardTask(const ForwardTaskRequest &request,
                                 ForwardTaskReply *reply,
                                 RequestDoneCallback done_callback) = 0;
};

/// The `GrpcServer` for `NodeManagerService`.
class NodeManagerServer : public GrpcServer {
 public:
  /// Constructor.
  ///
  /// \param[in] port See super class.
  /// \param[in] handler The service handler that actually handle the requests.
  NodeManagerServer(const uint32_t port, NodeManagerServiceHandler &service_handler)
      : GrpcServer("NodeManager", port), service_handler_(service_handler){};

  void RegisterServices(::grpc::ServerBuilder &builder) override {
    /// Register `NodeManagerService`.
    builder.RegisterService(&service_);
  }

  void InitServerCallFactories(
      std::vector<std::unique_ptr<ServerCallFactory>> *server_call_factories) override {
    // Initialize the factory for `ForwardTask` requests.
    std::unique_ptr<ServerCallFactory> forward_task_call_factory(
        new ServerCallFactoryImpl<NodeManagerService, NodeManagerServiceHandler,
                                  ForwardTaskRequest, ForwardTaskReply>(
            service_, &NodeManagerService::AsyncService::RequestForwardTask,
            service_handler_, &NodeManagerServiceHandler::HandleForwardTask, cq_));
    server_call_factories->push_back(std::move(forward_task_call_factory));
  }

 private:
  /// The grpc async service object.
  NodeManagerService::AsyncService service_;
  /// The service handler that actually handle the requests.
  NodeManagerServiceHandler &service_handler_;
};

}  // namespace rpc
}  // namespace ray

#endif
