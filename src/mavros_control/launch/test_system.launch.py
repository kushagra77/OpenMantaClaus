import os
from launch_ros.actions import Node
from launch import LaunchDescription
from launch_ros.actions import PushRosNamespace
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.actions import GroupAction, IncludeLaunchDescription
from launch_xml.launch_description_sources import XMLLaunchDescriptionSource



def generate_launch_description():
    namespace = ''
    nodes = []
    params_file = PathJoinSubstitution([
        FindPackageShare('brain'),
        'launch',
        'params.yaml'
    ])

    controller = Node(
        package='mavros_control',
        executable='controller',
        namespace=namespace,
        name='controller',
        parameters=[params_file, {}]
    )
    nodes.append(controller)

    mavros = GroupAction(
        actions=[
            PushRosNamespace(namespace),
            IncludeLaunchDescription(
                XMLLaunchDescriptionSource([
                    PathJoinSubstitution([
                        FindPackageShare('mavros_control'),
                        'launch',
                        'mavros.launch'
                    ])
                ]),
                launch_arguments={
                    "fcu_url": "udp://0.0.0.0:14550@"
                }.items()
            ),
        ]
    )
    nodes.append(mavros)

    debug_cv = Node(
        package='cv',
        executable='testing_node',
        namespace=namespace,
        name='testing_node',
        output='screen'
    )
    nodes.append(debug_cv)

    return LaunchDescription(nodes)
