import os
from launch_ros.actions import Node
from launch import LaunchDescription
from launch_ros.actions import PushRosNamespace
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.actions import GroupAction, IncludeLaunchDescription
from launch_xml.launch_description_sources import XMLLaunchDescriptionSource


def get_var(var, default):
    try:
        return os.environ[var]
    except:
        return default

def generate_launch_description():
    namespace = ''
    navigation_type = int(get_var('NAVIGATION_TYPE', 0))
    nodes = []

    controller = Node(package='mavros_control',
                      executable='controller',
                      namespace=namespace,
                      name='controller',
                      parameters=[
                        {'xy_tolerance': 0.7,
                         'z_tolerance': 0.3,
                         'use_altitude': False,
                         'navigation_type': navigation_type,
                        }])
    nodes.append(controller)

    # MAVROS
    mavros = GroupAction(
                    actions=[
                        # push_ros_namespace to set namespace of included nodes
                        PushRosNamespace(namespace),
                        # MAVROS
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

    # Foxglove (web-based rviz)
    # foxglove = GroupAction(
    #                 actions=[
    #                     # push_ros_namespace to set namespace of included nodes
    #                     PushRosNamespace(namespace),
    #                     # Foxglove
    #                     IncludeLaunchDescription(
    #                         XMLLaunchDescriptionSource([
    #                             PathJoinSubstitution([
    #                                 FindPackageShare('foxglove_bridge'),
    #                                 'launch',
    #                                 'foxglove_bridge_launch.xml'
    #                             ])
    #                         ]),
    #                     )
    #                 ]
    #             )
    # nodes.append(foxglove)

    return LaunchDescription(nodes)