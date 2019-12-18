package org.ray.streaming.runtime.master.scheduler.controller;

import java.util.List;

import org.ray.api.RayActor;

import org.ray.streaming.runtime.core.graph.executiongraph.ExecutionVertex;
import org.ray.streaming.runtime.worker.JobWorkerContext;

/**
 * WorkerLifecycleController is responsible for JobWorker Actor's creation and destruction
 */
public interface IWorkerLifecycleController {

  /**
   * Create a worker.
   * @param executionVertex: the specified execution vertex
   * @return true if worker creation succeeded
   */
  boolean createWorker(ExecutionVertex executionVertex);

  /**
   * Stop a worker.
   * @param executionVertex: the specified execution vertex
   * @return true if worker destruction succeeded
   */
  boolean destroyWorker(ExecutionVertex executionVertex);

  /**
   * Create workers.
   * @param executionVertices: the specified execution vertices
   * @return true if workers creation succeeded
   */
  boolean createWorkers(List<ExecutionVertex> executionVertices);

  /**
   * Destroy workers.
   * @param executionVertices: the specified execution vertices
   * @return true if workers destruction succeeded
   */
  boolean destroyWorkers(List<ExecutionVertex> executionVertices);

  /**
   * Init a worker.
   * @param rayActor: target worker's actor
   * @param jobWorkerContext: the worker context
   * @return true if worker creation succeeded
   */
  boolean initWorker(RayActor rayActor, JobWorkerContext jobWorkerContext);

  /**
   * Start a worker.
   * @param rayActor: target worker's actor
   * @return
   */
  boolean startWorker(RayActor rayActor);
}
