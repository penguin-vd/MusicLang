# MusicLang
A language that generates Midi (for now)

## Features
What will it take to create a MusicLang
- [x] Creating the Language
    - [x] Lexer
    - [x] Parser
    - [x] Evaluator
- [ ] Making a Midi Library
    - [x] Adding midi object
    - [x] Adding note object
    - [x] Adding time object
    - [ ] Add a lot of helper things
- [ ] Making a UI for non-code people
    - [ ] RayGUI


## Example
```midilang
let midi = make_midi_object();

midi->bpm = 120;
midi->add_note(NOTES->C4);
midi->wait(TIME->QUARTER);
midi->add_note(NOTES->C5);
midi->generate_midi("octave.midi");
```
