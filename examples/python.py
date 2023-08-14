import genomix
import os

LIB_PATH = os.environ.get("DRONE_VV_PATH")
MODULE_PATH = os.path.join(LIB_PATH, "lib/genom/pocolibs/plugins", "Yolov8.so")

handle = genomix.connect("localhost:8080")

print("Module path: {}".format(MODULE_PATH))
yolo = handle.load(MODULE_PATH)

yolo.set_debug(1)

yolo.set_classes(
    classes=["person", "car", "truck", "bus", "bicycle", "motorbike", "cell phone"]
)
yolo.connect_port(local="ImageFrame", remote="MonocularCamera/Frame/raw")

yolo.start_object_detection()

# To get the detections
yolo.Detections()

yolo.stop_object_detection()
