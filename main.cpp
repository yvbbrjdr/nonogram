#include <iostream>
#include <sstream>

#include "nonogram.h"

int main()
{
    int row, col;
    Nonogram nonogram;
    Nonogram::Constraints row_constraints, col_constraints;

    std::string line;
    getline(std::cin, line);
    std::istringstream iss(line);
    iss >> row >> col;

    for (int i = 0; i < row; ++i) {
        Nonogram::Descriptions desc;
        int n;
        std::string line;
        getline(std::cin, line);
        std::istringstream iss(line);
        while (iss >> n)
            desc.push_back(n);
        row_constraints.push_back(desc);
    }

    for (int i = 0; i < col; ++i) {
        Nonogram::Descriptions desc;
        int n;
        std::string line;
        getline(std::cin, line);
        std::istringstream iss(line);
        while (iss >> n)
            desc.push_back(n);
        col_constraints.push_back(desc);
    }

    nonogram.set_constraints(row_constraints, col_constraints);
    nonogram.set_board(Nonogram::empty_board(row, col));
    if (!nonogram.solve())
        printf("IMPOSSIBLE!\nLast state:\n");
    nonogram.print_board();

    return 0;
}
