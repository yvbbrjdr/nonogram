#include <vector>

class Nonogram {
public:
    enum State {
        UNKNOWN,
        WHITE,
        BLACK
    };

    typedef std::vector<State> Line;
    typedef std::vector<Line> Board;
    typedef std::vector<int> Descriptions;
    typedef std::vector<Descriptions> Constraints;

    static Board empty_board(int rows, int cols);
    Board get_board();
    void set_board(const Board &b);
    void set_constraints(const Constraints &row, const Constraints &col);
    bool solve();
    void print_board();
private:
    enum RowCol {
        ROW,
        COL
    };

    Board board;
    Constraints row_constraints;
    Constraints col_constraints;

    Line get_line(RowCol row_col, int i);
    void set_line(RowCol row_col, int i, const Line &line);
    static Line line_solve(const Line &line, const Descriptions &desc);
    bool solved(int *row, int *col);
};
