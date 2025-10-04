#include <libremidi/libremidi.hpp>

#include <iostream>

libremidi::input_port find_virtual_port_by_name(libremidi_api api, std::string_view const& name) {
    libremidi::observer obs(
        libremidi::observer_configuration{.track_hardware = false, .track_virtual = true},
        libremidi::observer_configuration_for(api)
    );

    for (auto const& port : obs.get_input_ports()) {
        if (port.port_name == name) {
            return port;
        }
    }

    return {};
}

int main() {
    constexpr std::string_view port_name = "My virtual output port";

    auto virtualOutput = libremidi::midi_out{{}, libremidi::alsa_seq::output_configuration{}};
    if (virtualOutput.open_virtual_port(port_name) != stdx::error{}) {
        std::cerr << "Error opening virtual port" << std::endl;
        return EXIT_FAILURE;
    }

    auto input = libremidi::midi_in{
        {.on_message =
             [](libremidi::message const& message) {
                 std::cout << "Received MIDI message with " << message.size() << " bytes : { ";
                 std::cout << std::hex;
                 for (auto byte : message) {
                     std::cout << "0x" << static_cast<int>(byte) << ", ";
                 }
                 std::cout << "} at timestamp " << std::dec << message.timestamp << std::endl;
             }},
        libremidi::alsa_seq::input_configuration{}
    };
    auto err = input.open_port(
        find_virtual_port_by_name(libremidi::API::ALSA_SEQ, port_name),
        "Virtual output loopback"
    );
    if (err != stdx::error{}) {
        std::cerr << "Error opening input port: " << err.message().data() << std::endl;
        return EXIT_FAILURE;
    } else {
        std::cout << "Successfully connected to virtual output port" << std::endl;
    }

    virtualOutput.send_message({0x90, 60, 127});
    virtualOutput.send_message({0x80, 60, 0});

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return EXIT_SUCCESS;
}