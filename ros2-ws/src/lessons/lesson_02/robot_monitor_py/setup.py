from setuptools import find_packages, setup

package_name = 'robot_monitor_py'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    package_data={'': ['py.typed']},
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='ros',
    maintainer_email='ros@todo.todo',
    description='cpu temperature monitoring',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'cpu_temperature_publisher = robot_monitor_py.nodes.cpu_temperature_publisher:main',
            "cpu_temperature_publisher_v2 = robot_monitor_py.nodes.cpu_temperature_publisher_v2:main"
        ],
    },
)
