#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    //inline std::string_view LRstrip(std::string_view line);

    std::string_view Lstrip(std::string_view line);
    std::string_view Rstrip(std::string_view line);

    using Number = std::variant<int, double>;

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeValue = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent(int step = 0) const {
            for (int i = 0; i < indent + step; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    class Node {
    public:

        bool IsInt() const { return std::holds_alternative<int>(value_); }
        //Возвращает true, если в Node хранится int либо double.
        bool IsDouble() const { return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_); }
        //Возвращает true, если в Node хранится double.
        bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
        bool IsBool() const { return std::holds_alternative<bool>(value_); }
        bool IsString() const { return std::holds_alternative<std::string>(value_); }
        bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
        bool IsArray() const { return std::holds_alternative<Array>(value_); }
        bool IsMap() const { return std::holds_alternative<Dict>(value_); }

        Node(std::nullptr_t) {}
        Node() {}
        Node(const Array& array);
        Node(const Dict& map);
        Node(const int value);
        Node(const double value);
        Node(const bool value);
        Node(const std::string& value);

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        const std::string& AsString() const;
        const NodeValue& GetNodeValue() const { return value_; }
        bool operator==(const Node& other) const {
            return value_ == other.GetNodeValue();
        }
        bool operator!=(const Node& other) const {
            return value_ != other.GetNodeValue();
        }
    private:
        NodeValue value_;
    };

    class Document {
    public:
        explicit Document(const Node& root);

        const Node& GetRoot() const;
        bool operator==(const Document& other) { return root_ == other.root_; }
        bool operator!=(const Document& other) { return root_ != other.root_; }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    void PrintNode(std::ostream& output, const Dict& value);
    void PrintNode(std::ostream& output, const Array& value);

    void PrintNode(std::ostream& output, const std::nullptr_t&);
    void PrintNode(std::ostream& output, const int& value);
    void PrintNode(std::ostream& output, const double& value);
    void PrintNode(std::ostream& output, const bool& value);
    void PrintNode(std::ostream& output, const std::string& value);



}  // namespace json