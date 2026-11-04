import enum

class BatteryChargeStatus(enum.Enum):
    NORMAL = 0
    LOW = 1
    DEEP_DISCHARGE = 3
    HIGH = 4
