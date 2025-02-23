from gpiozero import AngularServo
from time import sleep

servo = AngularServo(13, min_pulse_width=0.0006, max_pulse_width=0.0010)

servo.angle = 90

sleep(10)

servo.angle = 10
