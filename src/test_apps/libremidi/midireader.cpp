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

// inline std::ostream& operator<<(std::ostream& s, const libremidi::container_identifier& id) {
//     struct
//     {
//         std::ostream& s;
//         void operator()(libremidi::uuid u) { s << "uuid"; }
//         void operator()(std::string u) { s << u; }
//         void operator()(uint64_t u) { s << u; }
//         void operator()(std::monostate) {}
//     } vis{s};
//     std::visit(vis, id);
//     return s;
// }

int main(int argc, char** argv) {
    libremidi::input_port input_port;

    bool removed = false;
    std::string_view display_name;
    libremidi::container_identifier id;

    std::string_view device_name = "Traktor Kontrol S4";
    if (argc > 1) {
        device_name = argv[1];
    }

    // constexpr auto api_to_use = libremidi::API::ALSA_RAW;
    constexpr auto api_to_use = libremidi::API::ALSA_SEQ;

    std::vector<libremidi::observer> observers;
    for (auto api : libremidi::available_apis()) {
        if (api != api_to_use) continue;
        std::string_view api_name = libremidi::get_api_display_name(api);

        std::cout << "Displaying ports for: " << api_name << std::endl;
        libremidi::observer_configuration cbs;
        cbs.input_added = [=](const libremidi::input_port& p) {
            std::cout << api_name << " : input added " << p << "\n";
        };
        cbs.input_removed = [&](const libremidi::input_port& p) {
            // std::cout << api_name << " : input removed " << p << "\n";
            if (id == p.container) {
                std::cout << device_name << " was removed by ID\n";
                removed = true;
            }
            if (display_name == p.display_name) {
                std::cout << device_name << " was removed by name\n";
                removed = true;
            }
        };
        cbs.output_added = [=](const libremidi::output_port& p) {
            std::cout << api_name << " : output added " << p << "\n";
        };
        cbs.output_removed = [=](const libremidi::output_port& p) {
            std::cout << api_name << " : output removed " << p << "\n";
        };
        observers.emplace_back(cbs, libremidi::observer_configuration_for(api));
        std::cout << "\n" << std::endl;
    }

    for (auto const& obs : observers) {
        if (obs.get_current_api() != api_to_use) continue;
        for (auto const& port : obs.get_input_ports()) {
            if (std::string::npos != port.display_name.find(device_name)) {
                std::cout << "Using port " << port.display_name << std::endl;
                input_port = port;
                id = input_port.container;
                display_name = input_port.display_name;
                break;
            }
        }
        if (!input_port.port_name.empty()) break;
    }

    if (input_port.display_name.empty()) {
        std::cerr << "Could not find " << device_name << "\n";
        return EXIT_FAILURE;
    }

    // Create the midi object
    libremidi::midi_in midi{libremidi::input_configuration{.on_message = my_callback}};

    // Open a given midi port.
    midi.open_port(input_port);

    while (!removed) {
        // Keep the application alive while receiving messages
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Closing the MIDI port" << std::endl;

    // midi.close_port();

    return EXIT_SUCCESS;
}