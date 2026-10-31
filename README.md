# ROS 2 Practical

Practical robotics course project.

## Technology stack

- Ubuntu 26.04
- ROS 2 Lyrical
- Gazebo Jetty
- Docker Compose
- Python and C++

## Start environment

    docker compose up -d --build
    docker compose exec ros2 bash

## ROS check

ros2 run demo_nodes_cpp talker
ros2 run demo_nodes_py listener

## Workspace - [ros2-ws/src](ros2-ws)

## Hrdware inventory - [docs/hardware.md](docs/hardware.md)