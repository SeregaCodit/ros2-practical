from typing import Final

class LaunchDefaults:
    namespace: Final[str] = "robot_01"
    config_dir: Final[str] = "config"


def get_abs_name(namespace: str, name: str) -> list[str]:
    return[
        "/",
        namespace,
        "/",
        name.lstrip("/")
    ]