import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D
import threading
from collections import deque
import atexit

# --- CONFIGURATION ---
SERIAL_PORT = '/dev/cu.usbmodem190846701'  # Your Teensy port
BAUD_RATE = 115200
BUFFER_SIZE = 100

# --- Global Data Buffer ---
data_queue = deque(maxlen=BUFFER_SIZE)

# --- Serial Reading Thread ---
serial_conn = None

def read_serial():
    global serial_conn
    try:
        serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to {SERIAL_PORT}")
    except Exception as e:
        print(f"Failed to connect to {SERIAL_PORT}: {e}")
        return

    while True:
        if serial_conn and serial_conn.in_waiting:
            try:
                line = serial_conn.readline().decode('utf-8').strip()
                parts = line.split(',')
                if len(parts) == 3:
                    x, y, z = float(parts[0]), float(parts[1]), float(parts[2])
                    data_queue.append((x, y, z))
                    print(f"X: {x:.2f}, Y: {y:.2f}, Z: {z:.2f}")  # Debug print
            except (ValueError, UnicodeDecodeError):
                pass

def cleanup():
    if serial_conn and serial_conn.is_open:
        serial_conn.close()
        print("Serial connection closed.")

def init_plot():
    ax.set_xlim([-100, 100])
    ax.set_ylim([-100, 100])
    ax.set_zlim([-100, 100])
    ax.set_xlabel('X Axis (uT)')
    ax.set_ylabel('Y Axis (uT)')
    ax.set_zlabel('Z Axis (uT)')
    ax.set_title('Real-Time LIS2MDL Magnetometer Vector')
    return []

def update_plot(frame):
    ax.cla()
    ax.set_xlim([-100, 100])
    ax.set_ylim([-100, 100])
    ax.set_zlim([-100, 100])
    ax.set_xlabel('X (uT)')
    ax.set_ylabel('Y (uT)')
    ax.set_zlabel('Z (uT)')
    ax.set_title('3D Magnetic Field Vector')

    if len(data_queue) > 0:
        latest_x, latest_y, latest_z = data_queue[-1]
        
        xs = [p[0] for p in data_queue]
        ys = [p[1] for p in data_queue]
        zs = [p[2] for p in data_queue]
        
        ax.plot(xs, ys, zs, color='blue', alpha=0.7, linewidth=2)
        ax.scatter([latest_x], [latest_y], [latest_z], c='red', s=100, marker='o')
        ax.plot([0, latest_x], [0, latest_y], [0, latest_z], color='green', linewidth=3)
        
    return []

if __name__ == "__main__":
    serial_thread = threading.Thread(target=read_serial, daemon=True)
    serial_thread.start()
    
    atexit.register(cleanup)
    
    plt.ion()
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')
    
    ani = FuncAnimation(fig, update_plot, init_func=init_plot, interval=50, blit=False)
    
    print("Starting 3D plot. Close the window to exit.")
    plt.show()
    
    cleanup()
