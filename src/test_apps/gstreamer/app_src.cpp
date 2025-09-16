
#include "logger/logger.h"

#include <gst/app/gstappsrc.h>
#include <gst/audio/audio-info.h>
#include <gst/gst.h>

#include <cmath>
#include <iostream>
#include <vector>

using logger::log;

constexpr int SAMPLE_RATE = 48'000;
constexpr int CHANNELS = 1;
constexpr double TWO_PI = 6.283185307179586;

struct SineGenerator
{
    double phase = 0.0;
    double freq;
    double amplitude;

    SineGenerator(double f, double a)
    : freq(f)
    , amplitude(a) {}

    void fillBuffer(gint16* data, int n_samples) {
        double phase_step = TWO_PI * freq / SAMPLE_RATE;
        for (int i = 0; i < n_samples; i++) {
            data[i] = static_cast<gint16>(amplitude * std::sin(phase));
            phase += phase_step;
            if (phase > TWO_PI) phase -= TWO_PI;
        }
    }
};

static void need_data(GstElement* appsrc, guint, gpointer user_data) {
    log("need_data called at {}", appsrc->base_time);

    auto* gen = static_cast<SineGenerator*>(user_data);

    // Number of samples to generate per buffer
    int n_samples = 1'024;
    int buffer_size = n_samples * sizeof(gint16) * CHANNELS;

    GstBuffer* buffer = gst_buffer_new_and_alloc(buffer_size);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);

    log("buffer mapped");

    // Fill with sine wave
    gen->fillBuffer(reinterpret_cast<gint16*>(map.data), n_samples);

    log("buffer filled");

    gst_buffer_unmap(buffer, &map);

    log("buffer unmapped");

    static guint64 total_samples = 0;

    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(total_samples, GST_SECOND, SAMPLE_RATE);
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(n_samples, GST_SECOND, SAMPLE_RATE);

    total_samples += n_samples;

    log("pushing buffer");
    gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);
    log("buffer pushed");
}

int main(int argc, char* argv[]) {
    log("Starting application");

    gst_init(&argc, &argv);

    log("GStreamer initialized");

    // Frequency & amplitude
    double freq = 440.0;         // Hz
    double amplitude = 15000.0;  // out of 32767 (16-bit)

    SineGenerator gen(freq, amplitude);

    // Create pipeline: appsrc -> audioconvert -> audioresample -> autoaudiosink
    GstElement* pipeline = gst_pipeline_new("pipeline");
    GstElement* appsrc = gst_element_factory_make("appsrc", "source");
    GstElement* convert = gst_element_factory_make("audioconvert", "convert");
    GstElement* resample = gst_element_factory_make("audioresample", "resample");
    GstElement* sink = gst_element_factory_make("autoaudiosink", "sink");

    if (!pipeline || !appsrc || !convert || !resample || !sink) {
        log("Failed to create GStreamer elements");
        return -1;
    }

    // std::vector<GstAudioChannelPosition> chanPositions;
    // int numChannels = CHANNELS;
    // if (numChannels == 2) {
    //     chanPositions = {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
    //     GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT};
    // } else {
    //     chanPositions = {GST_AUDIO_CHANNEL_POSITION_MONO};
    // }

    // auto* audioInfo = gst_audio_info_new();
    // gst_audio_info_set_format(audioInfo, GST_AUDIO_FORMAT_S16, SAMPLE_RATE, numChannels,
    // chanPositions.data());

    // auto* caps = gst_audio_info_to_caps(audioInfo);
    // gst_audio_info_free(audioInfo);

    // g_object_set(G_OBJECT(appsrc), "caps", caps, nullptr);

    // Set appsrc caps (raw PCM, 16-bit signed, mono)
    GstCaps* caps = gst_caps_new_simple(
        "audio/x-raw",
        "format",
        G_TYPE_STRING,
        "S16LE",
        "rate",
        G_TYPE_INT,
        SAMPLE_RATE,
        "channels",
        G_TYPE_INT,
        CHANNELS,
        nullptr
    );
    // g_object_set(G_OBJECT(appsrc), "caps", caps, nullptr);

    gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
    gst_caps_unref(caps);

    // Configure appsrc
    gst_app_src_set_stream_type(GST_APP_SRC(appsrc), GST_APP_STREAM_TYPE_STREAM);

    g_object_set(
        G_OBJECT(appsrc),
        "stream-type",
        GST_APP_STREAM_TYPE_STREAM,
        "format",
        GST_FORMAT_TIME,
        "is-live",
        TRUE,
        nullptr
    );
    // g_object_set(
    //     G_OBJECT(appsrc),
    //     "caps",
    //     caps,
    //     "stream-type",
    //     GST_APP_STREAM_TYPE_STREAM,
    //     "format",
    //     GST_FORMAT_TIME,
    //     "is-live",
    //     TRUE,
    //     nullptr
    // );

    g_signal_connect(appsrc, "need-data", G_CALLBACK(need_data), &gen);

    gst_bin_add_many(GST_BIN(pipeline), appsrc, convert, resample, sink, nullptr);
    gst_element_link_many(appsrc, convert, resample, sink, nullptr);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Run main loop
    log("Running main loop");
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // Cleanup
    log("Stopping pipeline");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    log("Application exiting");
    return 0;
}
