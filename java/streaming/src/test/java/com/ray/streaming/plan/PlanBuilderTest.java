package com.ray.streaming.plan;


import com.beust.jcommander.internal.Lists;
import com.ray.streaming.api.context.RayContext;
import com.ray.streaming.api.partition.impl.KeyPartition;
import com.ray.streaming.api.partition.impl.RandomPartition;
import com.ray.streaming.api.stream.DataStream;
import com.ray.streaming.api.stream.StreamSink;
import com.ray.streaming.api.stream.StreamSource;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.testng.Assert;
import org.testng.annotations.Test;

public class PlanBuilderTest {

  private static final Logger LOGGER = LoggerFactory.getLogger(PlanBuilderTest.class);

  @Test
  public void testDataSync() {
    Plan plan = buildDataSyncPlan();
    List<PlanVertex> planVertexList = plan.getPlanVertexList();
    List<PlanEdge> planEdgeList = plan.getPlanEdgeList();

    Assert.assertEquals(planVertexList.size(), 2);
    Assert.assertEquals(planEdgeList.size(), 1);

    PlanEdge planEdge = planEdgeList.get(0);
    Assert.assertEquals(planEdge.getPartition().getClass(), RandomPartition.class);

    PlanVertex sinkVertex = planVertexList.get(1);
    PlanVertex sourceVertex = planVertexList.get(0);
    Assert.assertEquals(sinkVertex.getVertexType(), VertexType.SINK);
    Assert.assertEquals(sourceVertex.getVertexType(), VertexType.SOURCE);

  }

  public Plan buildDataSyncPlan() {
    RayContext rayContext = RayContext.buildContext();
    DataStream<String> dataStream = StreamSource.buildSource(rayContext,
        Lists.newArrayList("a", "b", "c"));
    StreamSink streamSink = dataStream.sink(x -> LOGGER.info(x));
    PlanBuilder planBuilder = new PlanBuilder(Lists.newArrayList(streamSink));

    Plan plan = planBuilder.buildPlan();
    return plan;
  }

  @Test
  public void testKeyByPlan() {
    Plan plan = buildKeyByPlan();
    List<PlanVertex> planVertexList = plan.getPlanVertexList();
    List<PlanEdge> planEdgeList = plan.getPlanEdgeList();

    Assert.assertEquals(planVertexList.size(), 3);
    Assert.assertEquals(planEdgeList.size(), 2);

    PlanVertex source = planVertexList.get(0);
    PlanVertex map = planVertexList.get(1);
    PlanVertex sink = planVertexList.get(2);

    Assert.assertEquals(source.getVertexType(), VertexType.SOURCE);
    Assert.assertEquals(map.getVertexType(), VertexType.PROCESS);
    Assert.assertEquals(sink.getVertexType(), VertexType.SINK);

    PlanEdge keyBy2Sink = planEdgeList.get(0);
    PlanEdge source2KeyBy = planEdgeList.get(1);

    Assert.assertEquals(keyBy2Sink.getPartition().getClass(), KeyPartition.class);
    Assert.assertEquals(source2KeyBy.getPartition().getClass(), RandomPartition.class);
  }

  public Plan buildKeyByPlan() {
    RayContext rayContext = RayContext.buildContext();
    DataStream<String> dataStream = StreamSource.buildSource(rayContext,
        Lists.newArrayList("1", "2", "3", "4"));
    StreamSink streamSink = dataStream.keyBy(x -> x)
        .sink(x -> LOGGER.info(x));
    PlanBuilder planBuilder = new PlanBuilder(Lists.newArrayList(streamSink));

    Plan plan = planBuilder.buildPlan();
    return plan;
  }

}