cmake_minimum_required(VERSION 3.20)
project(cpu_temperature_alarm)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(generate_parameter_library REQUIRED)


generate_parameter_library(cpu_temperature_alarm_params
  params/cpu_temperature_alarm_params.yaml
)

add_executable(cpu_temperature_alarm_notificator 
  src/cpu_temperature_alarm_notificator.cpp
)


target_link_libraries(cpu_temperature_alarm_notificator
  PRIVATE
    rclcpp::rclcpp
    sensor_msgs::sensor_msgs
    cpu_temperature_alarm_params
)

INSTALL(
  TARGETS cpu_temperature_alarm_notificator
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
