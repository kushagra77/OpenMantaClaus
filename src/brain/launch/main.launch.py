from launch_ros.actions import Node
from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    params_file = PathJoinSubstitution([
        FindPackageShare('brain'),
        'launch',
        'params.yaml',
    ])

    brain = Node(
        package='brain',
        executable='brain_node',
        name='brain',
        output='screen',
        parameters=[params_file]
    )

    return LaunchDescription([brain])