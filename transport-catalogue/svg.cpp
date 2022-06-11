#include "svg.h"

namespace svg {

using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, Rgb rgb){
        out << "rgb("sv << unsigned(rgb.red) << ","sv
            << unsigned(rgb.green) << ","sv
            << unsigned(rgb.blue) << ")"sv;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Rgba rgba){
        out << "rgba("sv << unsigned(rgba.red) << ","sv
            << unsigned(rgba.green) << ","sv
            << unsigned(rgba.blue) << ","sv
            << rgba.opacity << ")"sv;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap) {
            case StrokeLineCap::BUTT: {
                out << "butt"sv;
                break;
            }
            case StrokeLineCap::ROUND: {
                out << "round"sv;
                break;
            }
            case StrokeLineCap::SQUARE: {
                out << "square"sv;
                break;
            }
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join) {
            case StrokeLineJoin::ARCS: {
                out << "arcs"sv;
                break;
            }
            case StrokeLineJoin::BEVEL: {
                out << "bevel"sv;
                break;
            }
            case StrokeLineJoin::MITER: {
                out << "miter"sv;
                break;
            }
            case StrokeLineJoin::MITER_CLIP: {
                out << "miter-clip"sv;
                break;
            }
            case StrokeLineJoin::ROUND: {
                out << "round"sv;
                break;
            }
        }
        return out;
    }
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.emplace_back(std::unique_ptr<Object> (std::move(obj)));
    }

    void Document::Render(std::ostream &out) const {
        RenderContext ctx(out, 2, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

// ---------- Text ------------------

    Text::Text()
    : pos_(0.0, 0.0)
    , offset_(0.0, 0.0)
    , size_(1) {}
    
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text &Text::SetData(std::string data) {
        Encoding(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" "sv << "y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;
        out << data_;
        out << "</text>"sv;
    }
    
    void Text::Encoding(std::string data) {
        for (char c : data) {
            std::string result;
            switch (c) {
                case '<' :
                    result = "&lt;"sv;
                    break;
                case '>' :
                    result = "&gt;"sv;
                    break;
                case '"':
                    result = "&quot;"sv;
                    break;
                case '&':
                    result = "&amp;"sv;
                    break;
                case '\'':
                    result = "&apos;"sv;
                    break;
                default:
                    result = c;
                    break;
            }
            data_ += result;
        }
    }

// ---------- Polyline ------------------
    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (Point point : points_) {
            if (first) {
                out << point.x << "," << point.y;
                first = false;
            } else {
                out << " "sv << point.x << "," << point.y;
            }
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Rgb::Rgb()
    : red(0)
    , green(0)
    , blue(0)
    {}

    Rgb::Rgb(uint8_t red_input, uint8_t green_input, uint8_t blue_input)
    : red(red_input)
    , green(green_input)
    , blue(blue_input)
    {}

    Rgba::Rgba()
    :Rgb()
    , opacity(1.0)
    {}

    Rgba::Rgba(uint8_t red_input, uint8_t green_input, uint8_t blue_input, double opacity_input)
    : Rgb( red_input, green_input, blue_input)
    , opacity(opacity_input)
    {}


}  // namespace svg