#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <gui/view.h>
#include <gui/elements.h>
#include <furi_hal_subghz.h>
#include <dolphin/dolphin.h>

typedef struct SpyCameraDetectorView SpyCameraDetectorView;

typedef enum {
    SignalTypeUnknown,
    SignalType2_4GHz,
    SignalType5GHz,
    SignalTypeBluetooth,
    SignalTypeCustom
} SignalType;

typedef struct {
    uint32_t frequency;
    SignalType type;
    int8_t signal_strength;
    char name[32];
    bool is_threat;
    uint32_t timestamp;
} DetectedSignal;

typedef void (*SpyCameraDetectorViewCallback)(void* context);

struct SpyCameraDetectorView {
    View* view;
    SpyCameraDetectorViewCallback callback;
    void* context;
};

typedef struct {
    bool is_scanning;
    bool signal_detected;
    uint32_t signals_found;
    uint32_t scan_duration;
    uint32_t current_frequency;
    DetectedSignal last_detected_signal;
    uint8_t threat_level;
    char status_message[64];
    uint32_t false_positives;
} SpyCameraDetectorViewModel;

// Public API
SpyCameraDetectorView* spy_camera_detector_view_alloc();
void spy_camera_detector_view_free(SpyCameraDetectorView* detector_view);
View* spy_camera_detector_get_view(SpyCameraDetectorView* detector_view);
void spy_camera_detector_view_set_callback(
    SpyCameraDetectorView* detector_view, 
    SpyCameraDetectorViewCallback callback, 
    void* context);

void spy_camera_detector_view_start_scan(SpyCameraDetectorView* detector_view);
void spy_camera_detector_view_stop_scan(SpyCameraDetectorView* detector_view);
void spy_camera_detector_view_reset_stats(SpyCameraDetectorView* detector_view);
void spy_camera_detector_view_update_scan(SpyCameraDetectorView* detector_view);

#ifdef __cplusplus
}
#endif
