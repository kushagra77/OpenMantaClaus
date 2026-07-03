from launch import LaunchDescription
from launch.actions import GroupAction, IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import PushRosNamespace
from launch_ros.substitutions import FindPackageShare
from launch_xml.launch_description_sources import XMLLaunchDescriptionSource


def generate_launch_description():
    namespace = ''

    mavros = GroupAction(
        actions=[
            PushRosNamespace(namespace),
            IncludeLaunchDescription(
                XMLLaunchDescriptionSource([
                    PathJoinSubstitution([
                        FindPackageShare('brain'),
                        'launch',
                        'mavros.launch',
                    ])
                ]),
                launch_arguments={
                    'fcu_url': 'tcp://127.0.0.1:5760',
                    'gcs_url': 'udp://@localhost:14550',
                }.items(),
            ),
        ]
    )

    return LaunchDescription([mavros])