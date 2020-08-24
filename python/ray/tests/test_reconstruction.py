import json
import os
import signal
import sys

import numpy as np
import pytest

import ray
from ray.test_utils import (
    wait_for_condition,
    wait_for_pid_to_exit,
)

SIGKILL = signal.SIGKILL if sys.platform != "win32" else signal.SIGTERM


def test_cached_object(ray_start_cluster):
    config = json.dumps({
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    })
    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(num_cpus=0, _internal_config=config)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    @ray.remote
    def dependent_task(x):
        return

    obj = large_object.options(resources={"node1": 1}).remote()
    ray.get(dependent_task.options(resources={"node2": 1}).remote(obj))

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    wait_for_condition(
        lambda: not all(node["Alive"] for node in ray.nodes()), timeout=10)

    for _ in range(20):
        large_object.options(resources={"node2": 1}).remote()

    ray.get(dependent_task.remote(obj))


@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_reconstruction_cached_dependency(ray_start_cluster,
                                          reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(max_retries=0)
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    @ray.remote
    def chain(x):
        return x

    @ray.remote
    def dependent_task(x):
        return

    obj = large_object.options(resources={"node2": 1}).remote()
    obj = chain.options(resources={"node1": 1}).remote(obj)
    ray.get(dependent_task.options(resources={"node1": 1}).remote(obj))

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    wait_for_condition(
        lambda: not all(node["Alive"] for node in ray.nodes()), timeout=10)

    for _ in range(20):
        large_object.options(resources={"node2": 1}).remote()

    if reconstruction_enabled:
        ray.get(dependent_task.remote(obj))
    else:
        with pytest.raises(ray.exceptions.RayTaskError) as e:
            ray.get(dependent_task.remote(obj))
            with pytest.raises(ray.exceptions.UnreconstructableError):
                raise e.as_instanceof_cause()


@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_basic_reconstruction(ray_start_cluster, reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(max_retries=1 if reconstruction_enabled else 0)
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    @ray.remote
    def dependent_task(x):
        return

    obj = large_object.options(resources={"node1": 1}).remote()
    ray.get(dependent_task.options(resources={"node1": 1}).remote(obj))

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)

    if reconstruction_enabled:
        ray.get(dependent_task.remote(obj))
    else:
        with pytest.raises(ray.exceptions.RayTaskError) as e:
            ray.get(dependent_task.remote(obj))
            with pytest.raises(ray.exceptions.UnreconstructableError):
                raise e.as_instanceof_cause()


@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_basic_reconstruction_put(ray_start_cluster, reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(max_retries=1 if reconstruction_enabled else 0)
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    @ray.remote
    def dependent_task(x):
        return x

    obj = ray.put(np.zeros(10**7, dtype=np.uint8))
    result = dependent_task.options(resources={"node1": 1}).remote(obj)
    ray.get(result)
    del obj

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)

    for _ in range(20):
        ray.put(np.zeros(10**7, dtype=np.uint8))

    if reconstruction_enabled:
        ray.get(result)
    else:
        # The copy that we fetched earlier may still be local or it may have
        # been evicted.
        try:
            ray.get(result)
        except ray.exceptions.UnreconstructableError:
            pass


@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_basic_reconstruction_actor_task(ray_start_cluster,
                                         reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 2}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(
        max_restarts=-1,
        max_task_retries=-1 if reconstruction_enabled else 0,
        resources={"node1": 1},
        num_cpus=0)
    class Actor:
        def __init__(self):
            pass

        def large_object(self):
            return np.zeros(10**7, dtype=np.uint8)

        def pid(self):
            return os.getpid()

    @ray.remote
    def dependent_task(x):
        return

    a = Actor.remote()
    pid = ray.get(a.pid.remote())
    obj = a.large_object.remote()
    ray.get(dependent_task.options(resources={"node1": 1}).remote(obj))

    # Workaround to kill the actor process too since there is a bug where the
    # actor's plasma client hangs after the plasma store has exited.
    os.kill(pid, SIGKILL)

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 2}, object_store_memory=10**8)

    wait_for_pid_to_exit(pid)

    if reconstruction_enabled:
        ray.get(dependent_task.remote(obj))
    else:
        with pytest.raises(ray.exceptions.RayTaskError) as e:
            ray.get(dependent_task.remote(obj))
            with pytest.raises(ray.exceptions.UnreconstructableError):
                raise e.as_instanceof_cause()

    # Make sure the actor handle is still usable.
    pid = ray.get(a.pid.remote())


@pytest.mark.skipif(sys.platform == "win32", reason="Test failing on Windows.")
@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_basic_reconstruction_actor_constructor(ray_start_cluster,
                                                reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(max_retries=1 if reconstruction_enabled else 0)
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    # Both the constructor and a method depend on the large object.
    @ray.remote(max_restarts=-1)
    class Actor:
        def __init__(self, x):
            pass

        def dependent_task(self, x):
            return

        def pid(self):
            return os.getpid()

    obj = large_object.options(resources={"node1": 1}).remote()
    a = Actor.options(resources={"node1": 1}).remote(obj)
    ray.get(a.dependent_task.remote(obj))
    pid = ray.get(a.pid.remote())

    # Workaround to kill the actor process too since there is a bug where the
    # actor's plasma client hangs after the plasma store has exited.
    os.kill(pid, SIGKILL)

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)

    wait_for_pid_to_exit(pid)

    # Wait for the actor to restart.
    def probe():
        try:
            ray.get(a.dependent_task.remote(obj))
            return True
        except ray.exceptions.RayActorError:
            return False
        except (ray.exceptions.RayTaskError,
                ray.exceptions.UnreconstructableError):
            return True

    wait_for_condition(probe)

    if reconstruction_enabled:
        ray.get(a.dependent_task.remote(obj))
    else:
        with pytest.raises(ray.exceptions.RayTaskError) as e:
            x = a.dependent_task.remote(obj)
            print(x)
            ray.get(x)
            with pytest.raises(ray.exceptions.UnreconstructableError):
                raise e.as_instanceof_cause()


@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_multiple_downstream_tasks(ray_start_cluster, reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(max_retries=1 if reconstruction_enabled else 0)
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    @ray.remote
    def chain(x):
        return x

    @ray.remote
    def dependent_task(x):
        return

    obj = large_object.options(resources={"node2": 1}).remote()
    downstream = [
        chain.options(resources={
            "node1": 1
        }).remote(obj) for _ in range(4)
    ]
    for obj in downstream:
        ray.get(dependent_task.options(resources={"node1": 1}).remote(obj))

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)

    if reconstruction_enabled:
        for obj in downstream:
            ray.get(dependent_task.options(resources={"node1": 1}).remote(obj))
    else:
        with pytest.raises(ray.exceptions.RayTaskError) as e:
            for obj in downstream:
                ray.get(
                    dependent_task.options(resources={
                        "node1": 1
                    }).remote(obj))
            with pytest.raises(ray.exceptions.UnreconstructableError):
                raise e.as_instanceof_cause()


@pytest.mark.parametrize("reconstruction_enabled", [False, True])
def test_reconstruction_chain(ray_start_cluster, reconstruction_enabled):
    config = {
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    }
    # Workaround to reset the config to the default value.
    if not reconstruction_enabled:
        config["lineage_pinning_enabled"] = 0
    config = json.dumps(config)

    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0,
        _internal_config=config,
        object_store_memory=10**8,
        enable_object_reconstruction=reconstruction_enabled)
    ray.init(address=cluster.address)
    node_to_kill = cluster.add_node(num_cpus=1, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote(max_retries=1 if reconstruction_enabled else 0)
    def large_object():
        return np.zeros(10**7, dtype=np.uint8)

    @ray.remote
    def chain(x):
        return x

    @ray.remote
    def dependent_task(x):
        return x

    obj = large_object.remote()
    for _ in range(20):
        obj = chain.remote(obj)
    ray.get(dependent_task.remote(obj))

    cluster.remove_node(node_to_kill, allow_graceful=False)
    cluster.add_node(num_cpus=1, object_store_memory=10**8)

    if reconstruction_enabled:
        ray.get(dependent_task.remote(obj))
    else:
        with pytest.raises(ray.exceptions.RayTaskError) as e:
            ray.get(dependent_task.remote(obj))
            with pytest.raises(ray.exceptions.UnreconstructableError):
                raise e.as_instanceof_cause()


def test_reconstruction_stress(ray_start_cluster):
    config = json.dumps({
        "num_heartbeats_timeout": 10,
        "raylet_heartbeat_timeout_milliseconds": 100,
        "max_direct_call_object_size": 100,
        "task_retry_delay_ms": 100,
        "initial_reconstruction_timeout_milliseconds": 200,
    })
    cluster = ray_start_cluster
    # Head node with no resources.
    cluster.add_node(
        num_cpus=0, _internal_config=config, enable_object_reconstruction=True)
    ray.init(address=cluster.address)
    # Node to place the initial object.
    node_to_kill = cluster.add_node(
        num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)
    cluster.add_node(
        num_cpus=1, resources={"node2": 1}, object_store_memory=10**8)
    cluster.wait_for_nodes()

    @ray.remote
    def large_object():
        return np.zeros(10**5, dtype=np.uint8)

    @ray.remote
    def dependent_task(x):
        return

    for _ in range(3):
        obj = large_object.options(resources={"node1": 1}).remote()
        ray.get(dependent_task.options(resources={"node2": 1}).remote(obj))

        outputs = [
            large_object.options(resources={
                "node1": 1
            }).remote() for _ in range(1000)
        ]
        outputs = [
            dependent_task.options(resources={
                "node2": 1
            }).remote(obj) for obj in outputs
        ]

        cluster.remove_node(node_to_kill, allow_graceful=False)
        node_to_kill = cluster.add_node(
            num_cpus=1, resources={"node1": 1}, object_store_memory=10**8)

        i = 0
        while outputs:
            ray.get(outputs.pop(0))
            print(i)
            i += 1


if __name__ == "__main__":
    import pytest
    sys.exit(pytest.main(["-v", __file__]))
