#include "svg.h"

namespace svg {

	using namespace std::literals;


	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

	//	context.out << std::endl;
	}

	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\""sv;
		RenderAttrs(out);
		out << "/>"sv;
	}

	// ---------- Polyline ------------------
	Polyline& Polyline::AddPoint(Point point) {
		points_.push_back(point);
		return *this;
	}
	void Polyline::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<polyline points=\""sv;
		bool it_not_first = false;
		for (auto& point : points_)
		{
			if (it_not_first) out << ' ';
			out << point.x << ","sv << point.y;
			it_not_first = true;
		}
		out << "\""sv;
		RenderAttrs(out);
		out << "/>"sv;
	}

	// ---------- Text ------------------



	void Text::RenderObject(const RenderContext& context) const {
		//if (data_.empty()) return;

		auto& out = context.out;
		out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
		out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
		out << "font-size=\""sv << size_ << "\""sv;
		if (!font_family_.empty()) out << " font-family=\""sv << font_family_ << "\""sv;
		if (!font_weight_.empty()) out << " font-weight=\""sv << font_weight_ << "\""sv;
		RenderAttrs(out);
		out << ">"sv;

		for (auto c : data_) {
			switch (c)
			{
			case '\"':
				out << "&quot;"sv;
				break;
			case '\'':
				out << "&apos;"sv;
				break;
			case '<':
				out << "&lt;"sv;
				break;
			case '>':
				out << "&gt;"sv;
				break;
			case '&':
				out << "&amp;"sv;
				break;
			default:
				out << c;
				break;
			}
		}
		out << "</text>"sv;
	}

	Text& Text::SetPosition(const Point& pos) {
		pos_ = pos;
		return *this;
	}

	// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
	Text& Text::SetOffset(const Point& offset) {
		offset_ = offset;
		return *this;
	}

	// Задаёт размеры шрифта (атрибут font-size)
	Text& Text::SetFontSize(uint32_t size) {
		size_ = size;
		return *this;
	}

	// Задаёт название шрифта (атрибут font-family)
	Text& Text::SetFontFamily(const std::string& font_family) {
		font_family_ = font_family;
		return *this;
	}

	// Задаёт толщину шрифта (атрибут font-weight)
	Text& Text::SetFontWeight(const std::string& font_weight) {
		font_weight_ = font_weight;
		return *this;
	}

	// Задаёт текстовое содержимое объекта (отображается внутри тега text)
	Text& Text::SetData(const std::string& data) {
		data_ = data;
		return *this;
	}

	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects_ptr_.push_back(move(obj));
	}

	void Document::MapRender(const RenderContext& context) const {
		context.out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
		context.out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

		for (auto& obj : objects_ptr_) {
			context.out << "  "sv;
			obj.get()->Render(context);
		}
		context.out << "</svg>"sv;
	}


	std::ostream& operator<<(std::ostream& output, StrokeLineCap line_cap) {
			switch (line_cap) {
			case svg::StrokeLineCap::BUTT:
				output << "butt"sv;
				break;
			case svg::StrokeLineCap::ROUND:
				output << "round"sv;
				break;
			case svg::StrokeLineCap::SQUARE:
				output << "square"sv;
				break;
			default:
				break;
			}
		return output;
	}



	std::ostream& operator<<(std::ostream& output, StrokeLineJoin line_join) {
			switch (line_join) {
			case svg::StrokeLineJoin::ARCS:
				output << "arcs"sv;
				break;
			case svg::StrokeLineJoin::BEVEL:
				output << "bevel"sv;
				break;
			case svg::StrokeLineJoin::MITER:
				output << "miter"sv;
				break;
			case svg::StrokeLineJoin::MITER_CLIP:
				output << "miter-clip"sv;
				break;
			case svg::StrokeLineJoin::ROUND:
				output << "round"sv;
				break;
			default:
				break;
			}
		return output;
	}

	std::ostream& operator<< (std::ostream& out, const Color& color) {
		visit(ColorPrinter{ out }, color);
		return out;
	}

}  // namespace svg