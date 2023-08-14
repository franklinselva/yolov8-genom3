
#include <cstdlib>
#include "fg3utils/trace_f.h"
#include "fg3utils/macros.h"
#include <string>

#include "utils.h"

#include "acYolov8.h"

#include "Yolov8_c_types.h"

/* --- Activity start_object_detection ---------------------------------- */

/** Validation codel check_resource_path of activity start_object_detection.
 *
 * Returns genom_ok.
 * Throws Yolov8_e_BAD_IMAGE_PORT, Yolov8_e_OPENCV_ERROR,
 * Yolov8_e_BAD_CONFIG, Yolov8_e_OUT_OF_MEM.
 */
genom_event
check_resource_path(char **resource_path, const genom_context self)
{
  const char *pkg_config_cmd = "pkg-config Yolov8-genom3 --variable=datarootdir";
  std::string package_shared_dir = executeCommand(pkg_config_cmd);

  if (package_shared_dir.empty())
  {
    Yolov8_e_BAD_CONFIG_detail *d;
    snprintf(d->message, sizeof(d->message),
             "pkg-config Yolov8-genom3 --variable=datarootdir failed. Please check if the package is installed correctly.");
    CODEL_LOG_WARNING("Yolov8_e_BAD_CONFIG: %s", d->message);
    return Yolov8_e_BAD_CONFIG(d, self);
  }

  package_shared_dir = package_shared_dir + "/yolov8-genom3";

  *resource_path = const_cast<char *>(package_shared_dir.c_str());

  return genom_ok;
}
