
#include "acYolov8.h"

#include "Yolov8_c_types.h"


/* --- Function set_classes --------------------------------------------- */

/** Codel SetClasses of function set_classes.
 *
 * Returns genom_ok.
 * Throws Yolov8_e_BAD_CONFIG.
 */
genom_event
SetClasses(const sequence_string *class_names,
           sequence_string *classes, const genom_context self)
{
  /* skeleton sample: insert your code */
  /* skeleton sample */ return genom_ok;
}


/* --- Function set_debug ----------------------------------------------- */

/** Codel SetDebug of function set_debug.
 *
 * Returns genom_ok.
 */
genom_event
SetDebug(bool is_debug_mode, bool *debug, const genom_context self)
{
  /* skeleton sample: insert your code */
  /* skeleton sample */ return genom_ok;
}


/* --- Function show_image_frames --------------------------------------- */

/** Codel ShowFrames of function show_image_frames.
 *
 * Returns genom_ok.
 */
genom_event
ShowFrames(bool show_cv_frames, bool *show_frames,
           const genom_context self)
{
  /* skeleton sample: insert your code */
  /* skeleton sample */ return genom_ok;
}


/* --- Function set_verbose_level --------------------------------------- */

/** Codel SetVerboseLevel of function set_verbose_level.
 *
 * Returns genom_ok.
 */
genom_event
SetVerboseLevel(uint8_t verbose_level, uint8_t *v_level,
                const genom_context self)
{
  /* skeleton sample: insert your code */
  /* skeleton sample */ return genom_ok;
}


/* --- Function start_object_detection ---------------------------------- */

/** Codel SetStartDetection of function start_object_detection.
 *
 * Returns genom_ok.
 */
genom_event
SetStartDetection(bool *start_detection, const genom_context self)
{
  /* skeleton sample: insert your code */
  /* skeleton sample */ return genom_ok;
}


/* --- Function stop_object_detection ----------------------------------- */

/** Codel SetStopDetection of function stop_object_detection.
 *
 * Returns genom_ok.
 */
genom_event
SetStopDetection(bool *start_detection, const genom_context self)
{
  /* skeleton sample: insert your code */
  /* skeleton sample */ return genom_ok;
}
