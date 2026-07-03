from launch import LaunchDescription
from launch.actions import GroupAction, IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch.actions import RegisterEventHandler, ExecuteProcess, LogInfo
from launch.event_handlers import OnShutdown
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
                    'fcu_url': 'udp://0.0.0.0:14550@',
                }.items(),
            ),
        ]
    )
    
    # # 2. Define the shutdown action: Disarm
    # disarm_cmd = ExecuteProcess(
    #     cmd=['ros2', 'service', 'call', '/mavros/cmd/arming', 'mavros_msgs/srv/CommandBool', "{value: false}"],
    #     output='screen'
    # )
    
    # # 4. Register the Shutdown Event Handler
    # shutdown_handler = RegisterEventHandler(
    #     OnShutdown(
    #         on_shutdown=[
    #             LogInfo(msg="[SHUTDOWN DETECTED] Attempting to disarm..."),
    #             disarm_cmd
    #         ]
    #     )
    # )

    return LaunchDescription([mavros])

# ros2 service call /mavros/cmd/arming mavros_msgs/srv/CommandBool {value: false}