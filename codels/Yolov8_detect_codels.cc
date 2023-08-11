
#include "acYolov8.h"
#include "fg3utils/trace_f.h"
#include "fg3utils/macros.h"
#include <cstdio>

#include "Yolov8_c_types.h"
#include "detection/inference.h"
#include <string>
#include <cstdlib>

/* --- Setup ------------------------------------------------------ */

std::string executeCommand(const std::string &command)
{
  std::ostringstream output;
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe)
  {
    throw std::runtime_error("popen() failed.");
  }

  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    output << buffer;
  }

  int status = pclose(pipe);
  if (status != 0)
  {
    throw std::runtime_error("Command execution failed.");
  }

  return output.str().substr(0, output.str().size() - 1);
}

// OpenCV / DNN / Inference
const char *pkg_config_cmd = "pkg-config Yolov8-genom3 --variable=datarootdir";
std::string package_shared_dir = executeCommand(pkg_config_cmd);

std::string model_path = package_shared_dir + "/yolov8-genom3/models/yolov8s.onnx";
std::string classes_path = package_shared_dir + "/yolov8-genom3/classes/classes.txt";
Inference inf(model_path, cv::Size(640, 480), classes_path, true);

/* --- Task detect ------------------------------------------------------ */

/* --- Activity detect_objects ------------------------------------------ */

/** Codel FetchPorts of activity detect_objects.
 *
 * Triggered by Yolov8_start.
 * Yields to Yolov8_pause_start, Yolov8_poll.
 * Throws Yolov8_e_OUT_OF_MEM, Yolov8_e_BAD_IMAGE_PORT,
 *        Yolov8_e_OPENCV_ERROR, Yolov8_e_BAD_CONFIG.
 */
genom_event
FetchPorts(const Yolov8_ImageFrame *ImageFrame,
           const Yolov8_Intrinsics *Intrinsics,
           const Yolov8_Extrinsics *Extrinsics, bool debug,
           const genom_context self)
{
  // Check if all ports are connected and available
  if (!check_port_in_p(ImageFrame))
  {
    CODEL_LOG_WARNING("Image port not connected");
    return Yolov8_pause_start;
  }
  if (!check_port_in_p(Intrinsics))
  {
    CODEL_LOG_WARNING("Intrinsics port not connected");
    return Yolov8_pause_start;
  }
  if (!check_port_in_p(Extrinsics))
  {
    CODEL_LOG_WARNING("Extrinsics port not connected");
    return Yolov8_pause_start;
  }

  if (debug)
  {
    CODEL_LOG_INFO(2, 1, "All ports connected, fetching data");
  }

  return Yolov8_pause_start;
}

/** Codel FetchDataFromPorts of activity detect_objects.
 *
 * Triggered by Yolov8_poll.
 * Yields to Yolov8_pause_poll, Yolov8_main, Yolov8_ether.
 * Throws Yolov8_e_OUT_OF_MEM, Yolov8_e_BAD_IMAGE_PORT,
 *        Yolov8_e_OPENCV_ERROR, Yolov8_e_BAD_CONFIG.
 */
genom_event
FetchDataFromPorts(const Yolov8_ImageFrame *ImageFrame,
                   const Yolov8_Intrinsics *Intrinsics,
                   const Yolov8_Extrinsics *Extrinsics,
                   or_sensor_frame *image_frame,
                   or_sensor_intrinsics *intrinsics,
                   or_sensor_extrinsics *extrinsics, bool debug,
                   const genom_context self)
{
  or_sensor_frame *ImageFrameData;
  or_sensor_intrinsics *IntrinsicsData;
  or_sensor_extrinsics *ExtrinsicsData;

  // Read ports
  if (ImageFrame->read(self) == genom_ok && ImageFrame->data(self))
    ImageFrameData = ImageFrame->data(self);
  else
  {
    Yolov8_e_BAD_IMAGE_PORT_detail msg;
    snprintf(msg.message, sizeof(msg.message), "%s", "Failed to read image port. waiting");
    return Yolov8_pause_poll;
  }
  if (Intrinsics->read(self) == genom_ok && Intrinsics->data(self))
    IntrinsicsData = Intrinsics->data(self);
  else
  {
    Yolov8_e_BAD_IMAGE_PORT_detail msg;
    snprintf(msg.message, sizeof(msg.message), "%s", "Failed to read intrinsics port");
    return Yolov8_pause_poll;
  }
  if (Extrinsics->read(self) == genom_ok && Extrinsics->data(self))
    ExtrinsicsData = Extrinsics->data(self);
  else
  {
    Yolov8_e_BAD_IMAGE_PORT_detail msg;
    snprintf(msg.message, sizeof(msg.message), "%s", "Failed to read extrinsics port");
    return Yolov8_pause_poll;
  }

  // Copy data
  *image_frame = *ImageFrameData;
  *intrinsics = *IntrinsicsData;
  *extrinsics = *ExtrinsicsData;

  if (debug)
  {
    CODEL_LOG_INFO(2, 1, "Fetched new data...");
  }

  return Yolov8_main;
}

/** Codel DetectObjects of activity detect_objects.
 *
 * Triggered by Yolov8_main.
 * Yields to Yolov8_main, Yolov8_poll, Yolov8_ether.
 * Throws Yolov8_e_OUT_OF_MEM, Yolov8_e_BAD_IMAGE_PORT,
 *        Yolov8_e_OPENCV_ERROR, Yolov8_e_BAD_CONFIG.
 */
genom_event
DetectObjects(bool start_detection, const or_sensor_frame *image_frame,
              const or_sensor_intrinsics *intrinsics,
              const or_sensor_extrinsics *extrinsics,
              const sequence_string *classes, bool debug,
              bool show_frames, const genom_context self)
{
  if (!start_detection)
  {
    if (debug)
    {
      CODEL_LOG_WARNING("Detection not started");
    }
    return Yolov8_poll;
  }

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
    }
  }

  // Publish the image
  if (show_frames)
  {
    cv::imshow("Yolov8", image);
    cv::waitKey(1);
  }

  return Yolov8_main;
}
