package org.ray.streaming.runtime.master;

import java.io.Serializable;

import com.google.common.base.MoreObjects;

import org.ray.streaming.runtime.config.StreamingConfig;
import org.ray.streaming.runtime.core.graph.Graphs;
import org.ray.streaming.runtime.core.graph.executiongraph.ExecutionGraph;
import org.ray.streaming.runtime.core.graph.jobgraph.JobGraph;
import org.ray.streaming.runtime.core.resource.Resources;

public class JobMasterRuntimeContext implements Serializable {

  private StreamingConfig conf;

  private volatile Graphs graphs;
  private volatile Resources resources;

  public volatile JobMasterRuntimeStatus status = JobMasterRuntimeStatus.NONE;

  public JobMasterRuntimeContext(StreamingConfig conf) {
    this.conf = conf;
    this.resources = new Resources(conf.masterConfig.resourceConfig);
  }

  public String getJobName() {
    return conf.masterConfig.commonConfig.jobName();
  }

  public StreamingConfig getConf() {
    return conf;
  }

  public Graphs getGraphs() {
    return graphs;
  }

  public void setGraphs(JobGraph jobGraph, ExecutionGraph executionGraph) {
    this.graphs = new Graphs(jobGraph, executionGraph);
  }

  public Resources getResources() {
    return resources;
  }

  public void setResources(Resources resources) {
    this.resources = resources;
  }

  @Override
  public String toString() {
    return MoreObjects.toStringHelper(this)
        .add("status", status)
        .add("graphs", graphs.getExecutionGraph())
        .add("resources", resources)
        .add("conf", conf.getMap())
        .toString();
  }
}