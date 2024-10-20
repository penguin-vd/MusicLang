# MusicLang
**MusicLang** is a programming language designed for musicians and developers to easily generate MIDI files. Whether you're creating simple melodies or complex compositions, MusicLang provides intuitive syntax to bring your musical ideas to life.

## Example
The following code demonstrates how to generate a simple melody:

```midilang
let midi = make_midi_object();

midi->AddNote(NOTES->D5, TIME->EIGHTH, 127);
midi->Wait(TIME->EIGHTH);
midi->AddNote(NOTES->E5, TIME->EIGHTH, 127);
midi->Wait(TIME->EIGHTH);
midi->AddNote(NOTES->F5, TIME->EIGHTH, 127);
midi->Wait(TIME->EIGHTH);
midi->AddNote(NOTES->G5, TIME->EIGHTH, 127);
midi->Wait(TIME->EIGHTH);
midi->AddNote(NOTES->E5, TIME->EIGHTH, 127);
midi->Wait(TIME->EIGHTH);
midi->AddNote(NOTES->C5, TIME->EIGHTH, 127);
midi->Wait(TIME->EIGHTH);
midi->AddNote(NOTES->D5, TIME->EIGHTH, 127);
midi->GenerateMidi("TheLick.midi");
```

This code will create a MIDI file called **TheLick.midi** with a sequence of notes, demonstrating the straightforward yet powerful syntax of MusicLang.

## Getting Started
### Build Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/MusicLang.git
   ```
2. Create a build directory and navigate into it:
   ```bash
   mkdir build && cd build
   ```
3. Run CMake to configure the project:
   ```bash
   cmake ..
   ```
4. Compile the project:
   ```bash
   make
   ```

### Download
You can download the latest release [here](https://github.com/penguin-vd/MusicLang/releases).

## Upcoming Features
- **Graphical User Interface (GUI)**: A user-friendly interface that allows you to preview MIDI files, visualize your composition, and edit MIDI tracks using a block-based coding approach. This feature aims to make MusicLang even more accessible to musicians who prefer a visual coding experience.
