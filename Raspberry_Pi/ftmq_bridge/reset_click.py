import RPi.GPIO as GPIO
import time
GPIO.setmode(GPIO.BOARD)
GPIO.setup(29, GPIO.OUT)
GPIO.output(29,GPIO.HIGH)
GPIO.output(29,GPIO.LOW)
time.sleep(1)
GPIO.output(29,GPIO.HIGH)
