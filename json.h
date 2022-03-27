#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Data = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {

    public:
        using variant::variant;
        using Data = variant;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        double AsDouble() const;
        int AsInt() const;
        bool AsBool() const;
        const std::string& AsString() const;

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsBool() const;
        bool IsMap() const;

        friend bool operator==(const Node& left, const Node& right);
        friend bool operator!=(const Node& left, const Node& right);

        Data GetData() const {
            return *this;
        }
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& left, const Document& right);

    bool operator!=(const Document& left, const Document& right);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    void PrintNode(const Node& node, std::ostream& output);

}  // namespace json