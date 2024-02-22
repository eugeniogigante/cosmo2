import cv2
import numpy as np
#import RPi.GPIO as GPIO
from time import sleep
from flask import Flask, render_template, Response

# Inizializzazione del modulo L298N motor controller
#GPIO.setmode(GPIO.BOARD)
motor1_pins = [3, 5, 7]  # E' necessario adattare i pin in base alla tua configurazione
motor2_pins = [11, 13, 15]

#for pin in motor1_pins + motor2_pins:
    #GPIO.setup(pin, GPIO.OUT)
    #GPIO.output(pin, 0)

# Definizione delle funzioni per il controllo dei motori
def set_motor_speed(motor_pins, speed):
    #GPIO.output(motor_pins[0], speed > 0)
    #GPIO.output(motor_pins[1], speed < 0)
    #GPIO.output(motor_pins[2], GPIO.HIGH)
    pass

def set_motors(left_speed, right_speed):
    set_motor_speed(motor1_pins, left_speed)
    set_motor_speed(motor2_pins, right_speed)

# Inizializzazione della camera
camera = cv2.VideoCapture(0)  # Modifica il numero della telecamera se necessario

# Definizione dei parametri per il riconoscimento della palla rossa
lower_red = np.array([0, 200, 200])
upper_red = np.array([10, 255, 255])

# Definizione della funzione per il rilevamento e il tracking della palla
def track_red_ball(frame):

    font                   = cv2.FONT_HERSHEY_SIMPLEX
    bottomLeftCornerOfText = (10,20)
    fontScale              = 1
    fontColor              = (255,255,255)
    thickness              = 1
    lineType               = 2

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower_red, upper_red)
    contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if contours:
        c = max(contours, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        if radius > 10:
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
            positionxy = str(int(x)) + '---' + str(int(y))
            frame = cv2.putText(frame, positionxy, 
                bottomLeftCornerOfText, 
                font, 
                fontScale,
                fontColor,
                thickness,
                lineType)
            return int(x), int(y)
    return None, None

# Definizione della funzione per il controllo del rover
def control_rover(x, y, frame_center):
    commands = ''
    if x is not None:
        if x < frame_center - 50:
            set_motors(0.5, -0.5)  # Ruota a sinistra
            commands='Ruota a sinistra'
        elif x > frame_center + 50:
            set_motors(-0.5, 0.5)  # Ruota a destra
            commands='Ruota a destra'
        else:
            set_motors(0.5, 0.5)  # Avanza
            commands='Avanza'
    else:
        set_motors(0, 0)  # Ferma il rover se non vede la palla
        commands='non vede la palla'
    return commands

# Definizione dell'app Flask per la visualizzazione del video e il controllo del rover
app = Flask(__name__)

@app.route('/')
def index():
    return render_template('indexfollowball.html')

def gen():
    font                   = cv2.FONT_HERSHEY_SIMPLEX
    bottomLeftCornerOfText = (10,100)
    fontScale              = 1
    fontColor              = (255,255,255)
    thickness              = 1
    lineType               = 2

    while True:
        ret, frame = camera.read()
        frame_center = frame.shape[1] // 2
        x, y = track_red_ball(frame)
        commands = control_rover(x, y, frame_center)
        frame = cv2.putText(frame, commands, 
                bottomLeftCornerOfText, 
                font, 
                fontScale,
                fontColor,
                thickness,
                lineType)
        _, jpeg = cv2.imencode('.jpg', frame)
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n')

@app.route('/video_feed')
def video_feed():
    return Response(gen(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/move_forward')
def moveForward():
    print('forward')
    return Response(set_motors(0.5, 0.5))

@app.route('/move_left')
def moveleft():
    print('left')
    return Response(set_motors(0.5, -0.5))

@app.route('/move_right')
def moveright():
    print('right')
    return Response(set_motors(-0.5, 0.5))

@app.route('/move_stop')
def movestop():
    print('stop')
    return Response(set_motors(0, 0))

@app.route('/move_backward')
def movebackward():
    print('backward')
    return Response(set_motors(-0.5, -0.5))

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True)
