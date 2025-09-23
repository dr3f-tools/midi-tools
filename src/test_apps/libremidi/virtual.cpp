#include <libremidi/../../examples/utils.hpp>
#include <libremidi/libremidi.hpp>

#include <iostream>

// Set the configuration of our MIDI port
// Note that the callback will be invoked from a separate thread,
// it is up to you to protect your data structures afterwards.
// For instance if you are using a GUI toolkit, don't do GUI actions
// in that callback !
auto my_callback = [](const libremidi::message& message) {
    std::cout << "Received MIDI message with " << message.size() << " bytes : { ";
    std::cout << std::hex;
    for (auto byte : message) {
        std::cout << " 0x" << static_cast<int>(byte) << ", ";
    }
    std::cout << " } at timestamp " << std::dec << message.timestamp << "\n";
};

void list_api_port(libremidi_api api) {
    libremidi::observer obs(
        libremidi::observer_configuration{},
        libremidi::observer_configuration_for(api)
    );
    for (auto const& port : obs.get_input_ports()) {
        std::cout << "Input port: " << port << "\n";
    }
    for (auto const& port : obs.get_output_ports()) {
        std::cout << "Output port: " << port << "\n";
    }
}

int main() {
    list_api_port(libremidi::API::ALSA_SEQ);

    auto virtualOutput = libremidi::midi_out{{}, libremidi::alsa_seq::output_configuration{}};
    if (virtualOutput.open_virtual_port("My virtual output port") != stdx::error{}) {
        std::cerr << "Error opening virtual port\n";
        return EXIT_FAILURE;
    }

    auto virtualInput =
        libremidi::midi_in{{.on_message = my_callback}, libremidi::alsa_seq::input_configuration{}};

    if (virtualInput.open_virtual_port("My virtual input port") != stdx::error{}) {
        std::cerr << "Error opening virtual port\n";
        return EXIT_FAILURE;
    }

    std::cout << "Virtual ports opened. Connect them using aconnect 128:0 129:0.\n";

    std::cout << "Press Enter to continue...\n";
    std::cin.get();

    list_api_port(libremidi::API::ALSA_SEQ);

    virtualOutput.send_message({0x90, 60, 127});
    virtualOutput.send_message({0x80, 60, 0});

    std::cout << "Press Enter to exit...\n";
    std::cin.get();

    return EXIT_SUCCESS;
}