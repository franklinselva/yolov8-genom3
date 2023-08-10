
#include "acYolov8.h"

#include "Yolov8_c_types.h"


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
  /* skeleton sample: insert your code */
  /* skeleton sample */ return Yolov8_pause_start;
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
  /* skeleton sample: insert your code */
  /* skeleton sample */ return Yolov8_pause_poll;
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
  /* skeleton sample: insert your code */
  /* skeleton sample */ return Yolov8_main;
}
