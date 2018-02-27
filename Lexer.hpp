// Lexer.hpp
#ifndef BLXCPP_LEXER_HPP
#define BLXCPP_LEXER_HPP

#include <string>
#include <regex>
#include <functional>
#include <vector>
#include <map>
#include <stdexcept>

/*
 *
 * 重新整理一下设计思路
 *
 * 一、定义状态
 * 接受状态由状态字符串类型决定，不过首先需要定义接受的状态，和初始状态
 *
 * 二、定义规则，规则分成三部分 {接受的状态, 正则, 处理函数}
 * 正则部分由一个正则字符串来代替，可以用 R"(<正则字符串>)" 解决
 * 接受状态和处理函数都可以为空，为空则自动返回原状态和空token
 * 处理函数接受一个参数，捕获的字符串，返回一个捕获的状态。
 *
 * 三、定义 Token
 * Token { name: string, text: string }
 *
 */

namespace blxcpp {

// 默认状态，就相当于没有状态咯
enum LexerDefaultState { NORMAL = 0 }; // 0 为默认的初始化状态

// 默认 Token ，Token 必须要有 name 属性和 EOF_TOKEN 静态属性
struct LexerDefaultToken { std::string name; std::string text; const static LexerDefaultToken EOF_TOKEN; }; // name 为必须属性
const LexerDefaultToken LexerDefaultToken::EOF_TOKEN = {"EOF", ""};

// Lexer 主体
template<typename StateT = LexerDefaultState, typename TokenT = LexerDefaultToken>
class Lexer {
public:

    using State = StateT;
    using Token = TokenT;
    using TokenBuffer = std::vector<Token>;
    const static State init_state = static_cast<State>(0);

    struct Rule;
    using RuleBook = std::map<State, std::vector<Rule>>;

    struct Matched;

private:

public:
};

// 由处理函数捕获到的结果，包含状态转移和捕获 token
template<typename StateT, typename TokenT>
struct Lexer<StateT, TokenT>::Matched {
    State next_state = static_cast<State>(-1);
    TokenBuffer token_buffer;

    // 各种隐式转换函数，方便写返回函数
    Matched(State next_state, TokenBuffer token_buffer)
        : Matched(next_state, token_buffer) { }

    Matched(TokenBuffer token_buffer)
        : token_buffer(token_buffer) { }

    Matched(State next_state, Token token)
        : Matched(next_state, TokenBuffer {token}) { }

    Matched(Token token)
        : Matched(TokenBuffer {token}) { }

    Matched(State next_state)
        : Matched(next_state, TokenBuffer { }) { }

    Matched()
        : Matched(TokenBuffer { }) { }
};

// 规则
template<typename StateT, typename TokenT>
struct Lexer<StateT, TokenT>::Rule {
    using Handler =std::function<Matched(std::string)>;

    State state;
    std::regex regex;
    std::string regex_str;
    Handler handler;

    static Matched empty_handler(std::string) {
        return Matched();
    }

    // 同样各种隐式转换来方便书写规则
    Rule(State state, std::string regex_str, Handler handler)
        : state(state), regex("^(" + regex_str + ")"), regex_str(regex_str), handler(handler) { }

    Rule(std::string regex_str, Handler handler)
        : Rule(init_state, regex_str, handler) { }

    Rule(std::string regex_str)
        : Rule(init_state, regex_str, &empty_handler) { }
};

}

//namespace blxcpp {

//template<typename, typename StateT, StateT>
//class Lexer;


//struct LexerDefaultToken {
//    // 必须要有一个 name，用来做之后的语法分析
//    using Name = std::string;
//    Name name;

//    // col / line / length 需要用来装 token的位置信息用于报错
//    // 事实上主要是我懒得做成流式的了，麻烦，所以偷懒把所有信息装到 token 里面完事
//    size_t col, line, length;

//    // 必须要有一个 EOF TOKEN，不用 EOF 命名是因为 EOF 被不知道哪里的宏占用了
//    static const LexerDefaultToken EOF_TOKEN;

//    // 以下都是非必须的了需要怎么样都无所谓啦~
//    using Text = std::string;
//    Text text;

//    inline LexerDefaultToken(const char* name)
//        : name(name) { }

//    inline LexerDefaultToken(Name name)
//        : name(name) { }

//    inline LexerDefaultToken(Name name, Text text)
//        : name(name), text(text) { }
//};

// 预定一个 EOF_TOKEN 用作之后方便添加以及比较 EOF TOKEN
//const LexerDefaultToken LexerDefaultToken::EOF_TOKEN = "EOF";

// 正式开始写 Lexer 啦
//template<class TokenT = LexerDefaultToken, typename StateT = LexerDefaultState, StateT InitState = NORMAL>
//class Lexer {
//public:

//    // 导出 Toekn / State 的 type 方便外部访问
//    using Token = TokenT;
//    using State = StateT;
//    using TokenStream = std::vector<Token>;

//    // 初始状态 / 匹配参数
//    const static State init_state = InitState;

//    // 用来标记下一个状态
//    struct Matched {
//    private:
//        // 用来初始化的
//        Matched(bool is_changed, State next_state, TokenStream token_stream)
//            : is_changed(is_changed), next_state(next_state), token_stream(token_stream) { }

//    public:
//        bool is_changed;
//        State next_state;
//        TokenStream token_stream;

//        // 各种隐式转换函数，方便写返回函数
//        Matched(State next_state, TokenStream token_stream)
//            : Matched(true, next_state, token_stream) { }

//        Matched(TokenStream token_stream)
//            : Matched(false, State(), token_stream) { }

//        Matched(State next_state, Token token)
//            : Matched(next_state, TokenStream {token}) { }

//        Matched(Token token)
//            : Matched(TokenStream {token}) { }

//        Matched(State next_state)
//            : Matched(next_state, TokenStream { }) { }

//        Matched()
//            : Matched(TokenStream { }) { }
//    };

    // 用来记录匹配的规则
//    struct Rule {
//        using Handler =std::function<Matched(std::string)>;

//        State state;
//        std::regex regex;
//        std::string regex_str;
//        Handler handler;

//        static Matched empty_handler(std::string) {
//            return Matched();
//        }

//        // 同样各种隐式转换来方便书写规则
//        Rule(State state, std::string regex_str, Handler handler)
//            : state(state), regex("^(" + regex_str + ")"), regex_str(regex_str), handler(handler) { }

//        Rule(std::string regex_str, Handler handler)
//            : Rule(init_state, regex_str, handler) { }

//        Rule(std::string regex_str)
//            : Rule(init_state, regex_str, &empty_handler) { }
//    };

//    using RuleBook = std::map<State, std::vector<Rule>>;

//	struct ParserIterator {
//		begin
//	};

//private:
//    // 分词规则表
//    RuleBook rule_book;

//public:

//    // 构造，并且生成分词规则
//    Lexer(const std::vector<Rule>& rule_list) {
//        for (const Rule& rule : rule_list)
//            rule_book[rule.state].push_back(rule);
//    }

//    // 分词开始！
//    TokenStream parse(const std::string& input) {

//        std::regex eol(R"(\n)");
//        std::regex_constants::match_flag_type eol_flag =
//                std::regex_constants::match_default | std::regex_constants::match_any;

//        // 初始化
//        TokenStream token_stream;
//        State state = init_state;
//        std::string::const_iterator it = input.begin();
//        std::string::const_iterator end = input.end();

//        size_t col = 1, line = 1;

//        // 进入分词循环
//        while (it != end) {

//            // 获取当前状态对应的分词规则
//            std::vector<Rule> rules = rule_book[state];
//            bool is_matched = false;

//            // 遍历规则
//            std::smatch matched_result;
//            for (const Rule& rule : rules) {
//                if (std::search(it, end, matched_result, rule.regex)) {

//                    std::string matched_text = matched_result[0];

//                    // 不允许匹配一个空字符串
//                    if (matched_text.length() == 0)
//                        throw std::logic_error(
//                                "[RuleError] On rule \""
//                                + rule.regex_str
//                                + "\", rule does not allow to match empty string.");

//                    // 执行规则对应的处理函数
//                    Matched matched = rule.handler(matched_text);

//                    // 转换状态以及获取 token
//                    if (matched.is_changed) state = matched.next_state;
//                    for (Token& token : matched.token_stream) {
//                        token.col = col;
//                        token.line = line;
//                        token.length = matched_text.length();
//                        token_stream.push_back(token);
//                    }

//                    // 为下一次循环准备
//                    it += static_cast<int>(matched_text.length());
//                    is_matched = true;

//                    // 积累行列信息
//                    std::smatch matched_new_line;
//                    if (std::regex_search(matched_text, matched_new_line, eol, eol_flag)) {
//                        col = 1;
//                        line += matched_new_line.size();
//                    } else  {
//                        col += matched_text.length();
//                    }

//                    // 进入下一次循环
//                    break;
//                }
//            }

//            if (!is_matched) {
//                // 这里要报词法错误
//                throw std::logic_error("[LexerError] Line: " + std::to_string(line) + ", Column: " + std::to_string(col) + ".");
//            }
//        }

//        // 推入 EOF 然后债见
//        token_stream.push_back(Token::EOF_TOKEN);
//        return token_stream;
//    }

//};

//}

#endif // BLXCPP_LEXER_HPP
