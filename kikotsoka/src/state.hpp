/**
 * @file state.hpp
 * See the AUTHORS or Authors.txt file
 */

#ifndef KIKOTSOKA_STATE_HPP
#define KIKOTSOKA_STATE_HPP

#include <string>

namespace kikotsoka {

    class State {
    public:
        enum Values {
            VACANT = 0,
            WHITE = 1,
            BLACK = 2,
            BLOCK = 3,
            BLOCK_TMP = 4
        };

    };

}

#endif