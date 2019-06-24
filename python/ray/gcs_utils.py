from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import flatbuffers

from ray.core.generated.Language import Language
from ray.core.generated.ray.protocol.Task import Task

from ray.core.generated.gcs_pb2 import (
    ActorCheckpointIdData,
    ClientTableData,
    DriverTableData,
    ErrorTableData,
    ErrorType,
    GcsEntry,
    HeartbeatBatchTableData,
    HeartbeatTableData,
    ObjectTableData,
    ProfileTableData,
    TablePrefix,
    TablePubsub,
    TaskTableData,
)

__all__ = [
    "ActorCheckpointIdData",
    "ClientTableData",
    "DriverTableData",
    "ErrorTableData",
    "ErrorType",
    "GcsEntry",
    "HeartbeatBatchTableData",
    "HeartbeatTableData",
    "Language",
    "ObjectTableData",
    "ProfileTableData",
    "TablePrefix",
    "TablePubsub",
    "Task",
    "TaskTableData",
    "construct_error_message",
]

FUNCTION_PREFIX = "RemoteFunction:"
LOG_FILE_CHANNEL = "RAY_LOG_CHANNEL"
REPORTER_CHANNEL = "RAY_REPORTER"

# xray heartbeats
XRAY_HEARTBEAT_CHANNEL = str(
    TablePubsub.Value('HEARTBEAT_PUBSUB')).encode("ascii")
XRAY_HEARTBEAT_BATCH_CHANNEL = str(
    TablePubsub.Value('HEARTBEAT_BATCH_PUBSUB')).encode("ascii")

# xray driver updates
XRAY_DRIVER_CHANNEL = str(TablePubsub.Value('DRIVER_PUBSUB')).encode("ascii")

# These prefixes must be kept up-to-date with the TablePrefix enum in
# gcs.proto.
# TODO(rkn): We should use scoped enums, in which case we should be able to
# just access the flatbuffer generated values.
TablePrefix_RAYLET_TASK_string = "RAYLET_TASK"
TablePrefix_OBJECT_string = "OBJECT"
TablePrefix_ERROR_INFO_string = "ERROR_INFO"
TablePrefix_PROFILE_string = "PROFILE"


def construct_error_message(driver_id, error_type, message, timestamp):
    """Construct a serialized ErrorTableData object.

    Args:
        driver_id: The ID of the driver that the error should go to. If this is
            nil, then the error will go to all drivers.
        error_type: The type of the error.
        message: The error message.
        timestamp: The time of the error.

    Returns:
        The serialized object.
    """
    data = ErrorTableData()
    data.driver_id = driver_id.binary()
    data.type = error_type
    data.error_message = message
    data.timestamp = timestamp
    return data.SerializeToString()
