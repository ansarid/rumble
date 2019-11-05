# rumble
Rumble a GamePad with C and Python.

## Compile
#### Requirements:
```
sudo apt install libc6-dev-i386
```
*Not needed on BeagleBone.

#### GCC:
```
gcc rumble.c -o rumble
```

### Usage

#### C:
```
rumble [ joystick (1-8) ]  [strong motor magnitude (%) ]  [weak motor magnitude (%) ]  [duration (miliseconds) ]
```
#### Python:
```
python3 joystick_rumble.py
```
