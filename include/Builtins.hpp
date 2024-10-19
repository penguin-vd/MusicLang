#include <memory>
#include <vector>
#include "Object.hpp"

std::shared_ptr<IObject> ExitCall(const std::vector<std::shared_ptr<IObject>>& args);
std::shared_ptr<IObject> Range(const std::vector<std::shared_ptr<IObject>>& args);
std::shared_ptr<IObject> Print(const std::vector<std::shared_ptr<IObject>>& args);
std::shared_ptr<IObject> MakeMidiObject(const std::vector<std::shared_ptr<IObject>>& args);

std::shared_ptr<IObject> Type(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args);
std::shared_ptr<IObject> AddNote(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args);
std::shared_ptr<IObject> Wait(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args);
std::shared_ptr<IObject> GenerateMidi(std::shared_ptr<IObject> self, const std::vector<std::shared_ptr<IObject>>& args);
