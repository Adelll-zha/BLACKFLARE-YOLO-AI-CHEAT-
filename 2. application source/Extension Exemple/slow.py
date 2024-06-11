import ctypes
import time

interception_dll = ctypes.cdll.LoadLibrary("./interception.dll")
# Charger le DLL
dll = ctypes.windll.LoadLibrary('./BLACKFLARE DRIVER.dll')


# Définir la fonction slow_down_mouse
slow_down_mouse = dll.slow_down_mouse
slow_down_mouse.restype = None


# Appeler la fonction slow_down_mouse
slow_down_mouse(True, 2)  # Ralentir le curseur
print("ok")