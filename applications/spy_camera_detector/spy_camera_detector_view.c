#include "spy_camera_detector_view.h"
#include <furi_hal.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <m-string.h>

#define SCAN_INTERVAL_MS 100
#define SIGNAL_THRESHOLD -75  // dBm
#define THREAT_FREQUENCIES_COUNT 12

static const uint32_t threat_frequencies[THREAT_FREQUENCIES_COUNT] = {
    2400000000,  // Common 2.4GHz cameras
    2412000000,
    2422000000,
    2437000000,
    2452000000,
    2462000000,
    5180000000,  // Common 5GHz cameras
    5200000000,
    5240000000,
    5745000000,
    240000000,   // Some wireless cameras
    433920000    // Common surveillance frequency
};

static const char* threat_frequency_names[THREAT_FREQUENCIES_COUNT] = {
    "2.4GHz Cam",
    "2.4GHz WiFi",
    "2.4GHz Dev",
    "2.4GHz Cam",
    "2.4GHz Dev",
    "2.4GHz Cam",
    "5GHz Cam",
    "5GHz WiFi",
    "5GHz Cam",
    "5GHz Dev",
    "240MHz Cam",
    "433MHz Dev"
};

static void render_progress_bar(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t progress) {
    canvas_draw_frame(canvas, x, y, width, 6);
    if(progress > 0) {
        uint8_t fill_width = (width - 2) * progress / 100;
        if(fill_width > 0) {
            canvas_draw_box(canvas, x + 1, y + 1, fill_width, 4);
        }
    }
}

static void render_signal_strength(Canvas* canvas, uint8_t x, uint8_t y, uint8_t strength) {
    // strength: 0-4 (0=weak, 4=strong)
    for(uint8_t i = 0; i < 5; i++) {
        if(i < strength) {
            canvas_draw_box(canvas, x + (i * 3), y - (i * 2), 2, (i * 2) + 2);
        } else {
            canvas_draw_frame(canvas, x + (i * 3), y - (i * 2), 2, (i * 2) + 2);
        }
    }
}

static void spy_camera_detector_view_draw_callback(Canvas* canvas, void* model) {
    SpyCameraDetectorViewModel* vm = model;
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    // Title and version
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Spy Camera Detector");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 12, AlignCenter, AlignTop, APP_VERSION);
    
    // Button panel
    elements_button_left(canvas, "Reset");
    elements_button_right(canvas, vm->is_scanning ? "Stop" : "Scan");
    
    canvas_set_font(canvas, FontSecondary);
    
    if(vm->is_scanning) {
        // Scanning UI
        canvas_draw_str(canvas, 5, 28, "Status: SCANNING");
        canvas_draw_str(canvas, 5, 38, vm->status_message);
        
        if(vm->signal_detected) {
            // Threat detected UI
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 5, 50, "THREAT DETECTED!");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 5, 60, "Check area carefully");
            
            // Threat level indicator
            render_progress_bar(canvas, 5, 65, 118, vm->threat_level * 25);
            
            char threat_str[32];
            snprintf(threat_str, sizeof(threat_str), "Threat Level: %d/4", vm->threat_level);
            canvas_draw_str_aligned(canvas, 64, 75, AlignCenter, AlignBottom, threat_str);
            
            // Signal strength indicator
            render_signal_strength(canvas, 100, 45, vm->threat_level);
        } else {
            // Normal scanning UI
            canvas_draw_str(canvas, 5, 50, "Scanning for signals...");
            
            // Animated scanning indicator
            uint8_t dot_count = (vm->scan_duration / 2) % 4;
            char dots[5] = {0};
            for(uint8_t i = 0; i < 4; i++) {
                dots[i] = (i < dot_count) ? '.' : ' ';
            }
            canvas_draw_str(canvas, 5, 60, dots);
            
            render_progress_bar(canvas, 5, 65, 118, (vm->scan_duration % 100));
        }
    } else {
        // Ready/Stopped UI
        canvas_draw_str(canvas, 5, 28, "Status: READY");
        canvas_draw_str(canvas, 5, 38, "Press Scan to start");
        canvas_draw_str(canvas, 5, 48, "detecting spy cameras");
        
        if(vm->signals_found > 0) {
            canvas_draw_str(canvas, 5, 60, "Last scan found:");
            char found_str[32];
            snprintf(found_str, sizeof(found_str), "%lu potential threats", vm->signals_found);
            canvas_draw_str(canvas, 5, 70, found_str);
        }
    }
    
    // Statistics footer
    char stats_str[64];
    snprintf(stats_str, sizeof(stats_str), "Signals: %lu | Time: %lus", 
             vm->signals_found, vm->scan_duration);
    canvas_draw_str_aligned(canvas, 64, 126, AlignCenter, AlignBottom, stats_str);
}

static bool spy_camera_detector_view_input_callback(InputEvent* event, void* context) {
    SpyCameraDetectorView* detector_view = context;
    bool consumed = false;
    
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyOk:
            consumed = true;
            if(detector_view->callback) {
                detector_view->callback(detector_view->context);
            }
            break;
        case InputKeyLeft:
            // Reset stats
            consumed = true;
            spy_camera_detector_view_reset_stats(detector_view);
            break;
        case InputKeyRight:
            // Start/Stop scan
            consumed = true;
            if(detector_view->callback) {
                detector_view->callback(detector_view->context);
            }
            break;
        default:
            break;
        }
    }
    
    return consumed;
}

static bool spy_camera_detector_signal_analysis(uint32_t frequency, int8_t rssi) {
    // Check if frequency is in threat list
    for(uint8_t i = 0; i < THREAT_FREQUENCIES_COUNT; i++) {
        if(frequency == threat_frequencies[i]) {
            return true;
        }
    }
    
    // Check signal strength (stronger signals are more suspicious)
    if(rssi > SIGNAL_THRESHOLD) {
        return true;
    }
    
    return false;
}

static uint8_t calculate_threat_level(uint32_t frequency, int8_t rssi) {
    uint8_t threat_level = 1;
    
    // Base threat level on frequency match
    for(uint8_t i = 0; i < THREAT_FREQUENCIES_COUNT; i++) {
        if(frequency == threat_frequencies[i]) {
            threat_level += 2; // Known camera frequency
            break;
        }
    }
    
    // Adjust based on signal strength
    if(rssi > -60) threat_level += 2;
    else if(rssi > -70) threat_level += 1;
    
    return (threat_level > 4) ? 4 : threat_level;
}

SpyCameraDetectorView* spy_camera_detector_view_alloc() {
    SpyCameraDetectorView* detector_view = malloc(sizeof(SpyCameraDetectorView));
    
    detector_view->view = view_alloc();
    view_set_context(detector_view->view, detector_view);
    view_set_draw_callback(detector_view->view, spy_camera_detector_view_draw_callback);
    view_set_input_callback(detector_view->view, spy_camera_detector_view_input_callback);
    
    view_allocate_model(detector_view->view, ViewModelTypeLocking, sizeof(SpyCameraDetectorViewModel));
    
    SpyCameraDetectorViewModel* vm = view_get_model(detector_view->view);
    vm->is_scanning = false;
    vm->signal_detected = false;
    vm->signals_found = 0;
    vm->scan_duration = 0;
    vm->current_frequency = 0;
    vm->threat_level = 0;
    vm->false_positives = 0;
    strcpy(vm->status_message, "Ready to scan");
    view_commit_model(detector_view->view, false);
    
    return detector_view;
}

void spy_camera_detector_view_free(SpyCameraDetectorView* detector_view) {
    furi_assert(detector_view);
    view_free(detector_view->view);
    free(detector_view);
}

View* spy_camera_detector_get_view(SpyCameraDetectorView* detector_view) {
    furi_assert(detector_view);
    return detector_view->view;
}

void spy_camera_detector_view_set_callback(
    SpyCameraDetectorView* detector_view, 
    SpyCameraDetectorViewCallback callback, 
    void* context) {
    furi_assert(detector_view);
    detector_view->callback = callback;
    detector_view->context = context;
}

void spy_camera_detector_view_start_scan(SpyCameraDetectorView* detector_view) {
    furi_assert(detector_view);
    
    SpyCameraDetectorViewModel* vm = view_get_model(detector_view->view);
    
    if(!vm->is_scanning) {
        vm->is_scanning = true;
        vm->scan_duration = 0;
        vm->signal_detected = false;
        strcpy(vm->status_message, "Initializing scanner...");
        
        view_commit_model(detector_view->view, true);
        
        dolphin_deed(DolphinDeedPluginStart);
    }
    
    view_unlock_model(detector_view->view);
}

void spy_camera_detector_view_stop_scan(SpyCameraDetectorView* detector_view) {
    furi_assert(detector_view);
    
    SpyCameraDetectorViewModel* vm = view_get_model(detector_view->view);
    
    if(vm->is_scanning) {
        vm->is_scanning = false;
        strcpy(vm->status_message, "Scanning stopped");
        view_commit_model(detector_view->view, true);
    }
    
    view_unlock_model(detector_view->view);
}

void spy_camera_detector_view_reset_stats(SpyCameraDetectorView* detector_view) {
    furi_assert(detector_view);
    
    SpyCameraDetectorViewModel* vm = view_get_model(detector_view->view);
    
    vm->signals_found = 0;
    vm->scan_duration = 0;
    vm->signal_detected = false;
    vm->threat_level = 0;
    vm->false_positives = 0;
    strcpy(vm->status_message, "Statistics reset");
    
    view_commit_model(detector_view->view, true);
    view_unlock_model(detector_view->view);
}

void spy_camera_detector_view_update_scan(SpyCameraDetectorView* detector_view) {
    furi_assert(detector_view);
    
    SpyCameraDetectorViewModel* vm = view_get_model(detector_view->view);
    
    if(vm->is_scanning) {
        vm->scan_duration++;
        
        // Simulate signal detection (in real implementation, this would use SubGhz)
        // Every 3 seconds, simulate a detection
        if((vm->scan_duration % 30) == 15) {
            // Simulate signal detection
            int8_t rssi = -90 + (furi_hal_random_get() % 40); // Random RSSI between -90 and -50
            uint32_t freq_index = furi_hal_random_get() % THREAT_FREQUENCIES_COUNT;
            uint32_t frequency = threat_frequencies[freq_index];
            
            if(spy_camera_detector_signal_analysis(frequency, rssi)) {
                vm->signal_detected = true;
                vm->signals_found++;
                vm->current_frequency = frequency;
                vm->threat_level = calculate_threat_level(frequency, rssi);
                
                snprintf(vm->status_message, sizeof(vm->status_message),
                        "%s - %ld MHz", threat_frequency_names[freq_index], frequency / 1000000);
                
                // Trigger notification
                NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
                if(vm->threat_level >= 3) {
                    // High threat - more aggressive notification
                    notification_message(notification, &sequence_set_vibro_on);
                    notification_message(notification, &sequence_blink_red_100);
                    notification_message(notification, &sequence_sound_alert);
                } else {
                    // Low threat - subtle notification
                    notification_message(notification, &sequence_single_vibro);
                    notification_message(notification, &sequence_blink_blue_100);
                }
                furi_record_close(RECORD_NOTIFICATION);
            }
        } else if((vm->scan_duration % 30) == 25) {
            // Clear detection after some time
            vm->signal_detected = false;
            snprintf(vm->status_message, sizeof(vm->status_message), "Scanning... No threats");
        } else if(vm->scan_duration % 10 == 0) {
            // Update scanning status
            snprintf(vm->status_message, sizeof(vm->status_message), 
                    "Scanning... %lus", vm->scan_duration);
        }
        
        view_commit_model(detector_view->view, true);
    }
    
    view_unlock_model(detector_view->view);
}
