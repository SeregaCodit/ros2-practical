#!/usr/bin/env bash
set -e

# Source ROS 2 environment
source "/opt/ros/${ROS_DISTRO}/setup.bash"

# Source colcon autocomplete, if available
COLCON_HOOK="/usr/share/colcon_argcomplete/hook/colcon-argcomplete.bash"

if [ -f "$COLCON_HOOK" ]; then
    source "$COLCON_HOOK"
fi

# Source local ROS 2 workspace, if built
if [ -f /ros2-ws/install/setup.bash ]; then
    source /ros2-ws/install/setup.bash
fi

exec "$@"