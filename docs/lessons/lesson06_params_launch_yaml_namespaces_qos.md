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

# YAML-configurations and Launch
### YAML-configuration files are override values from YAML-parameter chemas if validation is successeful.



| Feature | YAML Schema | YAML Configuration |
| :--- | :--- | :--- |
| **Purpose** | Node parameter contract | Node instance configuration |
| **Parameter types** | Yes | No, not declared separately |
| **Default values** | Yes | No |
| **Validation rules** | Yes | No |
| **Specific parameter values** | Default values | Override values |
| **C++ code generation** | Yes | No |
| **Usage** | Build time + generated code at runtime | Runtime |
| **Rebuild after changes** | Yes | Usually no if use `--symlink-install` |

Firstable refactor all topic names from absolute and dynamic (like `set_gpio23` in action server) to relative: [this commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/7cf8336e4a508b6d51be4a6c757a95f96e1e0a2b) and [this](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/e042683342d598c946cc204f390ec701073c2962)


create launch package:
```bash
ros2 pkg create robot_bringup --build_type ament_cmake --license Apache-2.0
```

add to `package.xml` file launch exec dependencies:
```xml
  <exec_depend>launch</exec_depend>
  <exec_depend>launch_ros</exec_depend>
  <exec_depend>ros2launch</exec_depend>
```

create `config` and `launch` directories in the package dir. Then install them in CMakeLists.txt:

```cmake
install(
  DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)
```

therewhy we using custom puthon modules and package was built with ament_cmake add to `package.xml`
```xml
  <buildtool_depend>ament_cmake_python</buildtool_depend>
```
and `CMakeLists.txt`:

```cmake
find_package(ament_cmake_python REQUIRED)

ament_python_install_package(${PROJECT_NAME})
```
Build. Install source.

**robot_bringup v1** [commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/ddaf8259d0ebdcf3ec2eff74e3b79f4a2996aeb3). But a have to repeat myseld by creating the same topic names in different launch files/ It's easy to make mestake, so I need single source of graph names - [commit](https://github.com/SeregaCodit/ros2-practical/pull/10/changes/edde10113b6123a4b66bd4ee52f8fb93ca0bec86)

create common launch file - [commit](https://github.com/SeregaCodit/ros2-practical/commit/390619fb8c0181639df331df0b8374fc67147ba7)

now wr can run all the system by one simple command again:
```bash
ros2 launch robot_bringup robot.launch.py
```
# QoS - Quality of Service 
### Core QoS Policies

#### 1. History
*Regulates which messages to keep in history.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `KEEP_LAST` | Store only the last `N` messages (defined by Depth). | `qos.keep_last(10);` |
| `KEEP_ALL` | Store all messages until the middleware resource limits are reached. | `qos.keep_all();` |

#### 2. Depth
*Regulates the maximum history depth for `KEEP_LAST`.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `size_t` (positive integer) | Size of the message queue buffer (ignored if History is `KEEP_ALL`). | `rclcpp::QoS qos(10);` |

#### 3. Reliability
*Regulates delivery reliability.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `RELIABLE` | Guarantees message delivery; retries if packets are lost. | `qos.reliable();` |
| `BEST_EFFORT` | Attempts delivery without retries; prioritizes speed over completeness. | `qos.best_effort();` |


| Publisher | Subscriber | Connection |
| :--- | :--- | :--- |
| `BEST_EFFORT` | `BEST_EFFORT` | 🟢 Compatible (Best effort) |
| `BEST_EFFORT` | `RELIABLE` | 🔴 **Incompatible** (No connection) |
| `RELIABLE` | `BEST_EFFORT` | 🟡 Compatible (Best effort) |
| `RELIABLE` | `RELIABLE` | 🟢 Compatible (Reliable) |


#### 4. Durability
*Regulates whether previous messages are available to new subscribers.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `VOLATILE` | New subscribers only receive messages published after they join. | `qos.durability_volatile();` |
| `TRANSIENT_LOCAL` | Publisher persists samples so late-joining subscribers receive past data. | `qos.transient_local();` |


| Publisher | Subscriber | Connection |
| :--- | :--- | :--- |
| `VOLATILE` | `VOLATILE` | 🟢 Compatible (Volatile) |
| `VOLATILE` | `TRANSIENT_LOCAL` | 🔴 **Incompatible** (No connection) |
| `TRANSIENT_LOCAL` | `VOLATILE` | 🟡 Compatible (Volatile — past messages not delivered) |
| `TRANSIENT_LOCAL` | `TRANSIENT_LOCAL` | 🟢 Compatible (Transient local — past messages delivered) |

#### 5. Deadline
*Regulates the expected maximum interval between messages.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `Duration` (`std::chrono`) | Triggers an event if no message is published/received within this period. | `qos.deadline(std::chrono::milliseconds(100));` |

#### 6. Lifespan
*Regulates the message expiration time.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `Duration` (`std::chrono`) | Messages older than this duration are dropped and not delivered. | `qos.lifespan(std::chrono::seconds(5));` |

#### 7. Liveliness
*Regulates publisher activity monitoring.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `AUTOMATIC` | ROS 2 system automatically considers the publisher alive while the node runs. | `qos.liveliness(rclcpp::LivelinessPolicy::Automatic);` |
| `MANUAL_BY_TOPIC` | Publisher must explicitly assert liveliness (by publishing or calling assert). | `qos.liveliness(rclcpp::LivelinessPolicy::ManualByTopic);` |

#### 8. Liveliness Lease Duration
*Regulates the timeout for activity confirmation.*

| Argument / Value | Interpretation | Example (`rclcpp`) |
| :--- | :--- | :--- |
| `Duration` (`std::chrono`) | Maximum time before a publisher is marked as inactive/dead if not asserted. | `qos.liveliness_lease_duration(std::chrono::seconds(1));` |

### Standard ROS 2 QoS Profiles

| Profile (`rclcpp` / `rclpy`) | Description | Key Settings (History, Depth, Reliability, Durability) | Use Cases |
| :--- | :--- | :--- | :--- |
| **`SystemDefaultsQoS`** | Uses default settings defined by the underlying RMW (DDS) implementation. | `KEEP_LAST`,<br> Depth: RMW default (usually `10`),<br>`RELIABLE`,<br>`VOLATILE` | General-purpose topics when relying on middleware defaults. |
| **`SensorDataQoS`** | Prioritizes low latency and high frequency over guaranteed delivery of every packet. | `KEEP_LAST`,<br>Depth: `5`,<br>`BEST_EFFORT`,<br>`VOLATILE` | High-frequency sensor streams (LiDAR, cameras, IMU, radar). |
| **`ServicesQoS`** | Ensures reliable request-response communication without stale queued calls. | `KEEP_LAST`,<br>Depth: `10`,<br>`RELIABLE`,<br>`VOLATILE` | ROS 2 Services and Action service channels. |
| **`ParametersQoS`** | Guarantees reliable delivery for parameter changes with a deeper queue to prevent drops. | `KEEP_LAST`,<br>Depth: `1000`,<br>`RELIABLE`,<br>`VOLATILE` | Parameter servers, parameter clients, and parameter event topics (`/parameter_events`). |
| **`ParameterEventsQoS`** | Tailored specifically for broadcasting parameter updates across nodes. | `KEEP_LAST`,<br>Depth: `1000` (C++) / `50` (Python),<br>`RELIABLE`,<br>`VOLATILE` | Monitoring and reacting to node parameter changes in real time. |
| **`ClockQoS`** | Fast, low-overhead clock synchronization with minimal history. | `KEEP_LAST`,<br>Depth: `1`,<br>`BEST_EFFORT`,<br>`VOLATILE` | Simulation time (`/clock` topic from Gazebo, Isaac Sim, etc.). |
| **`RosoutQoS`** | Reliable logging stream that retains a large buffer of log messages. | `KEEP_LAST`,<br>Depth: `1000`,<br>`RELIABLE`,<br>`TRANSIENT_LOCAL` | System logging (`/rosout` topic) so late-joining log viewers see recent logs. |

```cpp
// Standard profile
auto qos_default = rclcpp::QoS(10);

// Sensor data
auto qos_sensor = rclcpp::SensorDataQoS();

// Custom profile
auto qos_state = rclcpp::QoS(
    rclcpp::KeepLast(1)
)
.reliable()
.transient_local();
```

```python
from rclpy.qos import (
    QoSProfile,
    ReliabilityPolicy,
    DurabilityPolicy,
    HistoryPolicy,
)

qos_sensor = QoSProfile(
    history=HistoryPolicy.KEEP_LAST,
    depth=5,
    reliability=ReliabilityPolicy.BEST_EFFORT,
    durability=DurabilityPolicy.VOLATILE,
)
```

## PRACTICE

### 1. RELIABILITY
to send a message with qos parameter to the topic:
```bash
ros2 topic pub -r 2 --qos-reliability best_effort /qos_lab std_msgs/msg/String "{data: 'hello'}"
```
get echo with qos parameter:
```bash
ros2 topic echo --qos-reliability reliable /qos_lab
```

in this case we get warning because of best_effort publisher and reliable subscriber are not compatable:
```bash
[WARN] [1795861361.901648457] [_ros2cli_41803]: New publisher discovered on topic '/qos_lab', offering incompatible QoS. No messages will be received from it. Last incompatible policy: RELIABILITY
```
check topic profiles:
```bash
ros2 topic info /qos_lab --verbose
```
got:
```bash
Type: std_msgs/msg/String

Publisher count: 1

Node name: _ros2cli_41200
Node namespace: /
Topic type: std_msgs/msg/String
Topic type hash: RIHS01_df668c740482bbd48fb39d76a70dfd4bd59db1288021743503259e948f6b1a18
Endpoint type: PUBLISHER
GID: 01.0f.bc.7b.f0.a0.23.1b.00.00.00.00.00.00.07.03
QoS profile:
  Reliability: BEST_EFFORT
  History (Depth): KEEP_LAST (10)
  Durability: VOLATILE
  Lifespan: Infinite
  Deadline: Infinite
  Liveliness: AUTOMATIC
  Liveliness lease duration: Infinite

Subscription count: 0
```
as we can see our `Reliability` is `BEST_EFFORT` so we need to set subscriber `reliability` profile as `BEST_EFFORT` too:
```bash
ros2 topic echo --qos-reliability best_effort /qos_lab
```
result:
```bash
data: hello
---
data: hello
...
```

### TRANSIENT_LOCAL

Check saved message delivery to a subscriber that connecting later:
```bash
ros2 topic pub --once \ 
  --wait-matching-subscriptions 0 \
  --keep-alive 60 \
  --qos-reliability reliable \
  --qos-durability transient_local \
  --qos-depth 1 \
  /qos_state \
  std_msgs/msg/String \
  "{data: 'ARMED'}"
```
other terminal:
```bash
ros2 topic echo \
  --once \
  --qos-durability transient_local \
  --qos-reliability reliable \
  /qos_state
```
we can re-launch this command multiple times. Each time we well get state while publisher is alive.


Run new subscriber with `VOLATILE` durability policy:
```bash
timeout -s INT 5s ros2 topic echo \
  --qos-durability volatile \
  /qos_state
```
in this case the subscriber will not recive any messages as the publisher does not actually send any messages after the  subscriber has connected. Furthermore, a subscriber with the  `VOLATILE` value of `durability` policy  does not recive prior messages

### CURRENT ROS-GRAPH
```bash
ros2 launch robot_bringup robot.launch.py 
```
other terminal:
```bash
ros2 topic info /robot_01/system/state/arming_state --verbose
```
result:
```bash
Type: system_interfaces/msg/ArmingState

Publisher count: 1

Node name: arm_controller
Node namespace: /robot_01/safety/arm_disarm
Topic type: system_interfaces/msg/ArmingState
Topic type hash: RIHS01_4f9f122ef290107c5c20936b7448df705c2dca47b6938c1bc01f5f27813ef9f0
Endpoint type: PUBLISHER
GID: 01.0f.bc.7b.24.f3.e0.f1.00.00.00.00.00.00.16.03
QoS profile:
  Reliability: RELIABLE
  History (Depth): KEEP_LAST (10)
  Durability: VOLATILE
  Lifespan: Infinite
  Deadline: Infinite
  Liveliness: AUTOMATIC
  Liveliness lease duration: Infinite

Subscription count: 2

Node name: led_arm_disarm_indication_controller
Node namespace: /robot_01/interfaces/controllers
Topic type: system_interfaces/msg/ArmingState
Topic type hash: RIHS01_4f9f122ef290107c5c20936b7448df705c2dca47b6938c1bc01f5f27813ef9f0
Endpoint type: SUBSCRIPTION
GID: 01.0f.bc.7b.27.f3.d5.98.00.00.00.00.00.00.13.04
QoS profile:
  Reliability: RELIABLE
  History (Depth): KEEP_LAST (10)
  Durability: VOLATILE
  Lifespan: Infinite
  Deadline: Infinite
  Liveliness: AUTOMATIC
  Liveliness lease duration: Infinite

Node name: action_server
Node namespace: /robot_01/actions/blink
Topic type: system_interfaces/msg/ArmingState
Topic type hash: RIHS01_4f9f122ef290107c5c20936b7448df705c2dca47b6938c1bc01f5f27813ef9f0
Endpoint type: SUBSCRIPTION
GID: 01.0f.bc.7b.28.f3.18.2e.00.00.00.00.00.00.14.04
QoS profile:
  Reliability: RELIABLE
  History (Depth): KEEP_LAST (10)
  Durability: VOLATILE
  Lifespan: Infinite
  Deadline: Infinite
  Liveliness: AUTOMATIC
  Liveliness lease duration: Infinite
```