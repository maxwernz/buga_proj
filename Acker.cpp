//
// Created by Maximilian Wernz on 12.05.23.
//

#include "Acker.h"

#include <utility>

Acker::Acker(const std::vector<std::string>& lines, std::string  path) : m_lines(lines), m_path(std::move(path)){
    m_rows = int(lines.size());
    if (m_rows > 0) {
        m_columns = int(lines[0].size());
    } else {
        m_columns = 0;
    }
    m_flowers = get_flowers();
}

void Acker::print_out(bool show_file) {
    std::string rows = format(m_rows, 3);
    std::string columns = format(m_columns, 3);
    std::string flowers = format(m_flowers, 6);
    std::cout << m_path << ": Zeilen=" << rows << ", Spalten=" << columns << ", Blumen=" << flowers << std::endl;

    if (show_file) {
        std::cout << "  ";
        for (int i=0; i < m_columns; ++i) {
            std::cout << ' ' << i;
        }
        std::cout << std::endl;

        for (int i=0; i < m_rows; ++i) {
            std::cout << i << ":";
            for (int j=0; j < m_columns; ++j) {
                std::cout << ' ' << m_lines[i][j];
            }
            std::cout << std::endl;
        }
    }
}

int Acker::get_flowers() {
    int flowers = 0;
    for (const std::string& line : m_lines) {
        for (const char& ch : line) {
            if (ch == '*') {
                flowers += 1;
            }
        }
    }
    return flowers;
}
