# ROS 2 Parameters
### parameter types

| ROS 2 type | C++ | Python |
| :--- | :--- | :--- |
| `bool` | `bool` | `bool` |
| `integer` | `int64_t` | `int` |
| `double` | `double` | `float` |
| `string` | `std::string` | `str` |
| `byte_array` | array of bytes | corresponding byte representation |
| `bool_array` | `std::vector<bool>` | `list[bool]` |
| `integer_array` | `std::vector<int64_t>` | `list[int]` |
| `double_array` | `std::vector<double>` | `list[float]` |
| `string_array` | `std::vector<std::string>` | `list[str]` |


## generate_parameter_library
[GitHub](https://github.com/ros-tooling/generate_parameter_library?utm_source=chatgpt.com)

add `generate_parameter_library` to `apt install` commnd in dockerfile - [commit](https://github.com/SeregaCodit/ros2-practical/commit/475618b442038122f4cecaa17b9a74bc435b0ea2)  
|lesson|case|commit changes|
|------|----|--------------|
|02|2 read-only params with cross-parameter invariant| [commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/ac4cc0f501eb184953c00de64f25d06a40ea534f)|
|03|python node 2 params with cross-parameter invariant|[commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/b9d618124af216fe31e563c0bae1d6f609589322)|
|04|few nodes in one package with general parameter| [commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/66f12dc6d4af3d1c03858c34cd43e1c0479d2950)|
|05 blink_control|runtime parameters update in action client | [commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/fdf6bdbb9b2b87b42e6eb71187553e5616a15005)|
|05|other same param refactoring in lesson 5 files| [commit](https://github.com/SeregaCodit/ros2-practical/commit/0df0459b9d1366dbd17456b4de1217ec8c5a50ff)|
|

