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

int main() {
    libremidi::input_port input_port;
    libremidi::observer obs;
    for (libremidi::input_port const& port : obs.get_input_ports()) {
        std::cout << port.port_name << std::endl;
        if (std::string::npos != port.port_name.find("CASIO")) {
            std::cout << "Using port " << port.port_name << std::endl;
            input_port = port;
            break;
        }
    }

    if (input_port.port_name.empty()) {
        std::cerr << "Could not find the CASIO port\n";
        return EXIT_FAILURE;
    }

    // Create the midi object
    libremidi::midi_in midi{libremidi::input_configuration{.on_message = my_callback}};

    // Open a given midi port.
    // The argument is a libremidi::input_port gotten from a
    // libremidi::observer.
    midi.open_port(input_port);

    while (midi.is_port_connected()) {
        // Keep the application alive while receiving messages
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Closing the MIDI port" << std::endl;

    midi.close_port();

    return EXIT_SUCCESS;
}