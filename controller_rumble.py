import subprocess

#   rumble [ joystick (1-8) ]  [strong motor magnitude (%) ]  [weak motor magnitude (%) ]  [duration (miliseconds) ]

# output = subprocess.run(['./rumble', '0', '100', '100', '1000'], stdout=subprocess.PIPE)
# print(output.returncode)

def rumble(weak_motor, strong_motor, duration, joystick=0):
    c_rumble = subprocess.run(['./rumble', str(joystick), str(strong_motor), str(weak_motor), str(duration)], stdout=subprocess.PIPE)
    # print(c_rumble)
    if c_rumble.returncode == 0:
        pass
    elif c_rumble.returncode == 1:
        print("Can't Rumble.")
    elif c_rumble.returncode == 2:
        print("No Rumble.")
    elif c_rumble.returncode == 3:
        print("Error Playing Effect.")
    elif c_rumble.returncode == 4:
        print("Rumble requires sudo!")

    return
