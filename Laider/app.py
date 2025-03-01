import cv2
import base64
import threading
import time
import numpy as np
import serial
import struct
import json
from flask import Flask, jsonify, render_template_string, Response

# Configurazione LIDAR
LIDAR_SERIAL_PORT = "/dev/ttyS0"  # Porta seriale del LIDAR
BAUD_RATE_LIDAR = 230400
PACKET_LENGTH = 47
MEASUREMENT_LENGTH = 12
MESSAGE_FORMAT = "<xBHH" + "HB" * MEASUREMENT_LENGTH + "HHB"

# Configurazione motori
MOTOR_SERIAL_PORT = "/dev/ttyUSB0"  # Porta seriale per i motori
BAUD_RATE_MOTOR = 9600

# Configurazione della camera
camera = cv2.VideoCapture(0)
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

# Riconoscimento della palla rossa
lower_red = np.array([0, 200, 200])
upper_red = np.array([10, 255, 255])

# Variabili globali
latest_frame = ""
latest_lidar_data = {"x": [], "y": [], "c": []}
lidar_enabled = True
auto_tracking_enabled = True

# Avvia Flask
app = Flask(__name__)

# Funzione per analizzare i dati LIDAR
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

# Funzione per convertire i dati LIDAR in coordinate X, Y
def get_xyc_data(measurements):
    angle = np.radians([m[0] for m in measurements])
    distance = np.array([m[1] / 1000.0 for m in measurements])
    x = np.sin(angle) * distance
    y = np.cos(angle) * distance
    return x.tolist(), y.tolist(), [m[2] for m in measurements]

# Thread per leggere i dati LIDAR
def lidar_reader():
    global latest_lidar_data, lidar_enabled
    try:
        lidar_serial = serial.Serial(LIDAR_SERIAL_PORT, BAUD_RATE_LIDAR, timeout=0.5)
    except serial.SerialException as e:
        print(f"Errore nella connessione seriale LIDAR: {e}")
        return
    
    state = "SYNC0"
    data = b''
    measurements = []
    
    while True:
        if not lidar_enabled:
            time.sleep(0.1)
            continue
        
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
                        latest_lidar_data = {"x": x, "y": y, "c": c}
                        measurements = []
                else:
                    state = "SYNC0"
            time.sleep(0.01)
        except Exception as e:
            print(f"Errore durante la lettura del LIDAR: {e}")

# Funzione per inviare comandi ai motori
def control_motors(command):
    try:
        with serial.Serial(MOTOR_SERIAL_PORT, BAUD_RATE_MOTOR, timeout=1) as motor_serial:
            motor_serial.write(command.encode())
            print(f"Comando motore inviato: {command}")
    except serial.SerialException as e:
        print(f"Errore nella connessione seriale MOTORI: {e}")
# Funzione per calcolare la direzione della palla
def calculate_direction(ball_x, ball_y, frame_width, frame_height):
    center_x = frame_width // 2
    center_y = frame_height // 2

    if ball_x is None or ball_y is None:
        return "S"  # Stop se la palla non  rilevata

    # Calcola la differenza tra la posizione della palla e il centro
    delta_x = ball_x - center_x
    delta_y = ball_y - center_y

    # Soglie per decidere la direzione
    threshold = 50

    if abs(delta_x) < threshold and abs(delta_y) < threshold:
        return "S"  # Stop se la palla  vicina al centro
    elif delta_x < -threshold:
        return "L"  # Sinistra
    elif delta_x > threshold:
        return "R"  # Destra
    elif delta_y < -threshold:
        return "F"  # Avanti
    elif delta_y > threshold:
        return "B"  # Indietro
    else:
        return "S"  # Stop
    
# Funzione per catturare i frame della webcam
def capture_frames():
    global latest_frame
    while True:
        ret, frame = camera.read()
        if not ret:
            continue

        # Rileva la palla rossa
        ball_x, ball_y = track_red_ball(frame)

        # Se l'auto-tracking  abilitato, invia comandi ai motori
        if auto_tracking_enabled:
            direction = calculate_direction(ball_x, ball_y, frame.shape[1], frame.shape[0])
            control_motors(direction)

        # Codifica il frame in base64 per lo streaming
        _, buffer = cv2.imencode('.jpg', frame)
        latest_frame = base64.b64encode(buffer).decode('utf-8')
        time.sleep(0.5)
# Funzione per rilevare la palla rossa
def track_red_ball(frame):
    global latest_frame
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower_red, upper_red)
    contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if contours:
        c = max(contours, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        print("----Contours True-----X:", x, "----Y:", y, "----Radius:", radius)
        if radius > 1:
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
            latest_frame = frame
            return int(x), int(y)
    return None, None


# API per ottenere l'ultimo frame della webcam
@app.route('/frame')
def get_frame():
    return latest_frame

# API per ottenere gli ultimi dati LiDAR
@app.route('/lidar_data')
def get_lidar_data():
    return jsonify(latest_lidar_data)

# API per abilitare/disabilitare LIDAR e Auto-Tracking
@app.route('/enable_lidar')
def enable_lidar():
    global lidar_enabled
    lidar_enabled = True
    return "LIDAR abilitato"

@app.route('/disable_lidar')
def disable_lidar():
    global lidar_enabled
    lidar_enabled = False
    return "LIDAR disabilitato"

@app.route('/enable_auto_tracking')
def enable_auto_tracking():
    global auto_tracking_enabled
    auto_tracking_enabled = True
    return "Auto-Tracking abilitato"

@app.route('/disable_auto_tracking')
def disable_auto_tracking():
    global auto_tracking_enabled
    auto_tracking_enabled = False
    return "Auto-Tracking disabilitato"

# Pagina principale (HTML aggiornato con polling ogni 0.5s)
@app.route('/')
def index():
    return render_template_string(open("templates/indexV2.html").read())

if __name__ == '__main__':
    threading.Thread(target=capture_frames, daemon=True).start()
    threading.Thread(target=lidar_reader, daemon=True).start()
    app.run(host='0.0.0.0', port=5000, debug=False)
