let midi = make_midi();

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
