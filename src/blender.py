
import bpy
import serial
import json


def poll():
    obj1 = bpy.data.objects["Cube"]
    obj1.rotation_mode = "QUATERNION"
    obj2 = bpy.data.objects["Cube.001"]
    obj2.rotation_mode = "QUATERNION"
    sPort = serial.Serial('/dev/tty.usbmodem205F366850421')
    sPort.readline()
    data = sPort.readline()
    try:
        jData = json.loads(data.decode("utf-8").strip().strip("\x00"))
    except:
        jData = { "q1": [0, 0, 0, 1], "q2": [0, 0, 0, 1] }
    obj1.rotation_quaternion[1] = jData["q1"][0];
    obj1.rotation_quaternion[2] = jData["q1"][1];
    obj1.rotation_quaternion[3] = jData["q1"][2];
    obj1.rotation_quaternion[0] = jData["q1"][3];
    obj2.rotation_quaternion[1] = jData["q2"][0];
    obj2.rotation_quaternion[2] = jData["q2"][1];
    obj2.rotation_quaternion[3] = jData["q2"][2];
    obj2.rotation_quaternion[0] = jData["q2"][3];
    return 0.016

bpy.app.timers.register(poll)

