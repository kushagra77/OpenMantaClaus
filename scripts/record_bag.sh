#!/bin/bash

# List of topics to record for system operation
REQUIRED_TOPICS=(
    "/camera/img"
    "/camera/debug_img"
    "/mavros/state"
    "/mavros/imu/data"
    "/mavros/rc/in"
    "cv/feature_observations"
    "/tf"
    "/tf_static"
    # "/mavros/battery" # no sensor yet
    "/debug"
    "/mavros/rc/out"
)

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <bag_name>"
    exit 1
fi

BAG_NAME="bags/$1"
RECORD_DURATION_S=900 # should never really need more than 15 minutes

# Construct the ros2 bag record command
CMD=("ros2" "bag" "record" "-o" "$BAG_NAME" "${REQUIRED_TOPICS[@]}")

echo "Recording ROS bag: $BAG_NAME"
echo "Topics: ${REQUIRED_TOPICS[@]}"
echo "Auto-stop after ${RECORD_DURATION_S}s (15 minutes)"

# Execute the command and stop it with SIGINT after exactly 15 minutes.
"${CMD[@]}" &
REC_PID=$!

cleanup() {
    if kill -0 "$TIMER_PID" 2>/dev/null; then
        kill "$TIMER_PID" 2>/dev/null || true
    fi
}

trap 'if kill -0 "$REC_PID" 2>/dev/null; then kill -INT "$REC_PID"; fi; cleanup' INT TERM

(
    sleep "$RECORD_DURATION_S"
    if kill -0 "$REC_PID" 2>/dev/null; then
        echo "Reached 15 minutes. Stopping recording with SIGINT..."
        kill -INT "$REC_PID"
    fi
) &
TIMER_PID=$!

wait "$REC_PID"
REC_STATUS=$?
cleanup

exit "$REC_STATUS"