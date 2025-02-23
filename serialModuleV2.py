import serial
import time

# Define serial port and baud rate
port = "/dev/ttyS0"  # Serial port on Raspberry Pi Zero
baud_rate = 9600  # Baud rate for serial communication

# Open serial port
ser = serial.Serial(port, baud_rate, timeout=1)

try:
    while True:
        # Send a message
        message = "F"
        ser.write(message.encode())
        print("Message sent:", message)
        # Wait for a response
        #response = ser.readline().decode().strip()
        #print("Received:", response)
        # Delay for 5 seconds
        time.sleep(5)
        message = "S"
        ser.write(message.encode())
        print ("Stop")
        time.sleep(5)

finally:
    # Close the serial port
    ser.close()
