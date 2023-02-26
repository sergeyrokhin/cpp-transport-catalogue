#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <variant>

namespace svg {

	using namespace std::literals;

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
		NONE
	};

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
		NONE
	};

	struct Rgb {
		Rgb() {};
		Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {};
		uint8_t red = 0, green = 0, blue = 0;
	};
	struct Rgba {
		Rgba() {};
		Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : red(red), green(green), blue(blue), opacity(opacity) {};
		uint8_t red = 0, green = 0, blue = 0; double opacity = 1;
	};

	using Color = std::variant <std::monostate, std::string, Rgb, Rgba>;

	std::ostream& operator<< (std::ostream& out, const StrokeLineCap line_cap);
	std::ostream& operator<< (std::ostream& out, const StrokeLineJoin line_join);

	std::ostream& operator<< (std::ostream& out, const Color color);

	// Объявив в заголовочном файле константу со спецификатором inline,
	// мы сделаем так, что она будет одной на все единицы трансляции,
	// которые подключают этот заголовок.
	// В противном случае каждая единица трансляции будет использовать свою копию этой константы
	inline const Color NoneColor = "none"s;

	struct ColorPrinter {
		std::ostream& out;

		void operator()(std::monostate) const {
			//out << "none"sv;
		}
		void operator()(std::string color) const {
			out << color;
		}
		void operator()(Rgb color) const {
			out << "rgb("sv << unsigned(color.red) << ',' << unsigned(color.green) << ',' << unsigned(color.blue) << ')';
		}
		void operator()(Rgba color) const {
			out << "rgba("sv << unsigned(color.red) << ',' << unsigned(color.green) << ',' << unsigned(color.blue) << ',' << color.opacity << ')';
		}
	};

	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color) {
			fill_color_ = std::move(color);
			return AsOwner();
		}
		Owner& SetStrokeColor(Color color) {
			stroke_color_ = std::move(color);
			return AsOwner();
		}
		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			line_cap_ = line_cap;
			return AsOwner();
		}
		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			line_join_ = line_join;
			return AsOwner();
		}
		Owner& SetStrokeWidth(double width) {
			width_ = width;
			return AsOwner();
		}

	protected:
		~PathProps() = default;

		void RenderAttrs(std::ostream& out) const {

			if (fill_color_.index() != 0)
			{
				out << " fill="sv << '\"' << fill_color_ << '\"';
			}
			if (stroke_color_.index() != 0)
			{
				out << " stroke="sv << '\"' << stroke_color_ << '\"';
			}

			if (width_) out << " stroke-width=\""sv << width_ << "\""sv;
			if (line_cap_ != StrokeLineCap::NONE)
			{
				out << " stroke-linecap=\""sv << line_cap_ << "\""sv;;

			}
			if (line_join_ != StrokeLineJoin::NONE)
			{
				out << " stroke-linejoin=\""sv << line_join_ << "\""sv;;

			}
		}

	private:
		Owner& AsOwner() {
			// static_cast безопасно преобразует *this к Owner&,
			// если класс Owner — наследник PathProps
			return static_cast<Owner&>(*this);
		}
		Color fill_color_;
		Color stroke_color_;
		StrokeLineCap line_cap_ = StrokeLineCap::NONE;
		StrokeLineJoin line_join_ = StrokeLineJoin::NONE;
		double width_ = 0;
	};

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	/*
	 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
	 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
	 */
	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	/*
	 * Абстрактный базовый класс Object служит для унифицированного хранения
	 * конкретных тегов SVG-документа
	 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
	 */
	class Object {
	public:
		virtual void MapRender(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	/*
	 * Класс Circle моделирует элемент <circle> для отображения круга
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public svg::PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline final : public Object, public svg::PathProps<Polyline> {
	public:
		// Добавляет очередную вершину к ломаной линии
		Polyline& AddPoint(Point point);
	private:
		void RenderObject(const RenderContext& context) const override;

		std::deque<Point> points_;
	};

	/*
	 * Класс Text моделирует элемент <text> для отображения текста
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text final : public Object, public svg::PathProps<Text> {
	public:

		// Задаёт координаты опорной точки (атрибуты x и y)
		Text& SetPosition(Point pos);

		// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
		Text& SetOffset(Point offset);

		// Задаёт размеры шрифта (атрибут font-size)
		Text& SetFontSize(uint32_t size);

		// Задаёт название шрифта (атрибут font-family)
		Text& SetFontFamily(std::string font_family);

		// Задаёт толщину шрифта (атрибут font-weight)
		Text& SetFontWeight(std::string font_weight);

		// Задаёт текстовое содержимое объекта (отображается внутри тега text)
		Text& SetData(std::string data);

		// Прочие данные и методы, необходимые для реализации элемента <text>

	private:
		void RenderObject(const RenderContext& context) const override;
	private:
		Point pos_;
		Point offset_;
		uint32_t size_ = 1;
		std::string font_family_;
		std::string font_weight_;
		std::string data_;
	};

	struct ObjectContainer {
	public:
		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

		template <typename Obj>
		void Add(Obj obj) {
			AddPtr(std::make_unique<Obj>(std::move(obj)));
		}
	protected:
		~ObjectContainer() {}
	};

	struct Drawable {
	public:
		virtual void Draw(ObjectContainer& container) const = 0;
		
	protected:
		virtual ~Drawable() = default;
	};


	class Document : public ObjectContainer {
	public:

		Document() = default;

		// Добавляет в svg-документ объект-наследник svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		// Выводит в ostream svg-представление документа
		void MapRender(const RenderContext& context) const;

		// Прочие методы и данные, необходимые для реализации класса Document
		std::deque<std::unique_ptr<Object>> objects_ptr_;
	};

}  // namespace svg