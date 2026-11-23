#include <cstring>

#include "composition_root.hpp"
#include "ui/demo.hpp"
#include "ui/menu.hpp"

int main(int argc, char** argv) {
    auto facade = pos::buildFacade();
    if (argc > 1 && std::strcmp(argv[1], "--demo") == 0) {
        pos::ui::runDemo(*facade, std::cout);
    } else {
        pos::ui::runMenu(*facade, std::cin, std::cout);
    }
}
