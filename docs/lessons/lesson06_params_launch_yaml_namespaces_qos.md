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

|lesson|case|commit changes|
|------|----|--------------|
|02|2 read-only params with cross-parameter invariant| [commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/ac4cc0f501eb184953c00de64f25d06a40ea534f)|
