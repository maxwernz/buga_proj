#include <iostream>
#include <vector>
#include "pbma.h"
#include "Acker.h"

void aufg_1(const std::vector<Acker>& data, bool show_files) {
    for (Acker acker : data) {
        acker.print_out(show_files);
    }
}

int main(int argc, char** argv) {
    args_t args(argc, argv);
    std::vector<Acker> data;
    for (std::string path : args.positionals()) {
        path.insert(0, "../");
        if (file_exists(path)) {
            std::vector<std::string> lines = read_lines(path);
            data.emplace_back(lines, path);
        }
    }
    bool flag_z = args.flag("z");

    aufg_1(data, flag_z);
}

