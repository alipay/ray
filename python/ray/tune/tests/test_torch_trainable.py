import os
import pytest
from unittest.mock import patch
import torch
import torch.distributed as dist

import ray
from ray import tune
from ray.tune.integration.torch import (DistributedTrainableCreator,
                                        distributed_checkpoint_dir,
                                        _train_simple, _train_check_global)


@pytest.fixture
def ray_start_2_cpus():
    address_info = ray.init(num_cpus=2)
    yield address_info
    # The code after the yield will run as teardown code.
    ray.shutdown()
    # Ensure that tests don't ALL fail
    if dist.is_initialized():
        dist.destroy_process_group()


@pytest.fixture
def ray_start_4_cpus():
    address_info = ray.init(num_cpus=4)
    yield address_info
    # The code after the yield will run as teardown code.
    ray.shutdown()
    # Ensure that tests don't ALL fail
    if dist.is_initialized():
        dist.destroy_process_group()


def test_single_step(ray_start_2_cpus):  # noqa: F811
    trainable_cls = DistributedTrainableCreator(_train_simple, num_workers=2)
    trainer = trainable_cls()
    trainer.train()
    trainer.stop()


def test_step_after_completion(ray_start_2_cpus):  # noqa: F811
    trainable_cls = DistributedTrainableCreator(_train_simple, num_workers=2)
    trainer = trainable_cls(config={"epochs": 1})
    with pytest.raises(RuntimeError):
        for i in range(10):
            trainer.train()


def test_validation(ray_start_2_cpus):  # noqa: F811
    def bad_func(a, b, c):
        return 1

    with pytest.raises(ValueError):
        DistributedTrainableCreator(bad_func, num_workers=2)


def test_set_global(ray_start_2_cpus):  # noqa: F811
    trainable_cls = DistributedTrainableCreator(
        _train_check_global, num_workers=2)
    trainable = trainable_cls()
    result = trainable.train()
    assert result["is_distributed"]


def test_save_checkpoint(ray_start_2_cpus):  # noqa: F811
    trainable_cls = DistributedTrainableCreator(_train_simple, num_workers=2)
    trainer = trainable_cls(config={"epochs": 1})
    trainer.train()
    checkpoint_dir = trainer.save()
    model_state_dict, opt_state_dict = torch.load(
        os.path.join(checkpoint_dir, "checkpoint"))
    trainer.stop()


@pytest.mark.parametrize("enabled_checkpoint", [True, False])
def test_simple_tune(ray_start_4_cpus, enabled_checkpoint):
    trainable_cls = DistributedTrainableCreator(_train_simple, num_workers=2)
    analysis = tune.run(
        trainable_cls,
        config={"enable_checkpoint": enabled_checkpoint},
        num_samples=2,
        stop={"training_iteration": 2})
    assert analysis.trials[0].last_result["training_iteration"] == 2
    assert analysis.trials[0].has_checkpoint() == enabled_checkpoint


@pytest.mark.parametrize("rank", [0, 1])
def test_checkpoint(ray_start_2_cpus, rank):  # noqa: F811
    with patch("torch.distributed.get_rank") as rank_method:
        rank_method.return_value = rank
        with distributed_checkpoint_dir(step="test") as path:
            if rank == 0:
                assert path
        if rank != 0:
            assert not os.path.exists(path)


if __name__ == "__main__":
    import pytest
    import sys
    sys.exit(pytest.main(["-v", __file__]))
