# MusicLang
A language that generates Midi

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
