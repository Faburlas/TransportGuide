#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <set>

#include "domain.h"
#include "geo.h"
#include "svg.h"

namespace render {
    struct Settings {
        double width = 0;
        double height = 0;
        double padding = 0;
        double line_width = 0;
        double stop_radius = 0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset;
        int stop_label_font_size = 0;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width = 0;
        std::vector<svg::Color> color_palette;
    };

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);
    svg::Point MakePoint(double x, double y);
    svg::Color MakeColor(const std::string& color);
    svg::Color MakeColor(int r, int g, int b);
    svg::Color MakeColor(int r, int g, int b, double a);

    class SphereProjector final {
    public:
        SphereProjector() = default;

        SphereProjector(const tg::detail::Coordinates& left_top, 
            const tg::detail::Coordinates& right_bottom, 
            double width, double height, double padding);

        svg::Point operator()(const tg::detail::Coordinates& coords) const;
    private:
        double padding_ = 0;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coef_ = 0;
    };


    class MapRenderer final {
    public:
        MapRenderer() = default;
        svg::Document GetDocument() const;
        void SetSettings(const Settings& settings);
        Settings GetSettings() const;
        void SetBorder(const Stops& stops);
        void SetBusRoute(const Buses& buses);
        void SetStation(const Stops& stops);

    private:
        SphereProjector sphere_projector_;
        Settings settings_;
        size_t index_color_ = 0;
        svg::Document document_;

        void RenderBusRoute(const Bus& bus);
        void RenderBusRouteName(const Bus& bus);
        void RenderStation(const Stop& stop);
        void RenderStationName(const Stop& stop);
        svg::Polyline CreateBusRoute(const Bus& bus) const;
        svg::Color GetColor();
        svg::Point GetPoint(const tg::detail::Coordinates& coords) const;
        size_t GetIndexColor();
    };
}