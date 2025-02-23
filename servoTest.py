import RPi.GPIO as GPIO
import time

# Set GPIO mode
GPIO.setmode(GPIO.BCM)

# Set pin 32 as output
servo_pin = 13
GPIO.setup(servo_pin, GPIO.OUT)

# Create PWM instance
pwm = GPIO.PWM(servo_pin, 50)  # 50 Hz (20 ms PWM period)

# Function to set servo angle
def set_angle(angle):
    duty = angle / 18 + 2
    GPIO.output(servo_pin, True)
    pwm.ChangeDutyCycle(duty)
    time.sleep(1)  # Allow time for the servo to move
    GPIO.output(servo_pin, False)
    pwm.ChangeDutyCycle(0)

try:
    # Start PWM
    pwm.start(0)

    while True:
        # Move the servo to 0 degrees
        set_angle(40)
        time.sleep(1)

        # Move the servo to 90 degrees
        set_angle(90)
        time.sleep(1)

        # Move the servo to 180 degrees
        set_angle(180)
        time.sleep(1)

except KeyboardInterrupt:
    # Clean up GPIO
    pwm.stop()
    GPIO.cleanup()

