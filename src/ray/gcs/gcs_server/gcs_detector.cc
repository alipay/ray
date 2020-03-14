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

#include "gcs_detector.h"
#include "ray/common/ray_config.h"
#include "ray/gcs/redis_gcs_client.h"

namespace ray {
namespace gcs {

GcsDetector::GcsDetector(boost::asio::io_service &io_service,
                         std::shared_ptr<gcs::RedisGcsClient> gcs_client,
                         std::function<void()> callback)
    : gcs_client_(std::move(gcs_client)),
      detect_timer_(io_service),
      callback_(std::move(callback)) {
  Start();
}

void GcsDetector::Start() {
  RAY_LOG(INFO) << "Starting gcs detector.";
  Tick();
}

void GcsDetector::DetectRedis() {
  redisReply *reply = reinterpret_cast<redisReply *>(
      redisCommand(gcs_client_->primary_context()->sync_context(), "PING"));
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    RAY_LOG(ERROR) << "Redis is inactive.";
    callback_();
  } else {
    freeReplyObject(reply);
  }
}

/// A periodic timer that checks for timed out clients.
void GcsDetector::Tick() {
  DetectRedis();
  ScheduleTick();
}

void GcsDetector::ScheduleTick() {
  auto detect_period = boost::posix_time::milliseconds(
      RayConfig::instance().gcs_detect_timeout_milliseconds());
  detect_timer_.expires_from_now(detect_period);
  detect_timer_.async_wait([this](const boost::system::error_code &error) {
    if (error == boost::system::errc::operation_canceled) {
      return;
    }
    RAY_CHECK(!error) << "Detecting gcs failed with error: " << error.message();
    Tick();
  });
}

}  // namespace gcs
}  // namespace ray