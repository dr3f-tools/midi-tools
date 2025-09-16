#!/usr/bin/env python3

import mido
import gi

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

Gst.init(None)

# --- Find the CASIO USB-MIDI port ---
ports = mido.get_input_names()
casio_port = None
for port in ports:
    if "CASIO" in port.upper():
        casio_port = port
        break

if casio_port is None:
    raise RuntimeError(f"CASIO USB-MIDI not found. Available ports: {ports}")

print(f"Connecting to CASIO USB-MIDI: {casio_port}")
inport = mido.open_input(casio_port)

# --- Main pipeline ---
pipeline = Gst.Pipeline.new("synth")
mixer = Gst.ElementFactory.make("audiomixer", "mixer")
sink = Gst.ElementFactory.make("autoaudiosink", "sink")
pipeline.add(mixer)
pipeline.add(sink)
mixer.link(sink)

active_notes = {}

def midi_to_freq(note):
    return 440.0 * 2.0 ** ((note - 69) / 12.0)

def handle_midi(msg):
    global pipeline, mixer, active_notes

    if msg.type == 'note_on' and msg.velocity > 0:
        note = msg.note
        if note in active_notes:
            return
        freq = midi_to_freq(note)
        print(f"Note ON: {note}, freq {freq:.2f} Hz")

        src = Gst.ElementFactory.make("audiotestsrc", f"src_{note}")
        src.set_property("wave", "saw")
        src.set_property("freq", freq)
        src.set_property("volume", msg.velocity / 127.0)

        conv = Gst.ElementFactory.make("audioconvert", f"conv_{note}")

        pipeline.add(src)
        pipeline.add(conv)
        src.link(conv)
        conv.link(mixer)

        src.sync_state_with_parent()
        conv.sync_state_with_parent()

        active_notes[note] = (src, conv)

    elif msg.type in ('note_off', 'note_on') and (msg.type == 'note_off' or msg.velocity == 0):
        note = msg.note
        if note in active_notes:
            print(f"Note OFF: {note}")
            src, conv = active_notes[note]
            conv.unlink(mixer)
            src.set_state(Gst.State.NULL)
            conv.set_state(Gst.State.NULL)
            pipeline.remove(src)
            pipeline.remove(conv)
            del active_notes[note]

pipeline.set_state(Gst.State.PLAYING)
print("Listening to CASIO USB-MIDI... Press Ctrl+C to stop.")

try:
    for msg in inport:
        handle_midi(msg)
except KeyboardInterrupt:
    print("Stopping...")
finally:
    pipeline.set_state(Gst.State.NULL)
