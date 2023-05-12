#include <iostream>
#include <vector>
#include "pbma.h"
#include "Acker.h"

void aufg_1(const std::vector<Acker>& data, bool show_files) {
    for (Acker acker : data) {
        acker.print_out(show_files);
    }
}

void aufg_2(const std::vector<Acker>& data) {
    for (Acker acker : data) {
        int steps = 0;
        int tour_numbers = 0;
        // time
        Timer time;
        std::vector<std::string> tours = acker.simple_solution(steps, tour_numbers);
        std::cout << "einfache Loesung: " << tour_numbers << " Tour(en), " << steps << " Schritte, " << time.human_measure() << std::endl;
        for (const std::string& tour : tours) {
            std::cout << tour << std::endl;
        }
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

//    aufg_1(data, flag_z);
    aufg_2(data);
}

