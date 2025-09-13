#include <iostream>
#include <gst/gst.h>

int main(int argc, char** argv) {

    gst_init(&argc, &argv);

    GError *error = nullptr;

    // Equivalent to: gst-launch-1.0 audiotestsrc wave=sine freq=440 ! autoaudiosink
    GstElement *pipeline = gst_parse_launch(
        "audiotestsrc wave=sine freq=440 ! autoaudiosink",
        &error
    );

    if (!pipeline) {
        std::cerr << "Failed to create pipeline: "
                  << (error ? error->message : "unknown error") << std::endl;
        if (error) g_error_free(error);
        return -1;
    }

    // Set pipeline to PLAYING
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Run main loop
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // Clean up
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;


    return EXIT_SUCCESS;
}