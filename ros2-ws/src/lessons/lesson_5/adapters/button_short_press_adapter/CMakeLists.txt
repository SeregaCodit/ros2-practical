cmake_minimum_required(VERSION 3.20)
project(button_short_press_adapter)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(course_utils REQUIRED)
find_package(generate_parameter_library REQUIRED)

generate_parameter_library(short_press_adapter_parameters
  params/button_short_press_adapter_params.yaml
)

add_executable(button_short_press_adapter
  src/button_short_press_adapter.cpp
  src/main.cpp
)

target_include_directories(button_short_press_adapter
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(button_short_press_adapter
  rclcpp::rclcpp
  std_msgs::std_msgs
  course_utils::course_utils
  short_press_adapter_parameters
)

INSTALL(
  TARGETS button_short_press_adapter
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
