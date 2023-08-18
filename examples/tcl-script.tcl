# This script is not tested but provides an example of how to use the Yolov8 module
package require genomix
package require env

set LIB_PATH [env get DRONE_VV_PATH]
set MODULE_PATH [file join $LIB_PATH "lib/genom/pocolibs/plugins" "Yolov8.so"]

set handle [genomix connect "localhost:8080"]

puts "Module path: $MODULE_PATH"
$handle load $MODULE_PATH

yolov8::set_debug 1

yolov8::set_classes -classes {person car truck bus bicycle motorbike cell phone}
yolov8::connect_port -local ImageFrame -remote MonocularCamera/Frame/raw

yolov8::start_object_detection

# To get the detections
yolov8::Detections

# To pause / resume the detection
yolov8::pause_object_detection
yolov8::resume_object_detection

yolov8::stop_object_detection