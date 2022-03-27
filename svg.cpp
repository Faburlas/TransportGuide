#include "svg.h"

#include <sstream>

namespace svg {

    using namespace std::literals;

    Rgb::Rgb(uint8_t red_, uint8_t green_, uint8_t blue_) :
        red(red_), green(green_), blue(blue_)
    {}

    Rgba::Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_) :
        red(red_), green(green_), blue(blue_), opacity(opacity_)
    {}

    // ---------- Overloading SLC & SLJ ------------------

    std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& stroke_line_cap) {
        using namespace std::string_view_literals;
        switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            stream << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            stream << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            stream << "square"sv;
            break;
        }
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const StrokeLineJoin& stroke_line_join) {
        using namespace std::string_view_literals;
        switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            stream << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            stream << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            stream << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            stream << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            stream << "round"sv;
            break;
        }
        return stream;
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
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
        out << "<circle cx=\""s << center_.x << "\" cy=\""s << center_.y << "\" "s;
        out << "r=\""s << radius_ << "\" "s;
        RenderAttrs(out);
        out << "/>"s;
    }

    //---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""s;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ',' << points_[i].y;
            if (i != points_.size() - 1) {
                out << " "s;
            }
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>"s;
    }

    //---------- Text ------------------

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"s;
        RenderAttrs(out);
        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }

        out << '>';
        for (const auto& symbol : data_) {
            switch (symbol)
            {
            case '"':
                out << "&quot;";
                break;
            case '\'':
                out << "&apos;";
                break;
            case '<':
                out << "&lt;";
                break;
            case '>':
                out << "&gt;";
                break;
            case '&':
                out << "&amp;";
                break;
            default:
                out << symbol;
                break;
            }
        }
        out << "</text>"s;
    }

    //---------- Document ------------------
    void Document::AddPtr(std::shared_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"s << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"s << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg