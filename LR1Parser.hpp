// LR1Parser.hpp
#ifndef BLXCPP_LR1PARSER_HPP
#define BLXCPP_LR1PARSER_HPP

#include "Lexer.hpp"

namespace blxcpp {

class ParserDefaultAST {
};

template<typename TokenT = LexerDefaultToken, typename ASTT = ParserDefaultAST>
class LR1Parser {
public:
    using Token = TokenT;
    using AST = ASTT;
    using Name = typename Token::Name;

private:

private:

public:
    LR1Parser() {
    }
};

}

#endif // BLXCPP_LR1PARSER_HPP
