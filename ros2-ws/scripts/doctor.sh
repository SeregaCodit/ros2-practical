#!usr/bin/env bash
#!/usr/bin/env bash
set -eo pipefail

source /opt/ros/lyrical/setup.bash

echo
echo "=== ROS 2 Environment Doctor ==="

# TODO: Check Ubuntu version
echo
echo "=== CHECK UBUNTU VERSION ==="
cat /etc/os-release
# TODO: Check ROS_DISTRO
echo
echo "=== CHECK ROS_DISTRO =="
echo "${ROS_DISTRO}"
# TODO: Check Python and C++ compiler
echo
echo "=== CHECK PYTHON AND C++ COMPILER ==="
c++ --version
echo
python3 --version
echo
# TODO: Check colcon
echo "=== CHECK COLCON IS AVAILABLE ==="
if command -v colcon >/dev/null 2>&1; then
    echo "[OK] colcon available: $(command -v colcon)"
else
    echo "[ERROR] colcon not found"
    exit 1
fi
# TODO: Check ros_gz_bridge
echo
echo "=== CHECK ROS_GZ_BRIDGE ==="
if ros2 pkg prefix ros_gz_bridge >/dev/null 2>&1; then
    echo "[OK] ros_gz_bridge available"
else
    echo "[ERROR] ros_gz_bridge not found"
    exit 1
fi
# TODO: Check ROS workspace
echo "=== CHECK ROS WORSPACE ==="
echo ${WORSPACE}

echo "=== ALL CHECKS OK ==="