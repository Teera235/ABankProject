from svgpathtools import svg2paths
import pyautogui
import time

pyautogui.PAUSE = 0.01
pyautogui.FAILSAFE = False

svg_file = r"C:\Users\teerathap\Desktop\My Project\ABankProject\jetpack-28824.svg"
paths, attributes = svg2paths(svg_file)

print("เตรียมหน้าจอสำหรับวาดใน 5 วินาที...")
time.sleep(5)q
start_x, start_y = 500, 300

def complex_to_screen_coords(c):
    return start_x + c.real, start_y + c.imag

for path in paths:
    start_point = complex_to_screen_coords(path[0].start)
    pyautogui.moveTo(*start_point)
    pyautogui.mouseDown()

    for seg in path:
        # sample จุดน้อยลง เพื่อวาดเร็ว
        for t in [i/5 for i in range(21)]:  # 6 จุดต่อ segment เท่านั้น
            point = seg.point(t)
            x, y = complex_to_screen_coords(point)
            pyautogui.moveTo(x, y, duration=0)

    pyautogui.mouseUp()

print("✅ วาดเสร็จเรียบร้อยแล้ว!")
