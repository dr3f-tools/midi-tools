
#include "logger/logger.h"

#include <gst/app/gstappsrc.h>
#include <gst/audio/audio-info.h>
#include <gst/gst.h>

#include <cmath>
#include <iostream>
#include <vector>

using logger::log;

namespace
{
constexpr int SAMPLE_RATE = 48'000;
constexpr int CHANNELS = 1;
constexpr double TWO_PI = 6.283185307179586;
constexpr double freq = 400.;
constexpr double amplitude = 0.3;  // range [0.0, 1.0]

void gen_sine_wave(
    float* buffer,
    int numSamples,
    double phase,
    double frequency,
    double amplitude
) {
    double phaseStep = TWO_PI * frequency / SAMPLE_RATE;
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = static_cast<float>(amplitude * std::sin(phase));
        phase += phaseStep;
        if (phase > TWO_PI) {
            phase -= TWO_PI;
        }
    }
}

static void need_data(GstElement* appsrc, guint, gpointer) {
    constexpr int numSamples = 480;
    gsize bufSize = numSamples * sizeof(float) * CHANNELS;

    auto gstBuffer = gst_buffer_new_allocate(nullptr, bufSize, nullptr);
    if (!gstBuffer) {
        log("Failed to allocate gstreamer buffer of size={}", bufSize);
        return;
    }

    GstMapInfo map;
    gst_buffer_map(gstBuffer, &map, GST_MAP_WRITE);

    static double phase = 0.0;
    phase += (TWO_PI * freq * numSamples) / SAMPLE_RATE;
    phase = std::fmod(phase, TWO_PI);

    if (CHANNELS == 1) {
        gen_sine_wave(reinterpret_cast<float*>(map.data), numSamples, phase, freq, amplitude);
    } else {
        logger::log("Stereo not implemented");
    }

    static int samples_pushed = 0;
    GST_BUFFER_PTS(gstBuffer) = gst_util_uint64_scale(samples_pushed, GST_SECOND, SAMPLE_RATE);
    GST_BUFFER_DURATION(gstBuffer) = gst_util_uint64_scale(numSamples, GST_SECOND, SAMPLE_RATE);
    samples_pushed += numSamples;

    gst_buffer_unmap(gstBuffer, &map);
    GstFlowReturn ret{GST_FLOW_ERROR};
    g_signal_emit_by_name(appsrc, "push-buffer", gstBuffer, &ret);
    if (ret != GST_FLOW_OK) {
        log("error pushing buffer to appsrc");
    }
    gst_buffer_unref(gstBuffer);
}
}  // namespace

int main(int argc, char* argv[]) {
    log("Starting application");

    // set up the gstreamer
    gst_init(&argc, &argv);

    // Create the elements
    GstElement* appsrc = gst_element_factory_make("appsrc", "source");
    if (!appsrc) {
        log("GStreamer: 'appsrc' could not be created.\n");
    }

    {
        auto* audioInfo = gst_audio_info_new();
        std::vector<GstAudioChannelPosition> chanPositions;
        if (CHANNELS == 2) {
            chanPositions = {
                GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
            };
        } else {
            chanPositions = {GST_AUDIO_CHANNEL_POSITION_MONO};
        }
        gst_audio_info_set_format(
            audioInfo,
            GST_AUDIO_FORMAT_F32,
            SAMPLE_RATE,
            CHANNELS,
            chanPositions.data()
        );

        {
            auto* caps = gst_audio_info_to_caps(audioInfo);
            gst_audio_info_free(audioInfo);

            g_object_set(G_OBJECT(appsrc), "caps", caps, nullptr);
            gst_caps_unref(caps);
        }
    }

    g_signal_connect(appsrc, "need-data", G_CALLBACK(need_data), nullptr);
    g_object_set(
        G_OBJECT(appsrc),
        "format",
        GST_FORMAT_TIME,  // interpret PTS/DURATION in nanoseconds
        "block",
        TRUE,  // optional: apply backpressure
        nullptr
    );
    log("GStreamer 'appsrc' created!");

    GstElement* autoaudiosink = gst_element_factory_make("autoaudiosink", "audiosink");
    if (!autoaudiosink) {
        log("GStreamer: 'autoaudiosink' could not be created.\n");
    }
    log("GStreamer 'autoaudiosink' created!");

    // Create the empty pipeline
    GstElement* pipeline = gst_pipeline_new("test-pipeline");
    if (!pipeline) {
        log("GStreamer: 'pipeline' could not be created.\n");
        return EXIT_FAILURE;
    }
    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), appsrc, autoaudiosink, nullptr);
    if (gst_element_link_many(appsrc, autoaudiosink, nullptr) == FALSE) {
        log("GStreamer: Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return EXIT_FAILURE;
    }
    log("gst pipeline built!");

    // Start playing
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Run main loop
    log("Running main loop");
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // Cleanup
    log("Stopping pipeline");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(autoaudiosink);
    gst_object_unref(appsrc);
    g_main_loop_unref(loop);

    log("Application exiting");
    return EXIT_SUCCESS;
}
