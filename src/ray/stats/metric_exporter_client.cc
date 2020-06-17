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

#include <algorithm>

#include "ray/stats/metric_exporter_client.h"

namespace ray {
namespace stats {

void StdoutExporterClient::ReportMetrics(const MetricPoints &points) {
  RAY_LOG(DEBUG) << "Metric point size : " << points.size();
}

MetricExporterDecorator::MetricExporterDecorator(
    std::shared_ptr<MetricExporterClient> exporter)
    : exporter_(exporter) {}

void MetricExporterDecorator::ReportMetrics(const MetricPoints &points) {
  if (exporter_) {
    exporter_->ReportMetrics(points);
  }
}
}  // namespace stats
}  // namespace ray
