from flask import Flask, request, render_template
import cv2
import numpy as np
import mss
import time
import matplotlib.pyplot as plt
import yolov5
import ctypes

app = Flask(__name__)

# Load DLL
interception_dll = ctypes.cdll.LoadLibrary("./interception.dll")
dll = ctypes.WinDLL('./lib.dll')
dll.move_mouse.restype = None

# Set value to default
ellipse_a = 40
ellipse_b = 40
speed = 3
head_switch_threshold = 10
delay = 0
head_point = 'head'

# Variable to store detection status
detection_running = False

@app.route('/', methods=['GET', 'POST'])
def index():
    global ellipse_a, ellipse_b, speed, head_switch_threshold, delay, head_point
    if request.method == 'POST':
        ellipse_a = int(request.form['ellipse_a'])
        ellipse_b = int(request.form['ellipse_b'])
        speed = int(request.form['speed'])
        head_switch_threshold = int(request.form['head_switch_threshold'])
        delay = float(request.form['delay'])
        head_point = request.form['head_point']
    return render_template('index.html', ellipse_a=ellipse_a, ellipse_b=ellipse_b, speed=speed, head_switch_threshold=head_switch_threshold, delay=delay, head_point=head_point)

@app.route('/start', methods=['GET'])
def start():
    global detection_running
    detection_running = True
    # Object detection and mouse movement code
    model = yolov5.load('latop.pt')
    model.conf = 0.15
    model.iou = 0.45
    model.agnostic = False
    model.multi_label = False
    model.max_det = 1000

    with mss.mss() as sct:
        monitor = {"top": 0, "left": 0, "width": 1920, "height": 1080}
        prev_head_x = 0
        while detection_running:
            img = sct.grab(monitor)
            frame = np.array(img)
            frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

            results = model(frame, size=640)

            for box in results.pred[0].numpy():
                x1, y1, x2, y2 = box[:4].astype(int)
                conf = box[4]
                class_id = int(box[5])

                obj_center_x = (x1 + x2) / 2
                obj_center_y = (y1 + y2) / 2
                half_width = (x2 - x1) / 2
                half_height = (y2 - y1) / 2
                triangle_pts = [
                    (obj_center_x, y1),
                    (x1 + half_width, obj_center_y),
                    (x2 - half_width, obj_center_y),
                ]

                cv2.polylines(frame, [np.array(triangle_pts, np.int32)], True, (255, 255, 255), 1)
                cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 255, 255), 1)

                if head_point == 'head':
                    head_x = int((x1 + x2) / 2)
                    head_y = int(y1 + 0.1 * (y2 - y1))
                if head_point == 'neck':
                    head_x = int((x1 + x2) / 2)
                    head_y = int(y1 + 0.2 * (y2 - y1))
                if head_point == 'body':
                    head_x = int(obj_center_x)
                    head_y = int(obj_center_y)

                cv2.circle(frame, (head_x, head_y), 5, (0, 0, 255), -1)

                # check if the head point is inside the ellipse
                ellipse_x, ellipse_y = 1920 // 2, 1080 // 2
                if abs(head_x - prev_head_x) > head_switch_threshold or prev_head_x == 0:
                    # If the current head is far enough away from the previous head, or if it's the first head detected,
                    # we focus on this head.
                    prev_head_x = head_x
                    if (head_x - ellipse_x)**2 / ellipse_a**2 + (head_y - ellipse_y)**2 / ellipse_b**2 <= 1:
                        distance_x = head_x - 1920 // 2
                        distance_y = head_y - 1080 // 2
                        # Déplacement de la souris
                        print(distance_x, distance_y)
                        dll.move_mouse(distance_x * speed, distance_y * speed)
                        time.sleep(delay)

    return 'Détection arrêtée'

@app.route('/stop', methods=['GET'])
def stop():
    global detection_running
    detection_running = False
    return 'Détection arrêtée'

if __name__ == '__main__':
    app.run(debug=True)
