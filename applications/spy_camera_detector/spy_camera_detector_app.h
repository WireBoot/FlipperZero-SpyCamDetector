#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include "spy_camera_detector_view.h"

#define TAG "SpyCameraDetector"
#define APP_VERSION "v1.1.0"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SpyCameraDetectorView* detector_view;
    NotificationApp* notification;
    FuriTimer* scan_timer;
    
    // Detection state
    bool is_scanning;
    uint32_t signals_detected;
    uint32_t scan_duration;
    
    // Statistics
    uint32_t total_scans;
    uint32_t threats_found;
} SpyCameraDetectorApp;

typedef enum {
    SpyCameraDetectorViewIdMain,
} SpyCameraDetectorViewId;

#ifdef __cplusplus
}
#endif
