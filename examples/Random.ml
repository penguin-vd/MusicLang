let midi = make_midi();
let notes = [NOTES->C5, NOTES->D5, NOTES->E5, NOTES->G5, NOTES->A5];
let timings = [TIME->QUARTER, TIME->HALF, TIME->EIGHTH];

for (i in range(0, 10)) {
    let time = random(timings);
    let note = random(notes);
    midi->AddNote(note, time, 127);
    midi->Wait(time);
}

midi->GenerateMidi("Random.midi");
