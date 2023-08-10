
#include "acYolov8.h"
#include "fg3utils/trace_f.h"
#include "fg3utils/macros.h"
#include <cstdio>

#include "Yolov8_c_types.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include "detection/inference.h"

// Inference inf("/home/franklinselva/dev/work/drone-experiment/yolov8-onnx-cpp/source/models/yolov8s.onnx", cv::Size(640, 480),
//               "/home/franklinselva/dev/work/drone-experiment/yolov8-onnx-cpp/source/classes/classes.txt", true);

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
  double image_x = 0.0, image_y = 0.0;
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

  // auto detections = inf.runInference(image);

  return Yolov8_main;
}
