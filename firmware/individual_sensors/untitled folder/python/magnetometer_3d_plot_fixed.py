import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D
from collections import deque
import time

# --- CONFIGURATION ---
SERIAL_PORT = '/dev/cu.usbmodem190846701'  # Your Teensy port
BAUD_RATE = 115200
BUFFER_SIZE = 100

# --- Global Data Buffer ---
data_queue = deque(maxlen=BUFFER_SIZE)
serial_conn = None

def read_serial():
    """Read data from serial port in a non-blocking way"""
    global serial_conn
    try:
        serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
        print(f"✅ Connected to {SERIAL_PORT}")
    except Exception as e:
        print(f"❌ Failed to connect: {e}")
        return

    while True:
        if serial_conn and serial_conn.in_waiting:
            try:
                line = serial_conn.readline().decode('utf-8').strip()
                parts = line.split(',')
                if len(parts) == 3:
                    x = float(parts[0])
                    y = float(parts[1])
                    z = float(parts[2])
                    data_queue.append((x, y, z))
                    # Optional: print every 10th reading to avoid spam
                    if len(data_queue) % 10 == 0:
                        print(f"📊 Latest - X: {x:.1f}, Y: {y:.1f}, Z: {z:.1f}")
            except (ValueError, UnicodeDecodeError):
                pass
        time.sleep(0.01)  # Small delay to prevent CPU overload

# --- 3D Plot Update Function ---
def update_plot(frame):
    """Update the 3D plot with latest data"""
    ax.clear()
    
    # Set fixed axis limits
    ax.set_xlim([-150, 150])
    ax.set_ylim([-150, 150])
    ax.set_zlim([-150, 150])
    
    # Labels and title
    ax.set_xlabel('X (μT)', fontsize=10)
    ax.set_ylabel('Y (μT)', fontsize=10)
    ax.set_zlabel('Z (μT)', fontsize=10)
    ax.set_title('LIS2MDL Real-Time Magnetometer', fontsize=12)
    
    if len(data_queue) > 0:
        # Get latest point
        latest_x, latest_y, latest_z = data_queue[-1]
        
        # Convert deque to lists for plotting
        xs = [p[0] for p in data_queue]
        ys = [p[1] for p in data_queue]
        zs = [p[2] for p in data_queue]
        
        # Plot the trail
        ax.plot(xs, ys, zs, color='blue', alpha=0.6, linewidth=1.5)
        
        # Plot current point as red sphere
        ax.scatter([latest_x], [latest_y], [latest_z], 
                  c='red', s=120, marker='o', edgecolors='black', linewidth=1)
        
        # Draw vector from origin
        ax.plot([0, latest_x], [0, latest_y], [0, latest_z], 
                color='green', linewidth=2.5, alpha=0.8)
        
        # Add text annotation for current values
        ax.text2D(0.02, 0.95, f'X: {latest_x:.1f} μT\nY: {latest_y:.1f} μT\nZ: {latest_z:.1f} μT',
                 transform=ax.transAxes, fontsize=10,
                 bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    return []

# --- Main Execution ---
if __name__ == "__main__":
    print("🚀 Starting Magnetometer 3D Visualizer")
    print("=" * 50)
    
    # Start serial reading in a separate thread
    import threading
    serial_thread = threading.Thread(target=read_serial, daemon=True)
    serial_thread.start()
    
    # Wait a moment for serial connection to establish
    time.sleep(2)
    
    if not serial_conn or not serial_conn.is_open:
        print("❌ Could not establish serial connection. Check:")
        print("   1. Teensy is plugged in")
        print("   2. Correct port is selected")
        print("   3. Serial Monitor is closed")
        exit(1)
    
    # Setup the plot
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection='3d')
    
    # Create animation with proper settings
    ani = FuncAnimation(fig, update_plot, interval=50, cache_frame_data=False)
    
    print("✅ Visualization running! Close the plot window to exit.")
    print("📡 Move your magnetometer to see the 3D vector change")
    print("=" * 50)
    
    # Show the plot (this blocks until window is closed)
    plt.show()
    
    # Cleanup
    if serial_conn and serial_conn.is_open:
        serial_conn.close()
        print("👋 Serial connection closed")
