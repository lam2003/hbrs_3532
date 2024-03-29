#pragma once

#include "global.h"

namespace rs
{
static void ConfigLogger()
{
    setbuf(stdout, NULL);
    elog_init("/3532.log");

    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_DIR | ELOG_FMT_LINE | ELOG_FMT_TIME | ELOG_FMT_T_INFO | ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_TIME | ELOG_FMT_T_INFO | ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);
    elog_set_text_color_enabled(true);
    elog_start(STRINGIZE_VALUE_OF(RS_VERSION));
}
} // namespace rs