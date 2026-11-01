ARG ROS_DISTRO=lyrical

FROM ros:${ROS_DISTRO}-ros-base-resolute

ARG USERNAME=ros
ARG USER_UID=1000
ARG USER_GID=1000

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV ROS_DISTRO=${ROS_DISTRO}
ENV SHELL=/bin/bash


# Create a non-root user or rename default existing
RUN apt-get update && apt-get install -y \
    sudo \
    locales \
    nano \
    #dev tools
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-argcomplete \
    build-essential \
    cmake \
    git \
    # I2C and GPIO tools
    i2c-tools \
    libi2c-dev \
    python3-gpiozero \
    v4l-utils \
    libv4l-dev \
    libcamera-dev \
    libopencv-dev \
    python3-opencv \
    gpiod \
    libgpiod-dev \
    python3-libgpiod \
    # RQt tools may be commented out if using Foxglove Bridge instead. 
    ros-${ROS_DISTRO}-rqt \
    ros-${ROS_DISTRO}-rqt-graph \
    # Foxglove Bridge (instead of RQt) may be commented out if using RQt instead
    ros-${ROS_DISTRO}-foxglove-bridge \
    # gazebo
    ros-${ROS_DISTRO}-ros-gz \
    # demo_nodes 
    ros-${ROS_DISTRO}-demo-nodes-cpp \
    ros-${ROS_DISTRO}-demo-nodes-py \
    # description tools
    ros-${ROS_DISTRO}-xacro \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    && locale-gen en_US en_US.UTF-8 \
    && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

# Create or reuse non-root user
RUN set -eux; \
    if ! getent group "${USER_GID}" >/dev/null; then \
        groupadd --gid "${USER_GID}" "${USERNAME}"; \
    fi; \
    \
    if getent passwd "${USER_UID}" >/dev/null; then \
        EXISTING_USER="$(getent passwd "${USER_UID}" | cut -d: -f1)"; \
        if [ "${EXISTING_USER}" != "${USERNAME}" ]; then \
            usermod -l "${USERNAME}" "${EXISTING_USER}"; \
        fi; \
        usermod -d "/home/${USERNAME}" -m "${USERNAME}"; \
        usermod -g "${USER_GID}" "${USERNAME}"; \
    else \
        useradd \
            --uid "${USER_UID}" \
            --gid "${USER_GID}" \
            --create-home \
            --shell /bin/bash \
            "${USERNAME}"; \
    fi; \
    \
    echo "${USERNAME} ALL=(root) NOPASSWD:ALL" \
        > "/etc/sudoers.d/${USERNAME}"; \
    chmod 0440 "/etc/sudoers.d/${USERNAME}"

# Workspace and permissions
RUN mkdir -p /ros2-ws/src \
    && chown -R "${USER_UID}:${USER_GID}" /ros2-ws

# Install entrypoint with a fixed path
COPY entrypoint.sh /usr/local/bin/ros-entrypoint
RUN chmod +x /usr/local/bin/ros-entrypoint
# Configure interactive shell
RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" \
        >> "/home/${USERNAME}/.bashrc" \
    && echo '[ -f /usr/share/colcon_argcomplete/hook/colcon-argcomplete.bash ] && source /usr/share/colcon_argcomplete/hook/colcon-argcomplete.bash' \
        >> "/home/${USERNAME}/.bashrc"

WORKDIR /ros2-ws

USER ${USERNAME}

# Update rosdep as non-root user
RUN rosdep update

ENTRYPOINT ["/usr/local/bin/ros-entrypoint"]
CMD ["bash"]