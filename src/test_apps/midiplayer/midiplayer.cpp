#include "logger/logger.h"

#include <gst/app/gstappsrc.h>
#include <gst/audio/audio-info.h>
#include <gst/gst.h>

#include <libremidi/libremidi.hpp>

#include <cmath>
#include <iostream>
#include <mutex>
#include <vector>

using logger::log;

namespace
{
constexpr int SAMPLE_RATE = 48'000;
constexpr int CHANNELS = 1;
constexpr double TWO_PI = 6.283185307179586;
constexpr double amplitude = 0.3;  // range [0.0, 1.0]

enum class Waveform
{
    SINE,
    SAW,
    SQUARE
};
Waveform waveform = Waveform::SQUARE;

std::mutex mutex;
int previousNote = -1;
double nextFreq = 0.0;

auto midi_callback = [](const libremidi::message& message) {
    std::unique_lock lock(mutex);
    if (message[0] == 0x90 && message[2] != 0) {  // Note on
        previousNote = message[1];
        nextFreq = 440.0 * std::pow(2.0, (previousNote - 69) / 12.0);
        logger::log("Note on: {}, freq: {}", previousNote, nextFreq);
    } else if (message[0] == 0x80 || (message[0] == 0x90 && message[2] == 0)) {  // Note off
        int note = message[1];
        if (note != previousNote) {
            return;  // ignore, we only handle one note at a time
        }
        previousNote = -1;
        nextFreq = 0;
        logger::log("Note off: {}", note);
    }
};

[[maybe_unused]]
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

[[maybe_unused]]
void gen_saw_wave(float* buffer, int numSamples, double phase, double frequency, double amplitude) {
    double phaseStep = TWO_PI * frequency / SAMPLE_RATE;
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = static_cast<float>(amplitude * (2.0 * (phase / TWO_PI) - 1.0));
        phase += phaseStep;
        if (phase > TWO_PI) {
            phase -= TWO_PI;
        }
    }
}

[[maybe_unused]]
void gen_square_wave(
    float* buffer,
    int numSamples,
    double phase,
    double frequency,
    double amplitude
) {
    double phaseStep = TWO_PI * frequency / SAMPLE_RATE;
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = static_cast<float>(amplitude * (phase < M_PI ? 1.0 : -1.0));
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

    double freq = nextFreq;
    if (0 == freq) {
        // silence
        std::memset(map.data, 0, bufSize);
    } else {
        static double phase = 0.0;
        phase += (TWO_PI * freq * numSamples) / SAMPLE_RATE;
        phase = std::fmod(phase, TWO_PI);

        if (CHANNELS == 1) {
            switch (waveform) {
                case Waveform::SINE:
                    gen_sine_wave(
                        reinterpret_cast<float*>(map.data),
                        numSamples,
                        phase,
                        freq,
                        amplitude
                    );
                    break;
                case Waveform::SAW:
                    gen_saw_wave(
                        reinterpret_cast<float*>(map.data),
                        numSamples,
                        phase,
                        freq,
                        amplitude
                    );
                    break;
                case Waveform::SQUARE:
                    gen_square_wave(
                        reinterpret_cast<float*>(map.data),
                        numSamples,
                        phase,
                        freq,
                        amplitude
                    );
                    break;
            }
        } else {
            // Stereo, write interleaved data.
        }
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
    if (argc > 1) {
        std::string_view waveArg = argv[1];
        if (waveArg == "sine") {
            waveform = Waveform::SINE;
        } else if (waveArg == "saw") {
            waveform = Waveform::SAW;
        } else if (waveArg == "square") {
            waveform = Waveform::SQUARE;
        } else {
            logger::log("Unknown waveform: {}, using square", waveArg);
            waveform = Waveform::SQUARE;
        }
    }

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

    GstElement* alsasink = gst_element_factory_make("alsasink", "audiosink");
    if (!alsasink) {
        log("GStreamer: 'alsasink' could not be created.\n");
        return EXIT_FAILURE;
    }

    g_object_set(
        G_OBJECT(alsasink),
        "device",
        "plughw:0,3",
        // "plughw:TraktorKontrolS,0",
        "buffer-time",
        24'000,  // 24 ms
        "period-time",
        3'000,  // 3 ms
        nullptr
    );

    GstElement* volume = gst_element_factory_make("volume", "volume");
    if (!volume) {
        log("GStreamer: 'volume' element could not be created.\n");
    }

    // Set the volume (0.0 = silent, 1.0 = full scale)
    g_object_set(G_OBJECT(volume), "volume", 0.01, nullptr);  // 20% volume

    // Create the empty pipeline
    GstElement* pipeline = gst_pipeline_new("test-pipeline");
    if (!pipeline) {
        log("GStreamer: 'pipeline' could not be created.\n");
        return EXIT_FAILURE;
    }

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), appsrc, volume, alsasink, nullptr);
    if (gst_element_link_many(appsrc, volume, alsasink, nullptr) == FALSE) {
        log("GStreamer: Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return EXIT_FAILURE;
    }

    log("gst pipeline built!");

    log("Setting up MIDI input");

    libremidi::input_port input_port;

    bool removed = false;
    std::string_view port_name;
    libremidi::container_identifier id;

    std::vector<libremidi::observer> observers;
    for (auto api : libremidi::available_apis()) {
        if (api != libremidi::API::ALSA_SEQ) continue;
        std::string_view api_name = libremidi::get_api_display_name(api);

        logger::log("Displaying ports for: {}", api_name);
        libremidi::observer_configuration cbs;
        cbs.input_added = [=](const libremidi::input_port& p) {
            logger::log("{} : input added {}", api_name, p.display_name);
        };
        cbs.input_removed = [&](const libremidi::input_port& p) {
            if (id == p.container) {
                logger::log("The CASIO port was removed by ID");
                removed = true;
            }
            if (port_name == p.port_name) {
                logger::log("The CASIO port was removed by name");
                removed = true;
            }
        };
        cbs.output_added = [=](const libremidi::output_port& p) {
            logger::log("{} : output added {}", api_name, p.display_name);
        };
        cbs.output_removed = [=](const libremidi::output_port& p) {
            logger::log("{} : output removed {}", api_name, p.display_name);
        };
        observers.emplace_back(cbs, libremidi::observer_configuration_for(api));
    }

    for (auto const& obs : observers) {
        if (obs.get_current_api() != libremidi::API::ALSA_SEQ) continue;
        for (auto const& port : obs.get_input_ports()) {
            if (std::string::npos != port.port_name.find("CASIO")) {
                logger::log("Using port {}", port.port_name);
                input_port = port;
                id = input_port.container;
                port_name = input_port.port_name;
                break;
            }
        }
        if (!input_port.port_name.empty()) break;
    }

    if (input_port.port_name.empty()) {
        std::cerr << "Could not find the CASIO port\n";
        return EXIT_FAILURE;
    }

    // Create the midi object
    libremidi::midi_in midi{libremidi::input_configuration{.on_message = midi_callback}};

    // Open a given midi port.
    midi.open_port(input_port);

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
    gst_object_unref(alsasink);
    gst_object_unref(volume);
    gst_object_unref(appsrc);
    g_main_loop_unref(loop);

    log("Application exiting");
    return EXIT_SUCCESS;
}
