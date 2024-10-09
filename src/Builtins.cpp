#include "Builtins.hpp"


std::shared_ptr<IObject> ExitCall(const std::vector<std::shared_ptr<IObject>>& args) {
    int code = 0;
    if (!args.empty()) {
        if (args[0]->Type() == ObjectType::INTEGER) {
            code = std::static_pointer_cast<Integer>(args[0])->Value;
        }
    }
    return std::make_shared<ExitObject>(code);
}
