import subprocess

def rumble(weak_motor, strong_motor, duration, joystick=0):
    c_rumble = subprocess.run(['./rumble', str(joystick), str(strong_motor), str(weak_motor), str(duration)], stdout=subprocess.PIPE)

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

    return c_rumble.returncode
