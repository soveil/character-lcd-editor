import serial
import serial.tools.list_ports
import sys
import termios
import time


def main():
    port = find_arduino() if len(sys.argv) < 2 else sys.argv[1]
    fd = sys.stdin.fileno()
    orig = termios.tcgetattr(fd)

    new = termios.tcgetattr(fd)
    new[3] = new[3] & ~termios.ECHO & ~termios.ICANON | termios.ISIG
    new[6][termios.VMIN] = 1
    new[6][termios.VTIME] = 0

    if not port:
        print("Arduino not found")
        return

    try:
        arduino = serial.Serial(port, 9600, timeout=0)
    except serial.SerialException:
        print(f"Arduino not found at '{port}'")
    else:
        with arduino:
            time.sleep(2)
            print(f"Connected to {arduino.name}")
            print("Ready to stream key presses")
            while True:
                lines = arduino.readall()
                if len(lines) > 0:
                    print(lines.decode().strip())
                arduino.write(getch(fd, orig, new).encode())
                lines = arduino.readall()
                if len(lines) > 0:
                    print(lines.decode().strip())



def find_arduino():
    for port in serial.tools.list_ports.comports():
        if port.vid == 9025:
            return port.device
    return None


def getch(fd, orig, new) -> str:
    try:
        termios.tcsetattr(fd, termios.TCSANOW, new)
        return sys.stdin.read(1)
    except KeyboardInterrupt:
        sys.exit()
    finally:
        termios.tcsetattr(fd, termios.TCSANOW, orig)


if __name__ == "__main__":
    main()
