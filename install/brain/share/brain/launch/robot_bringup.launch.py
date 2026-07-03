from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node, PushRosNamespace
from launch_ros.substitutions import FindPackageShare
from launch_xml.launch_description_sources import XMLLaunchDescriptionSource


def generate_launch_description():
    namespace = ''
    nodes = []

    log_level_arg = DeclareLaunchArgument(
        'log_level',
        default_value='info', #add 'log_level:=warn' at end of launch command maybe?
        description='Set to "warn" for competition runs.'
    )
    nodes.append(log_level_arg)
    
    log_level = LaunchConfiguration('log_level')
    
    params_file = PathJoinSubstitution([
        FindPackageShare('brain'),
        'launch',
        'params.yaml',
    ])

    tasks = Node(
        package='tasks',
        executable='task_runner',
        name='task_runner',
        output='screen',
        parameters=[params_file],
        # ros_arguments=['--log-level', log_level]
    )
    nodes.append(tasks)

    cv = Node(
        package='cv',
        executable='cv_node',
        name='cv_node',
        output='screen',
        parameters=[params_file],
        # ros_arguments=['--log-level', log_level]
    )
    nodes.append(cv)
    
    bottom_cv = Node(
        package='cv',
        executable='bottom_cv_node',
        name='bottom_cv_node',
        output='screen',
        parameters=[params_file],
        ros_arguments=['--log-level', log_level]
    )
    nodes.append(bottom_cv)

    slam = Node(
        package='ekfslam',
        executable='ekfslam_node',
        name='ekfslam',
        output='screen',
        parameters=[params_file],
        ros_arguments=['--log-level', log_level]
    )
    nodes.append(slam)

    # mavros = GroupAction(
    #     actions=[
    #         PushRosNamespace(namespace),
    #         IncludeLaunchDescription(
    #             XMLLaunchDescriptionSource([
    #                 PathJoinSubstitution([
    #                     FindPackageShare('mavros_control'),
    #                     'launch',
    #                     'mavros.launch',
    #                 ])
    #             ]),
    #             launch_arguments={
    #                 'fcu_url': 'udp://0.0.0.0:14550@',
    #             }.items(),
    #         ),
    #     ]
    # )
    # nodes.append(mavros)

    return LaunchDescription(nodes)