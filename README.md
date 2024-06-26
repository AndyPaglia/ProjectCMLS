# ProjectCMLS
## Introduction
Our project consists of a glove with a sensor which detects the hand’s movements and by using a Bluetooth communication, the data are collected by our interaction system unit. The system unit elaborates the data received, to create a MIDI message suitable for SuperCollider. SuperCollider is used to generate different kind of waves that can be chosen by the GUI. The waves are then elaborated by the VST, created by JUCE, to obtain a sound similar to human voice. The graphic interface, other than choosing the type of wave, give the possibility to change some parameters. This interface communicates through OSC messages to SuperCollider.

## Processing

This Processing code creates an interactive graphical interface using the `controlP5` and `oscP5` libraries. The interface includes knob controls, a dropdown menu, and a movable circle. Additionally, the code enables communication via OSC (Open Sound Control) to interact with SuperCollider. Below is a description of the main features and sections of the code.

### Initialization

#### Global Variables

The code declares several global variables to manage the user interface components and OSC communication. These variables include objects for graphical controls, an array of musical notes, variables to manage the position of the circle, and parameters to draw boundary rectangles.

#### Setup

The `setup()` function configures the drawing environment, initializes the font, and creates ControlP5 objects for the user interface. Parameters and positions of various interface components, including knobs (Unison, Detune, and Volume) and a dropdown menu with waveform options, are defined. OSC connection with SuperCollider is also initialized.

### Drawing

The `draw()` function is executed in a loop and handles the graphical rendering of the interface. It draws boundary rectangles, text, and the movable circle. The circle changes position based on received OSC messages, and the text displays the current MIDI note.

### Control Events

#### Handling Control Events

The `controlEvent()` function manages events generated by user interaction with the interface controls. When a control is modified, an OSC message is created that includes the type of control and its current value, which is then sent to SuperCollider.

#### Receiving OSC Messages

The `oscEvent()` function handles received OSC messages. In particular, it responds to messages indicating hand movement and MIDI note messages. Movement values are mapped to control the position of the circle on the screen, while MIDI note messages update the displayed current note.

### Utility Functions

#### Mapping Float Values

The code includes a function to map float values from one range to another, useful for converting interface control values into a format suitable for OSC communication.

### Features

1. **Graphical Interface**: Includes three knobs (Unison, Detune, Volume), a dropdown menu to select waveforms, and a movable circle.
2. **OSC Communication**: Sending and receiving OSC messages to interact with SuperCollider.
3. **Circle Movement**: The circle within the rectangle moves in response to received OSC messages.
4. **MIDI Note Visualization**: Displays the current note received via OSC messages.

This interface is useful for controlling audio parameters in real-time through SuperCollider, facilitating the creation and manipulation of sounds in a visual and interactive manner.


## SuperCollider

This SuperCollider script sets up a sophisticated audio processing environment that includes MIDI and OSC (Open Sound Control) interactions, a variety of synthesizer waveforms, and integration with VST plugins for effects. Here’s a detailed explanation of the script:

#### Initialization and Setup

1. **Server Boot and MIDI Setup**: 
   The script starts by booting the SuperCollider server and initializing the MIDI client, connecting to all available MIDI sources.

2. **VST Plugin Search and Audio Bus Creation**: 
   It searches for VST plugins and creates an audio bus for routing audio signals.

#### Synth Definitions

3. **SynthDef for VST Effect**: 
   A SynthDef named `\effetto` is defined to handle VST plugins. It processes audio through the VST plugin and outputs the processed audio.

4. **SynthDefs for Different Waveforms**: 
   Several SynthDefs are defined, each generating a different type of waveform:
   - `Sin`: Generates a sine wave.
   - `Impulse`: Generates an impulse wave.
   - `Sawtooth`: Generates a sawtooth wave.
   - `Square`: Generates a square wave.

   Each SynthDef includes parameters for frequency, pan, detune, amplitude, and gate, and applies an ADSR envelope to shape the sound.

#### VST Plugin Control and Audio Playback

5. **VST Plugin Controller**: 
   An instance of the VST plugin is opened and controlled using `VSTPluginController`. The plugin can be visually edited via a GUI.

6. **Bus Playback**: 
   The audio bus is played to ensure that the audio routed through it is heard.

#### OSC and MIDI Interactions

7. **OSC Message Handling**: 
   OSC messages are set up to select different waveforms based on input, modify detune and volume, and receive other control messages.

8. **MIDI Note and Control Change Handling**: 
   - `MIDIdef.noteOn`: Handles MIDI note-on events, setting the frequency and triggering the gate.
   - `MIDIdef.noteOff`: Handles MIDI note-off events, releasing the gate.
   - `MIDIdef.cc`: Handles MIDI control change messages to adjust VST plugin parameters, specifically for controlling filter frequencies.

9. **Sending OSC Messages**: 
   The script sends OSC messages containing note frequency information to another application or hardware, facilitating communication and control across different platforms.

### Summary

This script demonstrates a complex setup where MIDI and OSC protocols are used to control various aspects of audio synthesis and processing in real-time. It integrates multiple waveforms, uses VST plugins for effects, and allows dynamic control through both MIDI and OSC messages. This setup is particularly useful for interactive audio applications, live performances, and experimental sound design.


## Juce

This audio plugin, implemented with the JUCE framework, connects to an OSC (Open Sound Control) server to receive messages that control the plugin's parameters. The audio processor applies three band-pass filters to copies of the original audio buffer, combining the results based on the values of the received MIDI messages.

### `CMLSJuceAudioProcessor` Class Constructor

The constructor:

- Initializes the audio processor with stereo input and output channels.
- Attempts to connect to port 9000 to receive OSC messages.
- Adds listeners for two specific OSC addresses: "/handMovement/x" and "/handMovement/y".

### Preparing for Playback

The `prepareToPlay` method:

- Configures the processor for playback, initializing the filters and creating temporary audio buffers for three bands.

### Audio Block Processing

The `processBlock` method:

- Normalizes the input channels.
- Creates copies of the original audio buffer.
- Applies a band-pass filter to each buffer copy with different cutoff frequencies.
- Calculates the cutoff frequencies based on received OSC messages.
- Combines the filtered copies and returns them to the original buffer, with weighting based on the received MIDI messages.

### Plugin Parameter Configuration

The `createLayout` method:

- Creates the plugin's parameter layout using `AudioProcessorValueTreeState`.

### Receiving OSC Messages

The `oscMessageReceived` method:

- Receives OSC messages and updates the corresponding MIDI values.

### Frequency Calculation

The `calcFreqs` method:

- Calculates the cutoff frequencies for the three band-pass filters (`freq1`, `freq2`, `freq3`) based on the MIDI values received through OSC messages.

