from launch import LaunchDescription
from launch.actions import GroupAction, IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node, PushRosNamespace
from launch_ros.substitutions import FindPackageShare
from launch_xml.launch_description_sources import XMLLaunchDescriptionSource


def generate_launch_description():
    namespace = ''
    nodes = []

    params_file = PathJoinSubstitution([
        FindPackageShare('manta_bringup'),
        'launch',
        'params.yaml',
    ])
    
    brain_params_file = PathJoinSubstitution([
        FindPackageShare('manta_bringup'),
        'launch',
        'brain_qual.yaml',
    ])
    
    cv_params_file = PathJoinSubstitution([
        FindPackageShare('manta_bringup'),
        'launch',
        'dummy_cv.yaml',
    ])

    brain = Node(
        package='brain',
        executable='brain_node',
        name='brain',
        output='screen',
        parameters=[brain_params_file]
    )
    nodes.append(brain)
    
    cv = Node(
        package='cv',
        executable='cv_node',
        name='cv_node',
        output='screen',
        parameters=[params_file, cv_params_file],
    )
    nodes.append(cv)

    tasks = Node(
        package='tasks',
        executable='task_runner',
        name='task_runner',
        output='screen',
        parameters=[params_file],
    )
    nodes.append(tasks)

    slam = Node(
        package='ekfslam',
        executable='ekfslam_node',
        name='ekfslam',
        output='screen',
        parameters=[params_file],
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