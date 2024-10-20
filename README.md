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
