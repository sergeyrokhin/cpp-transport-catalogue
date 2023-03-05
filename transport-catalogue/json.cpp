#include "json.h"

#include <exception>

using namespace std;

static constexpr int RESERVE_FOR_VALUE = 20;
static constexpr string_view NONE_TEXT = "null"sv;
static constexpr string_view TRUE_TEXT = "true"sv;
static constexpr string_view FALSE_TEXT = "false"sv;

namespace json {

	string_view Lstrip(string_view line) {
		while (!line.empty() && isspace(line[0])) {
			line.remove_prefix(1);
		}
		return line;
	}
	string_view Rstrip(string_view line) {
		auto end = line.size();
		while (!line.empty() && isspace(line[--end])) {
			line.remove_suffix(1);
		}
		return line;
	}


	Node LoadNode(istream& input);

	Number LoadNumber(std::istream& input) {
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
		}
		else {
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
					return std::stoi(parsed_num);
				}
				catch (...) {
					// В случае неудачи, например, при переполнении,
					// код ниже попробует преобразовать строку в double
				}
			}
			return std::stod(parsed_num);
		}
		catch (...) {
			throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
		}
	}

	// Считывает содержимое строкового литерала JSON-документа
	// Функцию следует использовать после считывания открывающего символа ":
	std::string LoadString(std::istream& input) {
		using namespace std::literals;

		auto it = std::istreambuf_iterator<char>(input);
		auto end = std::istreambuf_iterator<char>();
		std::string s;
		while (true) {
			if (it == end) {
				// Поток закончился до того, как встретили закрывающую кавычку?
				throw ParsingError("String parsing error");
			}
			const char ch = *it;
			if (ch == '"') {
				// Встретили закрывающую кавычку
				++it;
				break;
			}
			else if (ch == '\\') {
				// Встретили начало escape-последовательности
				++it;
				if (it == end) {
					// Поток завершился сразу после символа обратной косой черты
					throw ParsingError("String parsing error");
				}
				const char escaped_char = *(it);
				// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
				switch (escaped_char) {
				case 'n':
					s.push_back('\n');
					break;
				case 't':
					s.push_back('\t');
					break;
				case 'r':
					s.push_back('\r');
					break;
				case '"':
					s.push_back('"');
					break;
				case '\\':
					s.push_back('\\');
					break;
				default:
					// Встретили неизвестную escape-последовательность
					throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
				}
			}
			else if (ch == '\n' || ch == '\r') {
				// Строковый литерал внутри- JSON не может прерываться символами \r или \n
				throw ParsingError("Unexpected end of line"s);
			}
			else {
				// Просто считываем очередной символ и помещаем его в результирующую строку
				s.push_back(ch);
			}
			++it;
		}

		return s;
	}

	Node LoadDict(istream& input) {
		// { уже считана Далее будет " ....." : NodeValue }
		Dict result;
		char c;
		for (; input >> c && c != '}';) {
			if (c == ',') input >> c;
			if (c != '\"')	throw ParsingError(" { - not found `\"`"s);
			string key = LoadString(input);
			input >> c;
			if (c != ':')	throw ParsingError(" { - not found `:`"s);
			result.insert({ move(key), LoadNode(input) });
		}
		if (c == '}')
			return Node(move(result));
		else
			throw ParsingError("not found }"s);
	}

	Node LoadArray(istream& input) {
		Array result;
		char c;
		for (; input >> c && c != ']';) {
			if (c != ',') input.putback(c); //положили обратно
			result.push_back(LoadNode(input));
		}
		if (c == ']')
			return Node(move(result));
		else
			throw ParsingError("not found ]"s);
	}

	Node LoadWord(istream& input) {

		using namespace std::literals;

		auto it = std::istreambuf_iterator<char>(input);
		auto end = std::istreambuf_iterator<char>();
		std::string s;
		while (true) {
			if (it == end) {
				break;
			}
			else {
				// Просто считываем очередной символ и помещаем его в результирующую строку
				const char ch = *it;
				if (ch == '}' || ch == ']' || ch == ',')
				{
					break;
				}
				s.push_back(ch);
			}
			++it;
		}

		auto value_sv = Rstrip(s); //теперь без пробелов в начале и в конце
		if (value_sv == NONE_TEXT)	return Node{ nullptr };
		if (value_sv == TRUE_TEXT)	return Node{ true };
		if (value_sv == FALSE_TEXT)	return Node{ false };
		throw ParsingError("not recognize none, true, false"s);
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
			return Node{ LoadString(input) };
		}
		else if (c == 'n' || c == 't' || c == 'f') {
			input.putback(c);
			return Node{ LoadWord(input) };
		}
		else {
			input.putback(c);
			auto result = LoadNumber(input);
			if (holds_alternative<double>(result))
			{
				return Node{ get<double>(result) };
			}
			return Node{ get<int>(result) };
		}

	}

	Node::Node(const Array& array) : value_(array) {}

	Node::Node(const Dict& map) : value_(map) {}

	Node::Node(const int value) : value_(value) {}

	Node::Node(const bool value) : value_(value) {}

	Node::Node(const double value) : value_(value) {}

	Node::Node(const string& value) : value_(value) {}

	const Array& Node::AsArray() const {
		if(holds_alternative<Array>(value_)) return get<Array>(value_);
		throw std::logic_error("not Array"s);
	}

	const Dict& Node::AsDict() const {
		if (holds_alternative<Dict>(value_)) return get<Dict>(value_);
		throw std::logic_error("not Dict"s);
	}

	bool Node::AsBool() const {
		if (holds_alternative<bool>(value_)) return get<bool>(value_);
		throw std::logic_error("not Array"s);
	}


	int Node::AsInt() const {
		if (holds_alternative<int>(value_)) return get<int>(value_);
			throw std::logic_error("not int"s);
	}

	double Node::AsDouble() const {
		if (holds_alternative<double>(value_)) return get<double>(value_);
		return AsInt();
	}

	const string& Node::AsString() const {
		if (holds_alternative<string>(value_)) return get<string>(value_);
		throw std::logic_error("not string"s);
	}


	Document::Document(const Node& root)
		: root_(root) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void PrintNode(PrintContext& ctx, const std::nullptr_t&) {
		ctx.out << NONE_TEXT;
	}
	void PrintNode(PrintContext& ctx, const int& value) {
		ctx.out << value;
	}
	void PrintNode(PrintContext& ctx, const double& value) {
		ctx.out << value;
	}
	void PrintNode(PrintContext& ctx, const bool& value) {
		ctx.out << (value ? TRUE_TEXT : FALSE_TEXT);
	}
	void PrintNode(PrintContext& ctx, const string& value) {
		ctx.out << '\"';
		for (auto c : value) {
			switch (c)
			{
			case '"':
				ctx.out << "\\\""sv;
				break;
			case '\\':
				ctx.out << "\\\\"sv;
				break;
			case '\n':
				ctx.out << "\\n"sv;
				break;
			case '\r':
				ctx.out << "\\r"sv;
				break;
			default:
				ctx.out << c;
				break;
			}
		}
		ctx.out << '\"';
	}

	void PrintNode(PrintContext& ctx, const Array& value) {
		bool not_first = false;
		ctx.PrintIndent();
		ctx.out << '[' << ' ';
		for (auto& elem : value) {
			if (not_first) ctx.out << ", ";
			ctx.PrintIndent(2);
			not_first = true;
			visit(
				[&ctx](auto value) {
					auto next_context = ctx.Indented();
			PrintNode(next_context, value);
				}, elem.GetNodeValue());
		}
		ctx.PrintIndent();
		ctx.out << "]";
	}

	void PrintNode(PrintContext& ctx, const Dict& value) {
		bool not_first = false;
		ctx.PrintIndent();
		ctx.out << '{' << ' ';
		for (auto& elem : value) {
			if (not_first) ctx.out << ", ";
			ctx.PrintIndent(2);
			not_first = true;
			ctx.out << '\"' << elem.first << "\": ";
			visit(
				[&ctx](auto value) {
					auto next_context = ctx.Indented();
			PrintNode(next_context, value);
				}, elem.second.GetNodeValue());
		}
		ctx.PrintIndent();
		ctx.out << "}";
	}

	void Print(const Document& doc, std::ostream& output) {
		visit(
			[&output](auto value) {
				// Это универсальная лямбда-функция (generic lambda).
				// Внутри неё нужная функция PrintNode будет выбрана за счёт перегрузки функций.
				auto next_context = PrintContext{ output };
				PrintNode(next_context, value);
			}, doc.GetRoot().GetNodeValue());
	}

}  // namespace json