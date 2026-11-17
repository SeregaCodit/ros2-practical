cmake_minimum_required(VERSION 3.20)
project(gpio_driver)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(course_utils REQUIRED)

# External dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD_CXX REQUIRED IMPORTED_TARGET libgpiodcxx)

# ====== led_node ======
add_executable(led_node src/led_node.cpp)

# Link dependencies
target_link_libraries(led_node PRIVATE
  rclcpp::rclcpp
  std_msgs::std_msgs
  PkgConfig::GPIOD_CXX
)

# ====== button_node ======
add_executable(button_node src/button_node.cpp)
target_link_libraries(button_node PRIVATE
  rclcpp::rclcpp
  std_msgs::std_msgs
  PkgConfig::GPIOD_CXX
  course_utils::course_utils
)



install(
  TARGETS
   led_node
   button_node
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
