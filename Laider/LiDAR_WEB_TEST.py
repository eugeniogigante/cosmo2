import numpy as np
import serial
import struct
import threading
import json
import time
from flask import Flask, render_template
from flask_socketio import SocketIO
import eventlet
eventlet.monkey_patch()

# Configurazione Raspberry Pi
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 230400
PACKET_LENGTH = 47
MEASUREMENT_LENGTH = 12
MESSAGE_FORMAT = "<xBHH" + "HB" * MEASUREMENT_LENGTH + "HHB"

# Avvia Flask e WebSocket
app = Flask(__name__, template_folder="templates")
socketio = SocketIO(app, cors_allowed_origins="*")

def parse_lidar_data(data):
    length, speed, start_angle, *pos_data, stop_angle, timestamp, crc = struct.unpack(MESSAGE_FORMAT, data)
    start_angle /= 100.0
    stop_angle /= 100.0
    if stop_angle < start_angle:
        stop_angle += 360.0
    step_size = (stop_angle - start_angle) / (MEASUREMENT_LENGTH - 1)
    angles = [start_angle + step_size * i for i in range(MEASUREMENT_LENGTH)]
    distances = pos_data[0::2]
    confidences = pos_data[1::2]
    return list(zip(angles, distances, confidences))

def get_xyc_data(measurements):
    angle = np.radians([m[0] for m in measurements])
    distance = np.array([m[1] / 1000.0 for m in measurements])
    x = np.sin(angle) * distance
    y = np.cos(angle) * distance
    return x.tolist(), y.tolist(), [m[2] for m in measurements]

def lidar_reader():
    try:
        lidar_serial = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.5)
    except serial.SerialException as e:
        print(f"Errore nella connessione seriale: {e}")
        return
    
    state = "SYNC0"
    data = b''
    measurements = []
    
    while True:
        try:
            if state == "SYNC0":
                data = b''
                measurements = []
                if lidar_serial.read() == b'\x54':
                    data = b'\x54'
                    state = "SYNC1"
            elif state == "SYNC1":
                if lidar_serial.read() == b'\x2C':
                    state = "SYNC2"
                    data += b'\x2C'
                else:
                    state = "SYNC0"
            elif state == "SYNC2":
                data += lidar_serial.read(PACKET_LENGTH - 2)
                if len(data) != PACKET_LENGTH:
                    state = "SYNC0"
                    continue
                measurements += parse_lidar_data(data)
                state = "LOCKED"
            elif state == "LOCKED":
                data = lidar_serial.read(PACKET_LENGTH)
                if data[0] != 0x54 or len(data) != PACKET_LENGTH:
                    state = "SYNC0"
                    continue
                measurements += parse_lidar_data(data)
                if len(measurements) > 480:
                    x, y, c = get_xyc_data(measurements)
                    socketio.emit("lidar_data", json.dumps({"x": x, "y": y, "c": c}))
                    measurements = []
            time.sleep(0.01)  # Evita il sovraccarico della CPU
        except Exception as e:
            print(f"Errore durante la lettura del LIDAR: {e}")

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == "__main__":
    threading.Thread(target=lidar_reader, daemon=True).start()
    socketio.run(app, host='0.0.0.0', port=5000)
