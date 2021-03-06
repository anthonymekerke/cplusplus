/**
 * @file engine.cpp
 * See the AUTHORS or Authors.txt file
 */

#include "engine.hpp"
#include <iostream>

namespace kikotsoka {

    const Engine::Patterns Engine::PATTERNS = {
            {
                    {{false, false, true},  {false, true,  false}, {true,  false, false}},
                    {{true,  false, false}, {false, true,  false}, {false, false, true}}
            },
            {
                    {{false, true,  false}, {false, true,  false}, {true,  false, true}},
                    {{true,  false, false}, {false, true,  true},  {true,  false, false}},
                    {{true, false, true},  {false, true,  false}, {false, true,  false}},
                    {{false, false, true}, {true,  true,  false}, {false, false, true}}
            },
            {
                    {{false, true,  false}, {true,  true,  false}, {false, true,  true}},
                    {{false, true,  false}, {true,  true,  true},  {true,  false, false}},
                    {{true, true,  false}, {false, true,  true},  {false, true,  false}},
                    {{false, false, true}, {true,  true,  true},  {false, true,  false}}
            },
            {
                    {{true,  true,  false}, {true,  false, true},  {false, true,  true}},
                    {{false, true,  true},  {true,  false, true},  {true,  true,  false}}
            },
            {
                    {{true,  false, true},  {true,  false, true},  {true,  true,  true}},
                    {{true,  true,  true},  {true,  false, false}, {true,  true,  true}},
                    {{true, true,  true},  {true,  false, true},  {true,  false, true}},
                    {{true,  true,  true}, {false, false, true},  {true,  true,  true}}
            }
    };

    const size_t Engine::SIZE = 12;
    const size_t Engine::PAWN_START = 60;

    Engine::Engine()
            :_current_color(Color::BLACK), _board(SIZE), 
            _black_pawn_number(PAWN_START), _white_pawn_number(PAWN_START),
            _black_level(0), _white_level(0),
            _black_score(0), _white_score(0)
    {
        _black_last_move = Coordinates(-1,-1);
        _white_last_move = Coordinates(-1,-1);

        for (size_t l = 0; l < SIZE; ++l) {
            _board[l] = std::vector<State::Values>(SIZE);
            for (size_t c = 0; c < SIZE; ++c) {
                _board[l][c] = State::VACANT;
            }
        }
    }

    Engine::~Engine() { }

    State::Values Engine::state(const Coordinates& coordinates) const
    {
        return _board[coordinates.column_index()][coordinates.line_index()];
    }

    bool Engine::move(const Coordinates& coordinates){
        if(state(coordinates) != State::VACANT) {return false;}

        Color current_col = current_color() == Color::WHITE ? Color::WHITE : Color::BLACK;
        State::Values current_state = current_col == Color::WHITE ? State::WHITE : State::BLACK;

        if(current_col == Color::BLACK || current_col == Color::WHITE){
            _board[coordinates.column_index()][coordinates.line_index()] = current_state;
            pawn_number_m(current_col);

            if(current_color() == Color::BLACK) {black_last_move() = coordinates;}
            if(current_color() == Color::WHITE) {white_last_move() = coordinates;}

            return true;
        }

        return false;
    }

    void Engine::switch_player(){
        Color opponent_color = current_color() == Color::WHITE ? Color::BLACK : Color::WHITE;
        _current_color = opponent_color;
    }

    void Engine::configure_parameters_checking(const Coordinates& coord, 
            int& cs, int& ce, int& ls, int& le)
    {
        if(coord.column_index() > 1) {cs = -2;}
        if(coord.column_index() == 1) {cs = -1;}
        if(coord.column_index() == 0) {cs = 0;}

        if(coord.column_index() == SIZE - 1) {ce = -2;}
        if(coord.column_index() == SIZE - 2) {ce = -1;}
        if(coord.column_index() < SIZE - 2) {ce = 0;}
        
        if(coord.line_index() > 1) {ls = -2;}
        if(coord.line_index() == 1) {ls = -1;}
        if(coord.line_index() == 0) {ls = 0;}

        if(coord.line_index() == SIZE - 1) {le = -2;}
        if(coord.line_index() == SIZE - 2) {le = -1;}
        if(coord.line_index() < SIZE - 2) {le = 0;}
    }

    void Engine::aftermove(){
        int lev = current_color() == Color::BLACK ? black_level() : white_level();
        Coordinates coord = current_color() == Color::BLACK ? black_last_move() : white_last_move();
        int c_start, c_end, l_start, l_end, orientation;
        bool pattern_matching;

        configure_parameters_checking(coord, c_start, c_end, l_start, l_end);

        orientation = PATTERNS[lev].size();

        for(int o = 0; o < orientation; o++){
            for(int c = c_start; c <= c_end; c++){
                for(int l = l_start; l <= l_end; l++){
                    pattern_matching = match_pattern(lev, o, coord.column_index() + c, coord.line_index() + l);

                    if(pattern_matching){
                        lock_pattern(coord.column_index() + c, coord.line_index()+l);
                        level(_current_color);
                    }
                }
            }
        }
    }

    bool Engine::match_pattern(int s, int o, int c_start, int l_start){
        State::Values state_player = current_color() == Color::BLACK ? State::BLACK : State::WHITE;
        State::Values state_opponent = current_color() == Color::BLACK ? State::WHITE : State::BLACK;

        int checked_cells = 0;

        for(int l = 0; l < 3; l++){
            for(int c = 0; c < 3; c++){
                if((PATTERNS[s][o][l][c] == false && 
                        (_board[c+c_start][l+l_start] == state_opponent ||
                         _board[c+c_start][l+l_start] == State::VACANT)) ||
                    (PATTERNS[s][o][l][c] == true && 
                        _board[c+c_start][l+l_start] == state_player))
                {
                    checked_cells++;
                }
            }
        }

        return checked_cells == 9;
    }

    void Engine::lock_pattern(int c_start, int l_start){
        State::Values state_player = _current_color == Color::BLACK ? State::BLACK : State::WHITE;
        State::Values state_opponent = _current_color == Color::BLACK ? State::WHITE : State::BLACK;
        State::Values state_player_lock = _current_color == Color::BLACK ? State::BLACK_LOCK : State::WHITE_LOCK;

        for(int l = 0; l < 3; l++){
            for(int c = 0; c < 3; c++){
                if(_board[c_start+c][l_start+l] == state_opponent){
                    pawn_number_p(_current_color);
                    score(_current_color);
                    _board[c_start+c][l_start+l] = State::LOCK;
                }
                if(_board[c_start+c][l_start+l] == State::VACANT){
                    _board[c_start+c][l_start+l] = State::LOCK;
                }
                if(_board[c_start+c][l_start+l] == state_player){
                    _board[c_start+c][l_start+l] = state_player_lock;
                }
            }
        }
    }

    void Engine::block(){
        Coordinates opponent_last_move = _current_color == Color::BLACK ? white_last_move(): black_last_move();
        Coordinates player_last_move = _current_color == Color::BLACK ? black_last_move() : white_last_move();

        if(player_last_move.to_string() == "@0") {return;}

        int opponent_level = _current_color == Color::BLACK ? white_level() : black_level();

        unblock();

        if(opponent_level == 1){
            block_neighboring(opponent_last_move);
        }

        block_potential_patterns(player_last_move);
    }

    void Engine::block_neighboring(const Coordinates& coord){
        if(coord.column_index() != 0 && _board[coord.column_index() - 1][coord.line_index()]==State::VACANT){
            _board[coord.column_index() - 1][coord.line_index()] = State::BLOCK;
        }
        if(coord.column_index() != SIZE-1 && _board[coord.column_index() + 1][coord.line_index()]==State::VACANT){
            _board[coord.column_index() + 1][coord.line_index()] = State::BLOCK;
        }
        if(coord.line_index() != 0 && _board[coord.column_index()][coord.line_index()-1]==State::VACANT){
            _board[coord.column_index()][coord.line_index()-1] = State::BLOCK;
        }
        if(coord.column_index() != SIZE-1 && _board[coord.column_index()][coord.line_index()+1]==State::VACANT){
            _board[coord.column_index()][coord.line_index()+1] = State::BLOCK;
        }
    }

    void Engine::block_potential_patterns(const Coordinates& coord){
        int c_start, c_end, l_start, l_end;
        Coordinates current_coord;
        bool pattern_matching;

        int player_level = current_color() == Color::BLACK ? black_level() : white_level();

        configure_parameters_checking(coord, c_start, c_end, l_start, l_end);

        for(int cs = c_start; cs <= c_end; ++cs){
            for(int ls = l_start; ls <= l_end; ++ls){
                for(int ccc = 0; ccc < 3; ++ccc){
                    for(int ccl = 0; ccl < 3; ++ccl){

                        int column = coord.column_index() + cs + ccc;
                        int line = coord.line_index() + ls + ccl;
                        
                        if(_board[column][line] == State::VACANT){
                            _board[column][line] = _current_color == Color::BLACK ? State::BLACK : State::WHITE;

                            for(int l = 0; l < PATTERNS.size(); ++l){
                                for(int o = 0; o < PATTERNS[l].size(); ++o){
                                    if(l != player_level){
                                        pattern_matching = match_pattern(l, o, 
                                            cs + coord.column_index(), ls + coord.line_index());

                                        if(pattern_matching){
                                            _board[column][line] = State::BLOCK;
                                        }
                                    }
                                }
                            }

                            if(_board[column][line] != State::BLOCK) {_board[column][line] = State::VACANT;}
                        }
                    }
                }
            }
        }
    }

    void Engine::unblock(){
        for(int c = 0; c < SIZE; ++c){
            for(int l = 0; l < SIZE; ++l){
                if(_board[c][l] == State::BLOCK){
                    _board[c][l] = State::VACANT;
                }
            }
        }
    }

    int Engine::level(Color color){
        if(color == Color::BLACK){
            _black_level++;
            return _black_level;
        }

        if(color == Color::WHITE){
            _white_level++;
            return _black_level;
        }

        return -1;
    }

    int Engine::pawn_number_p(Color color){
        if(color == Color::BLACK){
            _black_pawn_number++;
            return _black_pawn_number;
        }

        if(color == Color::WHITE){
            _white_pawn_number++;
            return _white_pawn_number;
        }

        return 0;
    }

    int Engine::pawn_number_m(Color color){
        if(color == Color::BLACK){
            _black_pawn_number--;
            return _black_pawn_number;
        }

        if(color == Color::WHITE){
            _white_pawn_number--;
                return _white_pawn_number;
        }

        return 0;
    }

    int Engine::score(Color color){
        if(color == Color::BLACK){
            _black_score++;
            return _black_score;
        }

        if(color == Color::WHITE){
            _white_score++;
            return _white_score;
        }

        return 0;
    }

    /*
        Must be called after block() function
    */
    std::vector<Coordinates> Engine::get_possible_moves() const{
        std::vector<Coordinates> possible_moves;
        Coordinates coords;

        for(int l = 0; l < SIZE; ++l){
            for(int c = 0; c < SIZE; ++c){
                if(_board[c][l] == State::VACANT){
                    coords = Coordinates(c,l);
                    possible_moves.push_back(coords);
                }
            }
        }

        return possible_moves;
    }
}
