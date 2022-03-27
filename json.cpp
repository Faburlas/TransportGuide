#include "json.h"

#include <unordered_map>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c = '!';
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Miss ']' at the end");
            }
            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number"s);
                }
            };

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
            //Parse the integer part of the numbers
            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            //Parse the fractional part of a number
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            //Parse the exponential part of a number
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
                    //try string to int
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // if cant, try string to double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            static const std::unordered_map<char, char> escape_sequences{
                    {'a','\a'}, {'b','\b'}, {'f','\f'}, {'n','\n'},
                    {'r','\r'}, {'t','\t'}, {'v','\v'}, {'\'','\''},
                    {'\"','\"'}, {'\\','\\'}
            };
            string str;
            bool escaped = false;
            char c;
            for (; input.get(c) && !(!escaped && c == '\"');) {
                if (!escaped) {
                    if (c == '\\') {
                        escaped = true;
                    }
                    else {
                        str += c;
                    }
                }
                else {
                    if (const auto it = escape_sequences.find(c); it != escape_sequences.end()) {
                        str += it->second;
                    }
                    else {
                        throw ParsingError("Uncorrect escape sequence"s + std::to_string(c));
                    }
                    escaped = false;
                }
            }
            if ((escaped) || (c != '\"')) {
                throw ParsingError("Failed to read string"s);
            }
            return Node(move(str));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c = '!'; // Initialize to compare with '}'
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') {
                throw ParsingError("Parse error");
            }
            return Node(move(result));
        }

        Node LoadNullOrBool(istream& input) {
            std::string result;
            size_t size;
            if (input.peek() == 'f') {
                size = 5;
            }
            else {
                size = 4;
            }
            char c;
            for (size_t i = 0; i < size && input >> c; ++i) {
                result += c;
            }
            if (result == "null"s) {
                return { nullptr };
            }
            else if (result == "true"s) {
                return { true };
            }
            else if (result == "false"s) {
                return { false };
            }
            else {
                throw ParsingError("Failed to read null or bool");
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n' || c == 't' || c == 'f') {
                input.putback(c);
                return LoadNullOrBool(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    //As*Type* return Node
    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(*this);
        }
        else {
            throw invalid_argument("Received non array value in 'AsArray'");
        }
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return get<Dict>(*this);
        }
        else {
            throw invalid_argument("Received non dict value in 'AsMap'");
        }
    }

    const std::string& Node::AsString() const {
        if (IsString()) {
            return get<string>(*this);
        }
        else {
            throw invalid_argument("Received non string value in 'AsString'");
        }
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(*this);
        }
        else {
            throw invalid_argument("Received non int value in 'AsInt'");
        }
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return get<double>(*this);
        }
        else if (IsInt()) {
            return get<int>(*this);
        }
        else {
            throw invalid_argument("Received non double value in 'AsDouble'");
        }
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return get<bool>(*this);
        }
        else {
            throw invalid_argument("Received non bool value in 'AsBool'");
        }
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    // Is*Type* checkers
    bool Node::IsNull() const {
        return std::holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(*this) || IsInt();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    //operators == and != for Node And Document
    bool operator==(const Node& left, const Node& right) {
        return left.GetData() == right.GetData();
    }

    bool operator!=(const Node& left, const Node& right) {
        return !(left == right);
    }

    bool operator==(const Document& left, const Document& right) {
        return left.GetRoot() == right.GetRoot();
    }

    bool operator!=(const Document& left, const Document& right) {
        return !(left == right);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintNode(const Node& node, std::ostream& output);

    //operator() for different types.
    struct NodePrint {
        std::ostream& out;
        void operator()(std::nullptr_t) const {
            out << "null"s;
        }
        void operator()(Array array) const {
            int size = array.size() - 1;
            out << "[\n";
            for (const Node& node : array) {
                PrintNode(node, out);
                if (size > 0) {
                    out << ",\n";
                    --size;
                }
            }
            out << "\n]";
        }
        void operator()(Dict dict) const {
            int size = dict.size() - 1;
            out << "{\n";
            for (const auto& [key, node] : dict) {
                out << '"' << key << "\": ";
                PrintNode(node, out);
                if (size > 0) {
                    out << ",\n";
                    --size;
                }
            }
            out << "\n}";
        }
        void operator()(int integer) const {
            out << integer;
        }
        void operator()(double real) const {
            out << real;
        }
        void operator()(const std::string& str) const {
            out << '"';
            for (const auto& symbol : str) {
                if (symbol == '\"') {
                    out << '\\' << '\"';
                }
                else if (symbol == '\\') {
                    out << '\\' << '\\';
                }
                else if (symbol == '\n') {
                    out << '\\' << 'n';
                }
                else {
                    out << symbol;
                }
            }
            out << '"';
        }
        void operator()(bool boolean) const {
            if (boolean) {
                out << "true"s;
            }
            else {
                out << "false"s;
            }
        }
    };

    void PrintNode(const Node& node, std::ostream& output) {
        visit(NodePrint{ output }, node.GetData());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json