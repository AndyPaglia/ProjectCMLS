# ProjectCMLS
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
