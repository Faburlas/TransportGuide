#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <deque>

namespace svg {
    using namespace std::literals;

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& stroke_line_cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& stream, const StrokeLineJoin& stroke_line_join);

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_);
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_);
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    struct ColorPrintVariants {
        std::ostream& out;
        void operator()(std::monostate) const {
            out << "none"s;
        }
        void operator()(const std::string& color) const {
            out << color;
        }
        void operator()(Rgb color) const {
            out << "rgb("s << static_cast<unsigned>(color.red) << ',' << static_cast<unsigned>(color.green) << ',' << static_cast<unsigned>(color.blue) << ')';
        }
        void operator()(Rgba color) const {
            out << "rgba("s << static_cast<unsigned>(color.red) << ',' << static_cast<unsigned>(color.green) << ',' << static_cast<unsigned>(color.blue) << ',' << color.opacity << ')';
        }
    };


    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{ "none" };

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
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            // if smth = nullopt , don't output it
            if (fill_color_) {
                out << " fill=\""s;
                visit(ColorPrintVariants{ out }, *fill_color_);
                out << "\""s;
            }
            if (stroke_color_) {
                out << " stroke=\""s;
                visit(ColorPrintVariants{ out }, *stroke_color_);
                out << "\""s;
            }
            if (stroke_width_) {
                out << " stroke-width=\""s << *stroke_width_ << "\""s;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""s << *stroke_linecap_ << "\""s;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""s << *stroke_linejoin_ << "\""s;
            }
        }
    private:

        Owner& AsOwner() {
            // static_cast safely converts *this to Owner&,
            // if class Owner — inheritor PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };




    /*A helper structure that stores the context for displaying an indented SVG document
      Stores a reference to the output stream, the current value,
      and the indentation step when the element is output*/
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


    /* Abstruct base class Object */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj);
        virtual void AddPtr(std::shared_ptr<Object>&& obj) = 0;

    protected:
        virtual ~ObjectContainer() = default;
        std::deque<std::shared_ptr<Object>> objects_;
    };

    template <typename Obj>
    void
        ObjectContainer
        ::Add(Obj obj) {
        objects_.emplace_back(std::make_shared<Obj>(std::move(obj)));
    }


    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };



    /*
     * The Circle class models a <circle> element to display a circle
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;

    };

    /*
     * The Polyline class models a <polyline> element to display polylines
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * The Text class models the <text> element to display text
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        //Sets the coordinates of the reference point (attributes x and y)
        Text& SetPosition(Point pos);

        //Sets the offset relative to the reference point (attributes dx, dy)
        Text& SetOffset(Point offset);

        // Sets font sizes (font-size attribute)
        Text& SetFontSize(uint32_t size);

        // Sets the font name (font-family attribute)
        Text& SetFontFamily(std::string font_family);

        // Sets the font weight (font-weight attribute)
        Text& SetFontWeight(std::string font_weight);

        // Sets the text content of the object (displayed inside the text tag)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::string data_;
        std::string font_weight_;
        std::string font_family_;
        uint32_t size_ = 1;
        Point offset_ = { 0.0, 0.0 };
        Point pos_ = { 0.0, 0.0 };
    };

    class Document final : public ObjectContainer {
    public:
        void AddPtr(std::shared_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    };


}  // namespace svg