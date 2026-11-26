cmake_minimum_required(VERSION 3.20)
project(led_arming_indication_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(system_interfaces REQUIRED)
find_package(generate_parameter_library REQUIRED)

generate_parameter_library(led_indication_params
  params/led_indication_params.yaml
)

add_executable(arming_indication_controller src/led_arming_indication_controller.cpp)

target_include_directories(arming_indication_controller
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
) 

target_link_libraries(arming_indication_controller
    rclcpp::rclcpp
    std_msgs::std_msgs
    system_interfaces::system_interfaces
    led_indication_params
)

INSTALL(
  TARGETS arming_indication_controller
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
