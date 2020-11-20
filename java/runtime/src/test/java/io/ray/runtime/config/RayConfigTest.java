package io.ray.runtime.config;

import io.ray.runtime.generated.Common.WorkerType;
import java.util.Collections;
import org.testng.Assert;
import org.testng.annotations.Test;

public class RayConfigTest {

  @Test
  public void testCreateRayConfig() {
    RayConfig.setMethodLevel(
      "ray.job.code-search-path: path/to/ray/job/resource/path",
      "ray.raylet.config.one: 1",
      "ray.raylet.config.oneString: \"1\"",
      "ray.raylet.config.zero: 0",
      "ray.raylet.config.zero-string: \"0\"",
      "ray.raylet.config.positive-integer: 123",
      "ray.raylet.config.positive-integer-string: \"123\"",
      "ray.raylet.config.negative-integer: -123",
      "ray.raylet.config.negative-integer-string: \"-123\"",
      "ray.raylet.config.float: -123.456",
      "ray.raylet.config.float-string: \"-123.456\"",
      "ray.raylet.config.true: true",
      "ray.raylet.config.true-string: \"true\"",
      "ray.raylet.config.false: false",
      "ray.raylet.config.false-string: \"false\"",
      "ray.raylet.config.string: abc",
      "ray.raylet.config.string-string: \"abc\"");

    try {
      RayConfig rayConfig = RayConfig.create();
      Assert.assertEquals(WorkerType.DRIVER, rayConfig.workerMode);
      Assert.assertEquals(Collections.singletonList("path/to/ray/job/resource/path"),
          rayConfig.codeSearchPath);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("one"), 1);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("one-string"), 1);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("zero"), 0);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("zero-string"), 0);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("positive-integer"), 123);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("positive-integer-string"), 123);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("negative-integer"), -123);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("negative-integer-string"), -123);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("float"), -123.456f);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("float-string"), -123.456f);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("true"), true);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("true-string"), true);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("false"), false);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("false-string"), false);
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("string"), "abc");
      Assert.assertEquals(rayConfig.rayletConfigParameters.get("string-string"), "abc");
    } finally {
      RayConfig.setMethodLevel((String[]) null);
    }
  }
}
