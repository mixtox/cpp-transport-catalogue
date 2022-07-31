#include "map_renderer.h"

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

    void MapRenderer::SetRendererSettings(RenderSettings& settings) {
        render_settings_= std::move(settings);
    }

    svg::Document MapRenderer::GetSVG(const std::map<std::string_view, domain::BusPtr> &routes) const {

        using namespace std::string_literals;
        svg::Document result;
        std::map<std::string_view, domain::StopPtr> unique_stop;

        for (const auto& [key, route] : routes) {
            for (const auto stop : route->bus_stops) {
                unique_stop[stop->name] = stop;
            }
        }

        std::vector<geo::Coordinates> stop_coordinates;

        for (const auto [key, stop] : unique_stop) {
            stop_coordinates.push_back(stop->coordinates);
        }

        SphereProjector projector(stop_coordinates.begin(),
                                  stop_coordinates.end(),
                                  render_settings_.width,
                                  render_settings_.height,
                                  render_settings_.padding);

        size_t counter = 0;
        for (const auto [key, route] : routes) {
            if (route->bus_stops.empty()) {
                continue;
            }

            std::vector<domain::StopPtr> stop_vector {route->bus_stops.begin(), route->bus_stops.end()};
            if (!route->is_circle) {
                stop_vector.insert(stop_vector.end(), std::next(route->bus_stops.rbegin()), route->bus_stops.rend());
            }

            size_t color_index = counter % render_settings_.color_palette.size();
            svg::Polyline route_line;
            route_line.SetFillColor(svg::NoneColor).
                        SetStrokeColor(render_settings_.color_palette[color_index]).
                        SetStrokeWidth(render_settings_.line_width).
                        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            for (const auto& stop : stop_vector) {
                route_line.AddPoint(projector(stop->coordinates));
            }

            result.Add(route_line);
            ++counter;
        }

        counter = 0;
        for (const auto [key, route] : routes) {
            if (route->bus_stops.empty()) {
                continue;
            }
            size_t color_index = counter % render_settings_.color_palette.size();

            svg::Text underlayer_text;
            underlayer_text.SetData(route->bus_name).
                            SetPosition(projector(route->bus_stops.front()->coordinates)).
                            SetOffset(render_settings_.bus_label_offset).
                            SetFillColor(render_settings_.underlayer_color).
                            SetStrokeColor(render_settings_.underlayer_color).
                            SetFontFamily("Verdana"s).
                            SetFontSize(render_settings_.bus_label_font_size).
                            SetFontWeight("bold"s).
                            SetStrokeWidth(render_settings_.underlayer_width).
                            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            svg::Text route_name;
            route_name.SetData(route->bus_name).
                        SetPosition(projector(route->bus_stops.front()->coordinates)).
                        SetOffset(render_settings_.bus_label_offset).
                        SetFontFamily("Verdana"s).
                        SetFontSize(render_settings_.bus_label_font_size).
                        SetFontWeight("bold"s).
                        SetFillColor(render_settings_.color_palette[color_index]);

            result.Add(underlayer_text);
            result.Add(route_name);

            if (!route->is_circle && route->bus_stops.front() != route->bus_stops.back()) {
                result.Add(underlayer_text.SetPosition(projector(route->bus_stops.back()->coordinates)));
                result.Add(route_name.SetPosition(projector(route->bus_stops.back()->coordinates)));
            }
            ++counter;
        }

        for (const auto& [key, stop] : unique_stop ) {
            svg::Circle stop_circle;
            stop_circle.SetCenter(projector(stop->coordinates)).
                        SetRadius(render_settings_.stop_radius).
                        SetFillColor("white"s);
            result.Add(stop_circle);
        }

        for (const auto& [key, stop] : unique_stop ) {
            svg::Text underlayer_text;
            underlayer_text.SetData(stop->name).
                    SetPosition(projector(stop->coordinates)).
                    SetOffset(render_settings_.stop_label_offset).
                    SetFontSize(render_settings_.stop_label_font_size).
                    SetFontFamily("Verdana"s).
                    SetFillColor(render_settings_.underlayer_color).
                    SetStrokeColor(render_settings_.underlayer_color).
                    SetStrokeWidth(render_settings_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                    SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            svg::Text route_name;
            route_name.SetData(stop->name).
                    SetPosition(projector(stop->coordinates)).
                    SetOffset(render_settings_.stop_label_offset).
                    SetFontSize(render_settings_.stop_label_font_size).
                    SetFontFamily("Verdana"s).
                    SetFillColor("black"s);

            result.Add(underlayer_text);
            result.Add(route_name);
        }

        return result;
    }

    const RenderSettings MapRenderer::GetRenderSetting() const {
        return render_settings_;
    }

}
