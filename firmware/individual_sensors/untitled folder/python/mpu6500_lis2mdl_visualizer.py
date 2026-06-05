import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from collections import deque
import time
import sys

# ============================================
# CONFIGURATION
# ============================================
SERIAL_PORT = '/dev/cu.usbmodem190846701'  # Your Teensy port
BAUD_RATE = 115200
BUFFER_SIZE = 100  # Number of points to keep for trail

# ============================================
# SETUP SERIAL CONNECTION
# ============================================
print("=" * 60)
print("MPU6500 + LIS2MDL 3D Real-Time Visualizer")
print("=" * 60)

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"✅ Connected to {SERIAL_PORT}")
except Exception as e:
    print(f"❌ Failed to connect: {e}")
    print("\nChecklist:")
    print("  1. Is your Teensy plugged in?")
    print("  2. Is the Arduino Serial Monitor CLOSED?")
    print("  3. Is the port correct?")
    sys.exit(1)

# Wait for Teensy to reset
time.sleep(2)
print("📡 Reading sensor data...")
print("🎮 Move the sensors to see real-time visualization!")
print("❌ Close the plot window to exit")
print("=" * 60)

# ============================================
# DATA BUFFERS
# ============================================
# For magnetometer trail
mag_buffer = deque(maxlen=BUFFER_SIZE)

# For accelerometer trail
acc_buffer = deque(maxlen=BUFFER_SIZE)

# ============================================
# SETUP 3D PLOT
# ============================================
plt.ion()
fig = plt.figure(figsize=(14, 8))

# Create two subplots side by side
ax1 = fig.add_subplot(121, projection='3d')  # Magnetometer
ax2 = fig.add_subplot(122, projection='3d')  # Accelerometer

# Set titles
ax1.set_title('Magnetometer (Magnetic Field)', fontsize=12, fontweight='bold')
ax2.set_title('Accelerometer (Linear Acceleration)', fontsize=12, fontweight='bold')

# Colors for different axes
colors = {
    'x': 'red',
    'y': 'green', 
    'z': 'blue'
}

# ============================================
# HELPER FUNCTIONS
# ============================================
def setup_axes(ax, limits=(-2, 2)):
    """Configure 3D axes with consistent limits"""
    ax.set_xlim(limits)
    ax.set_ylim(limits)
    ax.set_zlim(limits)
    ax.set_xlabel('X Axis', fontsize=9)
    ax.set_ylabel('Y Axis', fontsize=9)
    ax.set_zlabel('Z Axis', fontsize=9)
    
    # Add grid
    ax.grid(True, alpha=0.3)
    
    # Add origin sphere
    ax.scatter([0], [0], [0], c='black', s=20, marker='o')

def draw_vector(ax, start, end, color, label, linewidth=2):
    """Draw a 3D vector from start to end"""
    ax.plot([start[0], end[0]], 
            [start[1], end[1]], 
            [start[2], end[2]], 
            color=color, linewidth=linewidth, label=label, alpha=0.8)
    
    # Add arrow head (simple cone)
    direction = np.array(end) - np.array(start)
    length = np.linalg.norm(direction)
    if length > 0:
        direction = direction / length
        arrow_point = np.array(end) - direction * 0.1
        ax.scatter([arrow_point[0]], [arrow_point[1]], [arrow_point[2]], 
                  c=color, s=30, marker='^')

def get_sensor_value(value, scale=1.0):
    """Scale sensor values for better visualization"""
    return value / scale

# ============================================
# MAIN VISUALIZATION LOOP
# ============================================
try:
    frame_count = 0
    last_print = time.time()
    
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            parts = line.split(',')
            
            # Expecting 10 values: Ax,Ay,Az,Gx,Gy,Gz,Temp,Mx,My,Mz
            if len(parts) == 10:
                try:
                    # Parse data
                    ax_val = float(parts[0]) / 1000.0  # Scale to g's
                    ay_val = float(parts[1]) / 1000.0
                    az_val = float(parts[2]) / 1000.0
                    
                    gx_val = float(parts[3])  # Gyro raw
                    gy_val = float(parts[4])
                    gz_val = float(parts[5])
                    
                    temp_val = float(parts[6])
                    
                    mx_val = float(parts[7])  # Magnetometer
                    my_val = float(parts[8])
                    mz_val = float(parts[9])
                    
                    # Store in buffers
                    mag_buffer.append((mx_val, my_val, mz_val))
                    acc_buffer.append((ax_val, ay_val, az_val))
                    
                    # Clear axes
                    ax1.clear()
                    ax2.clear()
                    
                    # ====================================
                    # LEFT PLOT: MAGNETOMETER
                    # ====================================
                    ax1.set_title(f'Magnetometer | X:{mx_val:6.1f} Y:{my_val:6.1f} Z:{mz_val:6.1f} µT', 
                                  fontsize=10, fontweight='bold')
                    ax1.set_xlim([-150, 150])
                    ax1.set_ylim([-150, 150])
                    ax1.set_zlim([-150, 150])
                    ax1.set_xlabel('X (µT)', fontsize=9)
                    ax1.set_ylabel('Y (µT)', fontsize=9)
                    ax1.set_zlabel('Z (µT)', fontsize=9)
                    ax1.grid(True, alpha=0.3)
                    
                    if len(mag_buffer) > 0:
                        # Plot trail
                        mag_x = [p[0] for p in mag_buffer]
                        mag_y = [p[1] for p in mag_buffer]
                        mag_z = [p[2] for p in mag_buffer]
                        ax1.plot(mag_x, mag_y, mag_z, 'gray', alpha=0.5, linewidth=1)
                        
                        # Plot current point
                        current_mag = mag_buffer[-1]
                        ax1.scatter([current_mag[0]], [current_mag[1]], [current_mag[2]], 
                                   c='red', s=100, marker='o', edgecolors='black', linewidth=1.5)
                        
                        # Draw vector from origin
                        ax1.plot([0, current_mag[0]], [0, current_mag[1]], [0, current_mag[2]], 
                                'green', linewidth=2.5, alpha=0.8)
                    
                    # ====================================
                    # RIGHT PLOT: ACCELEROMETER
                    # ====================================
                    ax2.set_title(f'Accelerometer | X:{ax_val:6.2f} Y:{ay_val:6.2f} Z:{az_val:6.2f} g', 
                                  fontsize=10, fontweight='bold')
                    ax2.set_xlim([-2, 2])
                    ax2.set_ylim([-2, 2])
                    ax2.set_zlim([-2, 2])
                    ax2.set_xlabel('X (g)', fontsize=9)
                    ax2.set_ylabel('Y (g)', fontsize=9)
                    ax2.set_zlabel('Z (g)', fontsize=9)
                    ax2.grid(True, alpha=0.3)
                    
                    if len(acc_buffer) > 0:
                        # Plot trail
                        acc_x = [p[0] for p in acc_buffer]
                        acc_y = [p[1] for p in acc_buffer]
                        acc_z = [p[2] for p in acc_buffer]
                        ax2.plot(acc_x, acc_y, acc_z, 'gray', alpha=0.5, linewidth=1)
                        
                        # Plot current point
                        current_acc = acc_buffer[-1]
                        ax2.scatter([current_acc[0]], [current_acc[1]], [current_acc[2]], 
                                   c='blue', s=100, marker='o', edgecolors='black', linewidth=1.5)
                        
                        # Draw vector from origin
                        ax2.plot([0, current_acc[0]], [0, current_acc[1]], [0, current_acc[2]], 
                                'purple', linewidth=2.5, alpha=0.8)
                    
                    # Update display
                    plt.draw()
                    plt.pause(0.03)
                    
                    # Print to console every second (about 30 frames)
                    frame_count += 1
                    current_time = time.time()
                    if current_time - last_print >= 1.0:
                        print(f"📊 Mag: ({mx_val:6.1f}, {my_val:6.1f}, {mz_val:6.1f}) | "
                              f"Acc: ({ax_val:5.2f}, {ay_val:5.2f}, {az_val:5.2f}) | "
                              f"Temp: {temp_val:5.1f}°C")
                        last_print = current_time
                        frame_count = 0
                        
                except ValueError as e:
                    pass  # Skip bad data
        else:
            time.sleep(0.01)
            
except KeyboardInterrupt:
    print("\n\n⏹️ Stopping visualization...")
    
finally:
    ser.close()
    plt.close('all')
    print("👋 Serial connection closed")
    print("✅ Done!")
