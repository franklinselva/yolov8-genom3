
#include "acYolov8.h"
#include "fg3utils/trace_f.h"
#include "fg3utils/macros.h"
#include <string>
#include <fstream>

#include "utils.h"
#include "detection/inference.h"
#include <opencv2/opencv.hpp>

#include "Yolov8_c_types.h"

Inference inf(cv::Size(640, 480), true);

/* --- Task detect_objects ---------------------------------------------- */

/** Codel FetchPorts of task detect_objects.
 *
 * Triggered by Yolov8_start.
 * Yields to Yolov8_pause_start, Yolov8_main.
 * Throws Yolov8_e_BAD_IMAGE_PORT, Yolov8_e_OPENCV_ERROR,
 *        Yolov8_e_BAD_CONFIG, Yolov8_e_OUT_OF_MEM.
 */
genom_event
FetchPorts(bool start_detection, const Yolov8_ImageFrame *ImageFrame,
           const Yolov8_Detections *Detections,
           const char *resource_path, bool debug,
           const genom_context self)
{
  if (!start_detection)
  {
    if (debug)
    {
      CODEL_LOG_WARNING("Detection not started");
    }
    return Yolov8_pause_start;
  }

  // Check if all ports are connected and available
  if (!check_port_in_p(ImageFrame))
  {
    CODEL_LOG_WARNING("Image port not connected");
    return Yolov8_pause_start;
  }

  if (debug)
  {
    CODEL_LOG_INFO(2, 1, "All ports connected, fetching data");
  }

  // Reserve memory for detections
  if (genom_sequence_reserve(&(Detections->data(self)->detections), 20) == -1)
  {
    Yolov8_e_OUT_OF_MEM_detail msg;
    snprintf(msg.message, sizeof(msg.message), "%s", "Failed to reserve memory for plates");
    // warnx("%s", msg.message);
    return Yolov8_e_OUT_OF_MEM(&msg, self);
  }

  // Load model
  const char *pkg_config_cmd = "pkg-config Yolov8-genom3 --variable=datarootdir";
  std::string package_shared_dir = executeCommand(pkg_config_cmd);
  std::string model_path = package_shared_dir + std::string("/yolov8-genom3/models/yolov8s.onnx");
  std::string classes_path = package_shared_dir + std::string("/yolov8-genom3/models/classes.txt");

  inf.loadModel(model_path, classes_path);

  return Yolov8_main;
}

/** Codel DetectObjects of task detect_objects.
 *
 * Triggered by Yolov8_main.
 * Yields to Yolov8_main, Yolov8_pause_main, Yolov8_ether.
 * Throws Yolov8_e_BAD_IMAGE_PORT, Yolov8_e_OPENCV_ERROR,
 *        Yolov8_e_BAD_CONFIG, Yolov8_e_OUT_OF_MEM.
 */
genom_event
DetectObjects(const sequence_string *classes,
              const Yolov8_ImageFrame *ImageFrame,
              const Yolov8_Detections *Detections, bool debug,
              bool show_frames, const genom_context self)
{

  or_sensor_frame *image_frame;
  if (ImageFrame->read(self) == genom_ok && ImageFrame->data(self))
    image_frame = ImageFrame->data(self);
  else
  {
    Yolov8_e_BAD_IMAGE_PORT_detail msg;
    snprintf(msg.message, sizeof(msg.message), "%s", "Failed to read image port. waiting");
    CODEL_LOG_ERROR(msg.message);
    return Yolov8_e_BAD_IMAGE_PORT(&msg, self);
  }

  // Release memory for detections
  Detections->data(self)->detections._length = 0;

  bool is_object_found = false;
  // Convert frame to cv::Mat
  cv::Mat image;
  if (image_frame->compressed)
  {
    std::vector<uint8_t> buf;
    buf.assign(image_frame->pixels._buffer, image_frame->pixels._buffer + image_frame->pixels._length);
    imdecode(buf, cv::IMREAD_COLOR, &image);
  }
  else
  {
    int type;
    if (image_frame->bpp == 1)
      type = CV_8UC1;
    else if (image_frame->bpp == 2)
      type = CV_16UC1;
    else if (image_frame->bpp == 3)
      type = CV_8UC3;
    else if (image_frame->bpp == 4)
      type = CV_8UC4;
    else
    {
      Yolov8_e_BAD_IMAGE_PORT_detail *msg;
      snprintf(msg->message, sizeof(msg->message), "%s", "Invalid image bpp");
      return Yolov8_e_BAD_IMAGE_PORT(msg, self);
    }

    image = cv::Mat(
        cv::Size(image_frame->width, image_frame->height),
        type,
        image_frame->pixels._buffer,
        cv::Mat::AUTO_STEP);
  }

  // Resize image
  cv::resize(image, image, cv::Size(640, 480)); // Should match the input size of the model
  std::vector<Detection> detections = inf.runInference(image);

  // Log detections
  if (debug)
  {
    size_t no_detections = detections.size();
    CODEL_LOG_INFO(2, 1, "Found %d objects", no_detections);
  }

  // Check if the object is found
  for (auto &detection : detections)
  {
    if (detection.confidence > 0.5)
    {
      is_object_found = true;

      // Check the detected class in the list of classes
      if (std::find(classes->_buffer, classes->_buffer + classes->_length, detection.className) != classes->_buffer + classes->_length)
      {
        if (show_frames)
        {
          // Convert the bounding box to the original image size
          detection.box.x = detection.box.x * image_frame->width / 640;
          detection.box.y = detection.box.y * image_frame->height / 480;
          detection.box.width = detection.box.width * image_frame->width / 640;
          detection.box.height = detection.box.height * image_frame->height / 480;

          // Draw the bounding box
          cv::rectangle(image, detection.box, cv::Scalar(0, 255, 0), 2);

          // Draw the class name
          cv::putText(image, detection.className, cv::Point(detection.box.x, detection.box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
        }
      }

      // Add the detection to the list
      or_Yolo_Detection detectionData;
      detectionData.bbox.x = detection.box.x;
      detectionData.bbox.y = detection.box.y;
      detectionData.bbox.w = detection.box.width;
      detectionData.bbox.h = detection.box.height;
      detectionData.class_id = detection.class_id;
      detectionData.label = const_cast<char *>(detection.className.c_str());
      detectionData.confidence = detection.confidence;

      Detections->data(self)->detections._buffer[Detections->data(self)->detections._length] = detectionData;
      Detections->data(self)->detections._length++;
    }
  }

  // Publish the image
  if (show_frames)
  {
    cv::imshow("Yolov8", image);
    cv::waitKey(1);
  }

  Detections->data(self)->image_frame = *image_frame;
  Detections->write(self);

  return Yolov8_main;
}

/* --- Activity start_object_detection ---------------------------------- */

/** Codel SetupClasses of activity start_object_detection.
 *
 * Triggered by Yolov8_start.
 * Yields to Yolov8_setup, Yolov8_pause_start, Yolov8_ether.
 * Throws Yolov8_e_BAD_IMAGE_PORT, Yolov8_e_OPENCV_ERROR,
 *        Yolov8_e_BAD_CONFIG, Yolov8_e_OUT_OF_MEM.
 */
genom_event
SetupClasses(char **resource_path, sequence_string *classes,
             bool debug, const genom_context self)
{

  const char *pkg_config_cmd = "pkg-config Yolov8-genom3 --variable=datarootdir";
  std::string package_shared_dir = executeCommand(pkg_config_cmd);

  std::string classes_path = package_shared_dir + "/yolov8-genom3/models/classes.txt";
  std::ifstream classes_file(classes_path);
  if (!classes_file.is_open())
  {
    char message[128];
    snprintf(message, sizeof(message), "Could not open classes file %s",
             classes_path.c_str());
    CODEL_LOG_WARNING(message);
    Yolov8_e_BAD_CONFIG_detail *d;
    strcpy(d->message, message);
    return Yolov8_e_BAD_CONFIG(d, self);
  }

  // Read classes from file
  std::string line;
  sequence_string classes_vector;
  if (genom_sequence_reserve(&(classes_vector), 80) == -1)
  {
    Yolov8_e_OUT_OF_MEM_detail msg;
    snprintf(msg.message, sizeof(msg.message), "%s", "Failed to reserve memory for plates");
    // warnx("%s", msg.message);
    return Yolov8_e_OUT_OF_MEM(&msg, self);
  }

  while (std::getline(classes_file, line))
  {
    classes_vector._buffer = (char **)realloc(
        classes_vector._buffer, (classes_vector._length + 1) * sizeof(char *));
    classes_vector._buffer[classes_vector._length] = (char *)malloc(line.size() + 1);
  }

  if (classes_vector._length == 0)
  {
    char message[128];
    snprintf(message, sizeof(message), "Classes file %s is empty",
             classes_path.c_str());
    CODEL_LOG_WARNING(message);
    Yolov8_e_BAD_CONFIG_detail *d;
    strcpy(d->message, message);
    return Yolov8_e_BAD_CONFIG(d, self);
  }

  if (debug)
  {
    CODEL_LOG_INFO(2, 1, "Found %d classes in file %s:", classes_vector._length,
                   classes_path.c_str());
  }

  // Set classes to detect
  if (classes->_length > 0)
  {
    // Check if provided classes are inside the file
    for (int i = 0; i < classes->_length; i++)
    {
      if (debug)
      {
        CODEL_LOG_INFO(2, 1, "  %s", classes->_buffer[i]);
      }
    }
  }
  else
  {
    // Use all classes from file
    classes->_length = classes_vector._length;
    classes->_buffer = classes_vector._buffer;
  }

  if (debug)
  {
    CODEL_LOG_INFO(2, 1, "Will detect %d classes:", classes->_length);
  }

  return Yolov8_setup;
}

/** Codel SetStartDetection of activity start_object_detection.
 *
 * Triggered by Yolov8_setup.
 * Yields to Yolov8_pause_setup, Yolov8_ether.
 * Throws Yolov8_e_BAD_IMAGE_PORT, Yolov8_e_OPENCV_ERROR,
 *        Yolov8_e_BAD_CONFIG, Yolov8_e_OUT_OF_MEM.
 */
genom_event
SetStartDetection(bool *start_detection, const genom_context self)
{
  *start_detection = true;
  return Yolov8_ether;
}

/* --- Activity stop_object_detection ----------------------------------- */

/** Codel SetStopDetection of activity stop_object_detection.
 *
 * Triggered by Yolov8_start.
 * Yields to Yolov8_ether.
 * Throws Yolov8_e_BAD_IMAGE_PORT, Yolov8_e_OPENCV_ERROR,
 *        Yolov8_e_BAD_CONFIG, Yolov8_e_OUT_OF_MEM.
 */
genom_event
SetStopDetection(bool *start_detection, const genom_context self)
{
  *start_detection = false;
  return Yolov8_ether;
}
