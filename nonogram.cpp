#include <cstdio>
#include <functional>
#include <map>
#include <queue>
#include <set>

#include "nonogram.h"

Nonogram::Board Nonogram::empty_board(int rows, int cols)
{
    Board board;
    Line line(cols, Nonogram::UNKNOWN);
    for (int i = 0; i < rows; ++i)
        board.push_back(line);
    return board;
}

Nonogram::Board Nonogram::get_board()
{
    return board;
}

void Nonogram::set_board(const Board &b)
{
    board = b;
}

void Nonogram::set_constraints(const Constraints &row, const Constraints &col)
{
    row_constraints = row;
    col_constraints = col;
}

bool Nonogram::solve()
{
    std::function<bool(int, int)> helper = [&](int modified_row, int modified_col) -> bool {
        std::queue<std::pair<RowCol, int>> pending_solve;
        std::set<std::pair<RowCol, int>> in_pending_solve;

        if (modified_row < 0 || modified_col < 0) {
            for (unsigned long i = 0; i < row_constraints.size(); ++i) {
                auto p = std::make_pair(ROW, i);
                pending_solve.push(p);
                in_pending_solve.insert(p);
            }
            for (unsigned long i = 0; i < col_constraints.size(); ++i) {
                auto p = std::make_pair(COL, i);
                pending_solve.push(p);
                in_pending_solve.insert(p);
            }
        } else {
            auto p = std::make_pair(ROW, modified_row);
            pending_solve.push(p);
            in_pending_solve.insert(p);
            p = std::make_pair(COL, modified_col);
            pending_solve.push(p);
            in_pending_solve.insert(p);
        }

        while (!pending_solve.empty()) {
            auto p = pending_solve.front();
            pending_solve.pop();
            in_pending_solve.erase(p);
            RowCol row_col = p.first;
            int i = p.second;
            RowCol other_row_col = row_col == ROW ? COL : ROW;
            Descriptions &desc = row_col == ROW ? row_constraints[i] : col_constraints[i];

            Line line = get_line(row_col, i);
            Line new_line = line_solve(line, desc);
            if (new_line.empty())
                return false;

            bool different = false;
            for (unsigned long j = 0; j < line.size(); ++j) {
                if (line[j] != new_line[j]) {
                    auto p = std::make_pair(other_row_col, j);
                    if (in_pending_solve.find(p) == in_pending_solve.end()) {
                        pending_solve.push(p);
                        in_pending_solve.insert(p);
                    }
                    different = true;
                }
            }
            if (different)
                set_line(row_col, i, new_line);
        }

        int row, col;
        if (solved(&row, &col))
            return true;

        Board backup = board;
        board[row][col] = BLACK;
        if (helper(row, col))
            return true;
        board = backup;
        board[row][col] = WHITE;
        if (helper(row, col))
            return true;

        board = backup;
        return false;
    };

    return helper(-1, -1);
}

void Nonogram::print_board()
{
    for (unsigned long i = 0; i < board.size(); ++i) {
        for (unsigned long j = 0; j < board[i].size(); ++j) {
            switch (board[i][j])
            {
            case UNKNOWN:
                putchar('?');
                break;
            case WHITE:
                putchar(' ');
                break;
            case BLACK:
                putchar('*');
                break;
            }
        }
        putchar('\n');
    }
}

Nonogram::Line Nonogram::get_line(RowCol row_col, int i)
{
    if (row_col == ROW) {
        return board[i];
    } else {
        Line line;
        for (unsigned long j = 0; j < board.size(); ++j)
            line.push_back(board[j][i]);
        return line;
    }
}

void Nonogram::set_line(RowCol row_col, int i, const Line &line)
{
    if (row_col == ROW) {
        board[i] = line;
    } else {
        for (unsigned long j = 0; j < board.size(); ++j)
            board[j][i] = line[j];
    }
}

Nonogram::Line Nonogram::line_solve(const Line &line, const Descriptions &desc)
{
    Line new_line;
    Descriptions splits;

    auto process_splits = [&]() {
        Line proposed_line;
        Descriptions s = splits;

        s[0] -= 1;
        s[s.size() - 1] -= 1;

        for (unsigned long i = 0; i < desc.size(); ++i) {
            for (int j = 0; j < s[i]; ++j)
                proposed_line.push_back(WHITE);
            for (int j = 0; j < desc[i]; ++j)
                proposed_line.push_back(BLACK);
        }
        for (int i = 0; i < s[s.size() - 1]; ++i)
            proposed_line.push_back(WHITE);

        for (unsigned long i = 0; i < line.size(); ++i) {
            if ((line[i] == WHITE && proposed_line[i] == BLACK) ||
                (line[i] == BLACK && proposed_line[i] == WHITE))
                return;
        }

        if (new_line.empty()) {
            new_line = proposed_line;
        } else {
            for (unsigned long i = 0; i < new_line.size(); ++i) {
                if (new_line[i] != proposed_line[i])
                    new_line[i] = UNKNOWN;
            }
        }
    };

    std::function<void(int, int)> split_int = [&](int sum, int n) {
        if (n == 1) {
            splits.push_back(sum);
            process_splits();
            splits.pop_back();
            return;
        }

        for (int i = 1; i <= sum - n + 1; ++i) {
            splits.push_back(i);
            split_int(sum - i, n - 1);
            splits.pop_back();
        }
    };

    int sum = 0;
    for (unsigned long i = 0; i < desc.size(); ++i)
        sum += desc[i];
    split_int(line.size() - sum + 2, desc.size() + 1);
    return new_line;
}

bool Nonogram::solved(int *row, int *col) {
    for (unsigned long i = 0; i < board.size(); ++i) {
        for (unsigned long j = 0; j < board[i].size(); ++j) {
            if (board[i][j] == UNKNOWN) {
                if (row)
                    *row = i;
                if (col)
                    *col = j;
                return false;
            }
        }
    }
    return true;
}
