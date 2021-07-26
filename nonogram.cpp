#include <cstdio>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <unistd.h>

#include "memoize.h"
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

            if (isatty(fileno(stdout))) {
                print_board();
                printf("\033[%ldA", row_constraints.size());
            }

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
    typedef std::function<bool(int, int)> FixFunction;
    typedef std::function<Line(int, int)> PaintFunction;

    Line s = line;
    s.insert(s.begin(), WHITE);

    auto match_single = [](State s, State d) -> bool {
        return s == UNKNOWN || s == d;
    };

    auto merge_c = [](State s, State t) -> State {
        return s == t ? s : UNKNOWN;
    };

    auto merge = [&](const Line &s, const Line &t) -> Line {
        Line ret;
        for (unsigned long i = 0; i < s.size(); ++i)
            ret.push_back(merge_c(s[i], t[i]));
        return ret;
    };

    FixFunction fix, fix_0, fix_1;

    fix = memoize(FixFunction([&](int i, int j) -> bool {
        return (i == 0) ? (j == 0) : (fix_0(i, j) || fix_1(i, j));
    }));

    fix_0 = memoize(FixFunction([&](int i, int j) -> bool {
        return (s[i - 1] == WHITE || s[i - 1] == UNKNOWN) && fix(i - 1, j);
    }));

    fix_1 = memoize(FixFunction([&](int i, int j) -> bool {
        int dj = desc[j - 1];
        if (j < 1 || i < dj + 1 || !match_single(s[i - dj - 1], WHITE))
            return false;
        for (int k = i - dj + 1; k <= i; ++k) {
            if (!match_single(s[k - 1], BLACK))
                return false;
        }
        return fix(i - dj - 1, j - 1);
    }));

    PaintFunction paint, paint_prime, paint_0, paint_1;

    paint = memoize(PaintFunction([&](int i, int j) -> Line {
        return i == 0 ? Line() : paint_prime(i, j);
    }));

    paint_prime = memoize(PaintFunction([&](int i, int j) -> Line {
        bool f0 = fix_0(i, j);
        bool f1 = fix_1(i, j);
        if (f0 && !f1)
            return paint_0(i, j);
        if (!f0 && f1)
            return paint_1(i, j);
        return merge(paint_0(i, j), paint_1(i, j));
    }));

    paint_0 = memoize(PaintFunction([&](int i, int j) -> Line {
        Line ret = paint(i - 1, j);
        ret.push_back(WHITE);
        return ret;
    }));

    paint_1 = memoize(PaintFunction([&](int i, int j) -> Line {
        int dj = desc[j - 1];
        Line ret = paint(i - dj - 1, j - 1);
        ret.push_back(WHITE);
        for (int k = 0; k < dj; ++k)
            ret.push_back(BLACK);
        return ret;
    }));

    if (desc.size() == 0)
        return Line(s.size(), WHITE);

    if (!fix(s.size(), desc.size()))
        return Line();
    
    Line new_line = paint(s.size(), desc.size());
    new_line.erase(new_line.begin());
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
