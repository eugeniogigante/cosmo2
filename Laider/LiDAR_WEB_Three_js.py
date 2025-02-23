import numpy as np
import serial
import struct
import threading
import json
import time
from flask import Flask, render_template
from flask_socketio import SocketIO
import eventlet

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
    """Parsa i dati grezzi del LiDAR in angoli, distanze e confidenze."""
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
    """Converte i dati polarizzati in coordinate cartesiane."""
    angle = np.radians([m[0] for m in measurements])
    distance = np.array([m[1] / 1000.0 for m in measurements])
    x = np.sin(angle) * distance
    y = np.cos(angle) * distance
    return x.tolist(), y.tolist(), [m[2] for m in measurements]

def lidar_reader():
    """Legge continuamente i dati dal LiDAR e li trasmette via WebSocket."""
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
            if state == "SYNC0" and lidar_serial.read() == b'\x54':
                data = b'\x54'
                state = "SYNC1"
            elif state == "SYNC1" and lidar_serial.read() == b'\x2C':
                state = "SYNC2"
                data += b'\x2C'
            elif state == "SYNC2":
                data += lidar_serial.read(PACKET_LENGTH - 2)
                if len(data) == PACKET_LENGTH:
                    measurements += parse_lidar_data(data)
                    state = "LOCKED"
                else:
                    state = "SYNC0"
            elif state == "LOCKED":
                data = lidar_serial.read(PACKET_LENGTH)
                if data[0] == 0x54 and len(data) == PACKET_LENGTH:
                    measurements += parse_lidar_data(data)
                    if len(measurements) > 480:
                        x, y, c = get_xyc_data(measurements)
                        socketio.emit("lidar_data", json.dumps({"x": x, "y": y, "c": c, "lidar_x": 0, "lidar_y": 0}))
                        measurements = []
                else:
                    state = "SYNC0"
            time.sleep(0.01)  # Evita il sovraccarico della CPU
        except Exception as e:
            print(f"Errore durante la lettura del LIDAR: {e}")

@app.route('/')
def index():
    """Serve la pagina HTML per la visualizzazione dei dati."""
    return render_template('index2.html')

if __name__ == "__main__":
    threading.Thread(target=lidar_reader, daemon=True).start()
    socketio.run(app, host='0.0.0.0', port=5000)

# Creazione del file index2.html
html_content = """
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>LIDAR 3D Visualization</title>
    <script src='https://cdnjs.cloudflare.com/ajax/libs/socket.io/4.0.1/socket.io.js'></script>
</head>
<body>
    <h2>LIDAR 3D Map</h2>
    <script type='module'>
        import * as THREE from 'https://cdn.jsdelivr.net/npm/three@0.128/build/three.module.js';
        import { OrbitControls } from 'https://cdn.jsdelivr.net/npm/three@0.128/examples/jsm/controls/OrbitControls.js';
        
        var scene = new THREE.Scene();
        scene.background = new THREE.Color(0x111111);
        var camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 100);
        camera.position.set(0, 0, 10);
        var renderer = new THREE.WebGLRenderer();
        renderer.setSize(window.innerWidth, window.innerHeight);
        document.body.appendChild(renderer.domElement);
        
        var controls = new OrbitControls(camera, renderer.domElement);
        controls.update();
        
        var lidarGeometry = new THREE.SphereGeometry(0.2, 32, 32);
        var lidarMaterial = new THREE.MeshBasicMaterial({color: 0xff0000});
        var lidarPoint = new THREE.Mesh(lidarGeometry, lidarMaterial);
        scene.add(lidarPoint);
        
        function animate() {
            requestAnimationFrame(animate);
            controls.update();
            renderer.render(scene, camera);
        }
        animate();
    </script>
</body>
</html>
"""

with open("templates/index2.html", "w") as f:
    f.write(html_content)
