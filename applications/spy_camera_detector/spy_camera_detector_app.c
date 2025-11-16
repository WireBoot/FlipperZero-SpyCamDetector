#include "spy_camera_detector_app.h"
#include "icons.h"

static void scan_timer_callback(void* context) {
    SpyCameraDetectorApp* app = context;
    if(app->is_scanning) {
        spy_camera_detector_view_update_scan(app->detector_view);
    }
}

static SpyCameraDetectorApp* spy_camera_detector_app_alloc() {
    SpyCameraDetectorApp* app = malloc(sizeof(SpyCameraDetectorApp));
    
    // Initialize core components
    app->gui = furi_record_open(RECORD_GUI);
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    
    // Initialize view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    
    // Initialize main view
    app->detector_view = spy_camera_detector_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, 
        SpyCameraDetectorViewIdMain, 
        spy_camera_detector_get_view(app->detector_view));
    
    // Initialize scan timer (100ms interval)
    app->scan_timer = furi_timer_alloc(scan_timer_callback, FuriTimerTypePeriodic, app);
    
    // Set initial state
    app->is_scanning = false;
    app->signals_detected = 0;
    app->scan_duration = 0;
    app->total_scans = 0;
    app->threats_found = 0;
    
    FURI_LOG_I(TAG, "Application initialized successfully");
    
    return app;
}

static void spy_camera_detector_app_free(SpyCameraDetectorApp* app) {
    furi_assert(app);
    
    // Stop timer
    if(app->scan_timer) {
        furi_timer_stop(app->scan_timer);
        furi_timer_free(app->scan_timer);
    }
    
    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, SpyCameraDetectorViewIdMain);
    spy_camera_detector_view_free(app->detector_view);
    
    // Free dispatcher
    view_dispatcher_free(app->view_dispatcher);
    
    // Close records
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    
    free(app);
    
    FURI_LOG_I(TAG, "Application freed");
}

static void spy_camera_detector_toggle_scan(SpyCameraDetectorApp* app) {
    if(app->is_scanning) {
        // Stop scanning
        app->is_scanning = false;
        furi_timer_stop(app->scan_timer);
        spy_camera_detector_view_stop_scan(app->detector_view);
        notification_message(app->notification, &sequence_blink_stop);
        FURI_LOG_I(TAG, "Scanning stopped");
    } else {
        // Start scanning
        app->is_scanning = true;
        app->total_scans++;
        furi_timer_start(app->scan_timer, furi_kernel_get_tick_frequency() / 10); // 100ms
        spy_camera_detector_view_start_scan(app->detector_view);
        FURI_LOG_I(TAG, "Scanning started");
    }
}

static bool spy_camera_detector_navigation_event_callback(void* context) {
    SpyCameraDetectorApp* app = context;
    
    // Stop scanning when exiting
    if(app->is_scanning) {
        spy_camera_detector_toggle_scan(app);
    }
    
    return false;
}

static void spy_camera_detector_view_callback(void* context) {
    SpyCameraDetectorApp* app = context;
    spy_camera_detector_toggle_scan(app);
}

int32_t spy_camera_detector_app(void* p) {
    UNUSED(p);
    
    SpyCameraDetectorApp* app = spy_camera_detector_app_alloc();
    
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate application");
        return -1;
    }
    
    // Set view callbacks
    spy_camera_detector_view_set_callback(
        app->detector_view, 
        spy_camera_detector_view_callback, 
        app);
    
    // Set initial view
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, 
        spy_camera_detector_navigation_event_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, SpyCameraDetectorViewIdMain);
    
    FURI_LOG_I(TAG, "Starting application main loop");
    
    // Run main loop
    view_dispatcher_run(app->view_dispatcher);
    
    FURI_LOG_I(TAG, "Application exiting");
    
    // Cleanup
    spy_camera_detector_app_free(app);
    
    return 0;
}
