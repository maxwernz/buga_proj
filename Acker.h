//
// Created by Maximilian Wernz on 12.05.23.
//

#ifndef BUGA_PROJ_ACKER_H
#define BUGA_PROJ_ACKER_H

#include <vector>
#include <string>
#include "pbma.h"


class Acker {
    std::vector<std::string> m_lines;
    int m_columns;
    int m_rows;
    int m_flowers;
    std::string m_path;

    int get_flowers();
public:
    Acker(const std::vector<std::string>& lines, std::string  path);
    int get_rows() const { return m_rows; }
    int get_columns() const { return m_columns; }
    void print_out(bool show_file);
};


#endif //BUGA_PROJ_ACKER_H
