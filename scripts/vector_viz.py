import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse

def create_elliptical_geometry_field():
    # 1. Setup the Grid (7x7m)
    side_length = 7.0
    step = 0.1
    x = np.arange(0, side_length + step, step)
    y = np.arange(0, side_length + step, step)
    X, Y = np.meshgrid(x, y)

    U = np.zeros_like(X)
    V = np.zeros_like(Y)

    # 2. Define Points
    points = [
        {
            'pos': (1.0, 5.0), 'strength': 0.7, 'type': 'rotational', 
            'cw': False, 'radii': (1.3, 3.0), 'angle': 0, 'range': None
        },
        {
            'pos': (3.0, 5.0), 'strength': 0.7, 'type': 'rotational', 
            'cw': True, 'radii': (1.3, 3.0), 'angle': 0, 'range': None
        },
        {
            'pos': (1.5, 1.5), 'strength': 0.5, 'type': 'rotational', 
            'cw': False, 'radii': (0.6, 0.6), 'angle': 0, 'range': 1.0
        },
    ]

    # --- SIMULATION PARAMETERS ---
    # Define starting robot position
    robot_start_pos = [7.0, 0.2] 
    dt = 0.05        # Time step (seconds)
    max_steps = 1000  # Duration of simulation
    # -----------------------------d recentd recent

    # Vector Field Calculation
    for pt in points:
        px, py = pt['pos']
        dx = X - px
        dy = Y - py
        rx, ry = pt['radii']
        angle_rad = np.radians(pt.get('angle', 0))
        
        dx_rot = dx * np.cos(angle_rad) + dy * np.sin(angle_rad)
        dy_rot = -dx * np.sin(angle_rad) + dy * np.cos(angle_rad)
        
        r_ell_sq = (dx_rot/rx)**2 + (dy_rot/ry)**2
        r_ell_sq[r_ell_sq == 0] = np.inf
        mag = pt['strength'] / r_ell_sq

        if pt['type'] == 'rotational':
            direction = 1 if pt.get('cw', True) else -1
            u_rot = (dy_rot / ry**2) * mag * direction
            v_rot = (-dx_rot / rx**2) * mag * direction
        elif pt['type'] == 'repulsor':
            u_rot = (dx_rot / rx**2) * mag
            v_rot = (dy_rot / ry**2) * mag
        else: # attractor
            u_rot = (-dx_rot / rx**2) * mag
            v_rot = (-dy_rot / ry**2) * mag

        u_pt = u_rot * np.cos(-angle_rad) + v_rot * np.sin(-angle_rad)
        v_pt = -u_rot * np.sin(-angle_rad) + v_rot * np.cos(-angle_rad)

        r_phys = np.sqrt(dx**2 + dy**2)
        if pt.get('range'):
            mask = r_phys > pt['range']
            u_pt[mask] = 0
            v_pt[mask] = 0
            
            # Define how wide the "fade-in" zone is (e.g., 0.5 meters)
            fade_margin = pt['range'] * 0.5
            
            # Calculate weight: 1.0 deep inside, 0.0 outside range, sliding between
            # Weight = (Range - CurrentDist) / Margin
            weight = (pt['range'] - r_phys) / fade_margin
            weight = np.clip(weight, 0, 1) # Ensure it stays between 0 and 1
            
            u_pt *= weight
            v_pt *= weight

        U += u_pt
        V += v_pt
    # --- SIMULATION LOOP ---
    traj_x = [robot_start_pos[0]]
    traj_y = [robot_start_pos[1]]
    curr_pos = np.array(robot_start_pos)

    for _ in range(max_steps):
        # Find index in grid closest to robot's current position
        ix = int(np.clip(curr_pos[0] / step, 0, side_length / step))
        iy = int(np.clip(curr_pos[1] / step, 0, side_length / step))
        
        # Get velocity from the field at that grid point
        vx = U[iy, ix]
        vy = V[iy, ix]
        
        # Update position: x = x + v * dt
        # We cap the speed so the robot doesn't "teleport" near high-intensity points
        speed = np.sqrt(vx**2 + vy**2)
        if speed > 2.0:
            vx, vy = (vx/speed)*2.0, (vy/speed)*2.0
            
        curr_pos += np.array([vx, vy]) * dt
        
        traj_x.append(curr_pos[0])
        traj_y.append(curr_pos[1])
        
        # Stop if robot leaves the 7x7 grid
        # if not (0 <= curr_pos[0] <= side_length and 0 <= curr_pos[1] <= side_length):
        #     break

    # --- VISUALIZATION ---
    magnitude = np.sqrt(U**2 + V**2)
    with np.errstate(divide='ignore', invalid='ignore'):
        U_norm = np.where(magnitude > 0, U / magnitude, 0)
        V_norm = np.where(magnitude > 0, V / magnitude, 0)

    plt.figure(figsize=(12, 10))
    plt.style.use('dark_background')
    
    skip = 2 
    v_max = np.percentile(magnitude[magnitude > 0], 95) if np.any(magnitude > 0) else 1

    # Plot Vector Field
    q = plt.quiver(X[::skip, ::skip], Y[::skip, ::skip], 
                   U_norm[::skip, ::skip], V_norm[::skip, ::skip], 
                   magnitude[::skip, ::skip], 
                   cmap='plasma', clim=(0, v_max), 
                   scale=35, width=0.004, pivot='mid', alpha=0.6)
    
    # Plot Trajectory (The Robot's Path)
    plt.plot(traj_x, traj_y, color='white', linewidth=3, label='Robot Trajectory', zorder=5)
    plt.plot(traj_x[0], traj_y[0], 'go', markersize=10, label='Start') # Start point
    plt.plot(traj_x[-1], traj_y[-1], 'ro', markersize=10, label='Current Position') # End point

    # Plot Field Sources
    colors = {'rotational': 'cyan', 'attractor': 'lime', 'repulsor': 'red'}
    for pt in points:
        color = colors.get(pt['type'], 'white')
        plt.plot(pt['pos'][0], pt['pos'][1], 'o', color=color, markersize=6)
        if pt.get('range'):
            circ = plt.Circle(pt['pos'], pt['range'], color=color, fill=False, linestyle=':', alpha=0.3)
            plt.gca().add_patch(circ)
        ell = Ellipse(xy=pt['pos'], width=pt['radii'][0]*2, height=pt['radii'][1]*2, 
                      angle=pt.get('angle', 0), color=color, fill=False, alpha=0.2)
        plt.gca().add_patch(ell)

    plt.title("Robot Trajectory Simulation in Elliptical Field")
    plt.xlim(0, side_length)
    plt.ylim(0, side_length)
    plt.legend(loc='upper right')
    plt.gca().set_aspect('equal')
    plt.show()

if __name__ == "__main__":
    create_elliptical_geometry_field()