# MusicLang
A language that generates Midi (for now)

## Features
What will it take to create a MusicLang
- [x] Creating the Language
    - [x] Lexer
    - [x] Parser
    - [x] Evaluator
- [ ] Making a Midi Library
    - [ ] Adding midi object
    - [ ] Adding note object
    - [ ] Adding time object
    - [ ] Add a lot of helper things
- [ ] Making a UI for non-code people
    - [ ] RayGUI


## Example
```music
let midi = create_midi_object();

midi->bpm = 120;
midi->add_note(C4);
midi->wait(QUARTER);
midi->add_note(C5);
midi->generate_midi("octave.midi");
```
