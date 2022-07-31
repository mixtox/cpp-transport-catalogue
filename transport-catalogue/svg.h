#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include <vector>
#include <optional>
#include <variant>

namespace svg {

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

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
        return {out, indent_step, indent + indent_step};
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

class Rgb {
public:
    Rgb();
    Rgb(uint8_t red_input, uint8_t green_input, uint8_t blue_input);

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

class Rgba final : public Rgb {
public:
    Rgba();
    Rgba(uint8_t red_input, uint8_t green_input, uint8_t blue_input, double opacity_input);

    double opacity = 0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{ "none" };

std::ostream& operator<<(std::ostream& out, Rgb rgb);
std::ostream& operator<<(std::ostream& out, Rgba rgba);

struct PrintColor {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none";
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb rgb) const {
        out << rgb;
    }
    void operator()(Rgba rgba) const {
        out << rgba;
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
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeWidth(double stroke_width) {
        stroke_width_ = stroke_width;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(PrintColor{out}, fill_color_.value());
            out << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(PrintColor{out}, stroke_color_.value());
            out << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineJoin> line_join_;
    std::optional<StrokeLineCap> line_cap_;
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle>{
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
class Polyline final : public Object, public PathProps<Polyline>{
public:
// Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text>{
public:
    Text();
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

private:
    void RenderObject(const RenderContext& context) const override;
    void Encoding(std::string data);

    Point pos_;
    Point offset_;
    uint32_t size_;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;

};


class ObjectContainer {
public:

    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }
    // Добавляет в svg-документ объект-наследник svg::Object
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    ~ObjectContainer() = default;

};

class Drawable {
public:
    virtual void Draw(svg::ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;
};



}  // namespace svg
