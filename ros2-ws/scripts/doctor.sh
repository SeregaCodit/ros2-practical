#!/usr/bin/env bash

set -eo pipefail

set +u
source /opt/ros/lyrical/setup.bash
set -u

WORKSPACE="/ros2-ws/src"

echo
echo "=== ROS 2 Environment Doctor ==="

echo
echo "=== CHECK UBUNTU VERSION ==="
source /etc/os-release

if [[ "$ID" == "ubuntu" && "$VERSION_ID" == "26.04" ]]; then
    echo "[OK] Ubuntu $VERSION_ID"
else
    echo "[ERROR] Expected Ubuntu 26.04, got: $PRETTY_NAME" >&2
    exit 1
fi

echo
echo "=== CHECK ROS_DISTRO =="
if [[ "${ROS_DISTRO:-}" != "lyrical" ]]; then
    echo "[ERROR] Expected ROS 2 Lyrical"
    exit 1
fi

echo "[OK] ROS_DISTRO: $ROS_DISTRO"

echo
echo "=== CHECK PYTHON AND C++ COMPILER ==="
c++ --version
echo
python3 --version
echo

echo "=== CHECK COLCON IS AVAILABLE ==="
if command -v colcon >/dev/null 2>&1; then
    echo "[OK] colcon available: $(command -v colcon)"
else
    echo "[ERROR] colcon not found"
    exit 1
fi

echo
echo "=== CHECK ROS_GZ_BRIDGE ==="
if ros2 pkg prefix ros_gz_bridge >/dev/null 2>&1; then
    echo "[OK] ros_gz_bridge available"
else
    echo "[ERROR] ros_gz_bridge not found"
    exit 1
fi

echo "=== CHECK ROS WORKSPACE ==="
echo
if [[ -d "$WORKSPACE" ]]; then
    echo "[OK] Workspace exists: $WORKSPACE"
else
    echo "[ERROR] Workspace not found: $WORKSPACE" >&2
    exit 1
fi

echo

echo "=== ALL CHECKS OK ==="