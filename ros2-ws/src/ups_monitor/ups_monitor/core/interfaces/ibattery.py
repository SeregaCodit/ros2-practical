from abc import ABC, abstractmethod
import enum


class IBattery:

    @abstractmethod
    def voltage_status(self, voltage: float) -> enum.Enum:
        """returns voltage status of battery"""
        pass

    @property
    @abstractmethod
    def type(self) -> str:
        pass

    @property
    @abstractmethod
    def cell_count(self) -> int:
        pass

    @cell_count.setter
    def cell_count(self, value: int):
        pass

    @property
    @abstractmethod
    def max_voltage(self) -> float:
        pass

    @max_voltage.setter
    def max_voltage(self, value: float) -> None:
        pass

    @property
    @abstractmethod
    def nominal_voltage(self) -> float:
        pass

    @nominal_voltage.setter
    def nominal_voltage(self, value: float) -> None:
        pass

    @property
    @abstractmethod
    def min_safe_voltage(self) -> float:
        pass

    @min_safe_voltage.setter
    def min_safe_voltage(self, value: float) -> None:
        pass

    @property
    @abstractmethod
    def deep_discharge_voltage(self) -> float:
        pass

    @deep_discharge_voltage.setter
    def deep_discharge_voltage(self, value: float) -> None:
        pass
