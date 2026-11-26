cmake_minimum_required(VERSION 3.20)
project(blink_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(system_interfaces REQUIRED)
find_package(Threads REQUIRED)
find_package(generate_parameter_library REQUIRED)

generate_parameter_library(action_server_params
  params/action_server_params.yaml
)

generate_parameter_library(action_client_params
  tof_sensor/action_client_params.yaml
)

add_executable(blink_action_server
  src/blink_action_server.cpp
  src/blink_action_server_main.cpp
)
add_executable(blink_action_client
  src/blink_action_client.cpp
  src/blink_action_client_main.cpp
)

target_include_directories(blink_action_server
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(blink_action_client
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(blink_action_server
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  std_msgs::std_msgs
  system_interfaces::system_interfaces
  Threads::Threads
  action_server_params
)
target_link_libraries(blink_action_client
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  std_msgs::std_msgs
  system_interfaces::system_interfaces
  action_client_params
)

INSTALL(
  TARGETS blink_action_server blink_action_client
  DESTINATION lib/${PROJECT_NAME}
)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
