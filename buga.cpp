#include <iostream>
#include "pbma.h"

int main(int argc, char** argv) {
    args_t args(argc, argv);
    for (const std::string& ele : args.positionals()) {
        std::cout << ele << std::endl;
    }

}
