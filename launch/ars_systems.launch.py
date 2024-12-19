from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # Node to start the ARS system (ars_system node)
        Node(
            package='space_station_eclss',
            executable='ars_system',
            name='ars_system',
            output='screen',
            parameters=['config/ars_sys_params.yaml'], 
            emulate_tty=True  
        ),
        # Node to start the baking process (baking_process node)
        Node(
            package='space_station_eclss',
            executable='baking_process',
            name='baking_process',
            output='screen',
            emulate_tty=True  
        ),
    ])
