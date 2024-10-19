#include "Builtins.hpp"

#include <algorithm>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>

#include "Enviroment.hpp"
#include "Object.hpp"
#include "fmt/core.h"

template <typename... Args>
std::shared_ptr<Error> CreateError(std::string format, Args... args) {
    return std::make_shared<Error>(fmt::format(format, args...));
}

std::shared_ptr<Error> CreateError(std::string format) {
    return std::make_shared<Error>(format);
}

std::shared_ptr<IObject> ExitCall(const std::vector<std::shared_ptr<IObject>>& args) {
    int code = 0;
    if (!args.empty()) {
        if (args[0]->Type() == ObjectType::INTEGER) {
            code = std::static_pointer_cast<Integer>(args[0])->Value;
        }
    }
    return std::make_shared<ExitObject>(code);
}

std::shared_ptr<IObject> Range(const std::vector<std::shared_ptr<IObject>>& args) {
    if (args.size() != 2 && args.size() != 3) {
        return CreateError("wrong number of arguments. got={0}, want=2 or 3", args.size());
    }
    for (const auto& arg : args) {
        if (arg->Type() != ObjectType::INTEGER) {
            return CreateError("wrong type expected. want=integer");
        }
    }
    int low = std::static_pointer_cast<Integer>(args[0])->Value;
    int high = std::static_pointer_cast<Integer>(args[1])->Value;
    int step = 1;
    if (args.size() == 3) {
        step = std::static_pointer_cast<Integer>(args[2])->Value;
    }
    return std::make_shared<IterObj>(low, high, step);
}

std::shared_ptr<IObject> Print(const std::vector<std::shared_ptr<IObject>>& args) {
    if (args.size() != 1) {
        return CreateError("wrong number of arguments. got={0}, want=1", args.size());
    }

    std::cout << args[0]->Inspect() << std::endl;
    return Env::NULLOBJ;
}

std::shared_ptr<IObject> MakeMidiObject(const std::vector<std::shared_ptr<IObject>>& args) {
    if (args.size() != 0) {
        return CreateError("wrong number of arguments. got={0}, want=0", args.size());
    }
    return make_shared<MidiObj>();
}

std::shared_ptr<IObject> Type(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args) {
    return make_shared<StringObj>(fmt::format("{0}", self->Type()));
}

std::shared_ptr<IObject> AddNote(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args) {
    if (self->Type() != ObjectType::MIDI) {
        return CreateError("{} doesn't have the function AddNote", self->Type());
    }

    if (args.size() != 3) {
        return CreateError("wrong number of arguments. got={0}, want=3", args.size());
    }

    if (args[0]->Type() != ObjectType::INTEGER || args[1]->Type() != ObjectType::INTEGER || args[2]->Type() != ObjectType::INTEGER) {
        return CreateError("type mismatch, want 3x INTEGER got {0}, {1}, {2}", args[0]->Type(), args[1]->Type(), args[2]->Type());
    }

    auto midi = static_pointer_cast<MidiObj>(self);
    int note_duration_tick = TICKS_PER_QUARTER * 4 / static_pointer_cast<Integer>(args[1])->Value;
    midi->Notes.push_back(MidiNoteEvent(
        static_pointer_cast<Integer>(args[0])->Value,
        static_pointer_cast<Integer>(args[2])->Value,
        midi->currentTime, true));

    midi->Notes.push_back(MidiNoteEvent(
        static_pointer_cast<Integer>(args[0])->Value, 0,
        midi->currentTime + note_duration_tick, false));

    return Env::NULLOBJ;
}

std::shared_ptr<IObject> Wait(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args) {
    if (self->Type() != ObjectType::MIDI) {
        return CreateError("{} doesn't have the function Wait", self->Type());
    }

    if (args.size() != 1) {
        return CreateError("wrong number of arguments. got={0}, want=1", args.size());
    }

    if (args[0]->Type() != ObjectType::INTEGER) {
        return CreateError("type mismatch, want INTEGER got {0}", args[0]->Type());
    }

    auto midi = static_pointer_cast<MidiObj>(self);
    midi->currentTime += 480 * 4 / static_pointer_cast<Integer>(args[0])->Value;
    return Env::NULLOBJ;
}

void write_big_endian(std::ofstream& file, uint32_t value, size_t byte_count) {
    for (int i = (byte_count - 1) * 8; i >= 0; i -= 8) {
        file.put(static_cast<unsigned char>((value >> i) & 0xFF));
    }
}

std::shared_ptr<IObject> GenerateMidi(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args) {
    if (self->Type() != ObjectType::MIDI) {
        return CreateError("{} doesn't have the function GenerateMidi", self->Type());
    }

    if (args.size() != 1) {
        return CreateError("wrong number of arguments. got={0}, want=1", args.size());
    }

    if (args[0]->Type() != ObjectType::STRING) {
        return CreateError("type mismatch, want INTEGER got {0}", args[0]->Type());
    }

    auto midi = static_pointer_cast<MidiObj>(self);
    std::string filename = static_pointer_cast<StringObj>(args[0])->Value;
    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return CreateError("failed to create file with name={0}", filename);
    }

    // Write midi header
    file.write("MThd", 4);
    write_big_endian(file, 6, 4);    // 4 bytes for header length
    write_big_endian(file, 1, 2);    // 2 bytes for format type
    write_big_endian(file, 1, 2);    // 2 bytes for number of tracks
    write_big_endian(file, 480, 2);  // 2 bytes for time division

    std::sort(midi->Notes.begin(), midi->Notes.end(), [](const MidiNoteEvent& a, const MidiNoteEvent& b) {
        return a.Time < b.Time;
    });

    // Write track data
    file.write("MTrk", 4);
    std::vector<uint8_t> trackData;
    uint32_t lastTime = 0;

    for (auto& event : midi->Notes) {
        auto data = event.GenerateEvent(lastTime);
        trackData.insert(trackData.end(), data.begin(), data.end());
        lastTime = event.Time;
    }

    trackData.push_back(0x00);  // Delta time 0
    trackData.push_back(0xFF);  // Meta event
    trackData.push_back(0x2F);  // End of track
    trackData.push_back(0x00);  // Meta event length

    write_big_endian(file, trackData.size(), 4);

    if (!trackData.empty()) {
        file.write(reinterpret_cast<const char*>(trackData.data()), trackData.size());
    }

    file.close();

    return Env::NULLOBJ;
}
