#include "Builtins.hpp"
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
