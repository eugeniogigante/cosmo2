import cv2
import numpy as np
import RPi.GPIO as GPIO
from time import sleep
from flask import Flask, render_template, Response, request
#from gpiozero import AngularServo
from time import sleep
import serial


# Inizializzazione del modulo L298N motor controller
GPIO.setmode(GPIO.BCM)
motor1_pins = [4, 27, 22]  # E' necessario adattare i pin in base alla tua configurazione
motor2_pins = [23, 24, 25]

servo_pin = 13
GPIO.setup(servo_pin, GPIO.OUT)
#servo = AngularServo(13, min_pulse_width=0.0006, max_pulse_width=0.0023)

# Define serial port and baud rate
port = "/dev/ttyS0"  # Serial port on Raspberry Pi Zero
baud_rate = 9600  # Baud rate for serial communication
# Open serial port
ser = serial.Serial(port, baud_rate, timeout=1)

# Create PWM instance
pwm = GPIO.PWM(servo_pin, 50)  # 50 Hz (20 ms PWM period)

for pin in motor1_pins + motor2_pins:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, 0)

# Definizione delle funzioni per il controllo dei motori
def set_motor_speed(motor_pins, speed):
    GPIO.output(motor_pins[0], speed > 0)
    GPIO.output(motor_pins[1], speed < 0)
    GPIO.output(motor_pins[2], GPIO.HIGH)
    #pass

# Function to set servo angle
def set_angle(angle):
    duty = angle / 18 + 2
    GPIO.output(servo_pin, True)
    pwm.ChangeDutyCycle(duty)
    sleep(1)  # Allow time for the servo to move
    GPIO.output(servo_pin, False)
    pwm.ChangeDutyCycle(0)
    #pwm.stop()
    #GPIO.cleanup()

def set_motors(left_speed, right_speed):
    set_motor_speed(motor1_pins, left_speed)
    set_motor_speed(motor2_pins, right_speed)

# Inizializzazione della camera
camera = cv2.VideoCapture(0)
#camera = cv2.VideoCapture(0, cv2.CAP_DSHOW)  # Modifica il numero della telecamera se necessario
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

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
            
            #set_angle(40)
            #sleep(1)
            #set_angle(180)
            #sleep(3)
            return int(x), int(y)
    return None, None

# Definizione della funzione per il controllo del rover
def control_rover(x, y, frame_center):
    commands = ''
    if x is not None:
        if x < frame_center - 50:
            set_motors(0.5, -0.5)  # Ruota a Left
            commands='Ruota a sinistra'
            message = "L"
            ser.write(message.encode())
        elif x > frame_center + 50:
            set_motors(-0.5, 0.5)  # Ruota a Right
            commands='Ruota a destra'
            message = "R"
            ser.write(message.encode())
        else:
            set_motors(0.5, 0.5)  # forward
            commands='Avanza'
            message = "F"
            ser.write(message.encode())
    else:
        set_motors(0, 0)  # Ferma il rover se non vede la palla
        commands='non vede la palla'
        message = "S"
        ser.write(message.encode())
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
    message = "F"
    ser.write(message.encode())
    return Response(set_motors(0.5, 0.5))

@app.route('/move_left')
def moveleft():
    message = "L"
    ser.write(message.encode())
    print('left')
    return Response(set_motors(0.5, -0.5))

@app.route('/move_right')
def moveright():
    message = "R"
    ser.write(message.encode())
    print('right')
    return Response(set_motors(-0.5, 0.5))

@app.route('/move_stop')
def movestop():
    print('stop')
    message = "S"
    ser.write(message.encode())
    return Response(set_motors(0, 0))

@app.route('/move_backward')
def movebackward():
    print('backward')
    message = "B"
    ser.write(message.encode())   
    return Response(set_motors(-0.5, -0.5))

@app.route('/update_servo', methods=['POST'])
def update_servo():
    if request.method =='POST':
       angle = int(request.form.get('angle'))
       pwm.start(0)
       set_angle(angle)
       print(angle)
       return Response('OK', angle)
    else:
       return Response('error servo')

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False)
