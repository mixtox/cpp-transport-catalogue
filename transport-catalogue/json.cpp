#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream &input);

        Node LoadArray(istream &input) {
            Array result;
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array Error!");
            }
            return Node(move(result));
        }

        Node LoadNumber(std::istream &input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream &input) {
            string result;
            int true_input = 1;
            while (input.peek() != EOF) {
                char c = input.get();
                if (c == '"') {
                    ++true_input;
                    break;
                }
                if (c == '\\') {
                    input.get(c);
                    switch (c) {
                        case '"':
                            result.push_back('"');
                            break;
                        case '\\':
                            result.push_back('\\');
                            break;
                        case 'n':
                            result.push_back('\n');
                            break;
                        case 'r':
                            result.push_back('\r');
                            break;
                        case 't':
                            result.push_back('\t');
                            break;
                        default:
                            throw ParsingError("invalid escape character!"s);
                    }
                } else {
                    result.push_back(c);
                }
            }
            if (true_input % 2 != 0) {
                throw ParsingError("Unpaired quotes!");
            }
            return Node(std::move(result));
        }

        Node LoadDict(istream &input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if (!input) {
                throw ParsingError("Dict Error!");
            }
            return Node(move(result));
        }

        Node LoadAlpha(istream &input) {
            string str;
            while (isalpha(input.peek())) {
                str.push_back(input.get());
            }
            if (str == "null"s) {
                return Node(nullptr);
            } else if (str == "true"s) {
                return Node(true);
            } else if (str == "false"s) {
                return Node(false);
            } else {
                throw ParsingError("Wrong input: "s);
            }
        }

        Node LoadNode(istream &input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Unexpected EOF"s);
            }
            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (isalpha(c)) {
                input.putback(c);
                return LoadAlpha(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

        struct NodePrint {
            ostream &out;

            void operator()(nullptr_t) const {
                out << "null"s;
            }

            void operator()(const Array &arr) const {
                out << "["s;
                bool first_step = true;
                for (auto &element: arr) {
                    if (first_step) {
                        first_step = false;
                    } else {
                        out << ", "s;
                    }
                    PrintNode(element, out);
                }
                out << "]"s;
            }

            void operator()(const Dict &dict) const {
                out << "{"s;
                bool first_step = true;
                for (auto&[key, value]: dict) {
                    if (first_step) {
                        first_step = false;
                    } else {
                        out << ", "s;
                    }
                    PrintNode(key, out);
                    out << ": "s;
                    PrintNode(value, out);
                }
                out << "}"s;
            }

            void operator()(bool value) const {
                out << (value ? "true"s : "false"s);
            }

            void operator()(int value) const {
                out << value;
            }

            void operator()(double value) const {
                out << value;
            }

            void operator()(string str) const {
                out << '\"';
                for (const char ch: str) {
                    switch (ch) {
                        case '\\':
                            out << '\\' << '\\';
                            break;
                        case '"':
                            out << '\\' << '"';
                            break;
                        case '\n':
                            out << '\\' << 'n';
                            break;
                        case '\r':
                            out << '\\' << 'r';
                            break;
                        case '\t':
                            out << '\\' << 't';
                            break;
                        default:
                            out << ch;
                    }
                }
                out << '\"';
            }
        };
    }  // namespace

    Document::Document(Node root)
            : root_(move(root)) {
    }

    const Node &Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document &rhs) {
        return (root_ == rhs.GetRoot());
    }

    bool Document::operator!=(const Document &rhs) {
        return !(root_ == rhs.GetRoot());
    }

    Document Load(istream &input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document &doc, std::ostream &output) {
        std::visit(NodePrint{output}, doc.GetRoot().GetValue());
    }

    void PrintNode(const Node &node, ostream &output) {
        std::visit(NodePrint{output}, node.GetValue());
    }

    Node::Node(Node::Value value) {
        this->swap(value);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return holds_alternative<Dict>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(*this);
    }

    const Array &Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("It is not Array!"s);
        }
        return get<Array>(*this);
    }

    const Dict Node::AsDict() const {
        if (!IsDict()) {
            throw logic_error("It is not Dict!"s);
        }
        return get<Dict>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("It is not bool!"s);
        }
        return get<bool>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("It is not int!"s);
        }
        return get<int>(*this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw logic_error("It is not double!"s);
        }
        if (IsPureDouble()) {
            return get<double>(*this);
        } else {
            return AsInt();
        }
    }

    const std::string Node::AsString() const {
        if (!IsString()) {
            throw logic_error("It is not string!"s);
        }
        return get<std::string>(*this);
    }

    const NodeType &Node::GetValue() const {
        return *this;
    }

    NodeType &Node::GetValue() {
        return *this;
    }

    bool Node::operator==(const Node &rhs) const {
        return (this->GetValue() == rhs.GetValue());
    }

    bool Node::operator!=(const Node &rhs) const {
        return !(this->GetValue() == rhs.GetValue());
    }


}  // namespace json
