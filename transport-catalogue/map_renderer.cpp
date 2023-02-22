#include "map_renderer.h"
#include <algorithm>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace std;

namespace render {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    inline const string RENDER_SETTINGS_TEXT = "render_settings";
    inline const string RENDER_WIDTH_TEXT = "width";
    inline const string RENDER_HEIGHT_TEXT = "height";
    inline const string RENDER_PADDING_TEXT = "padding";
    inline const string RENDER_STOP_RA_TEXT = "stop_radius";
    inline const string RENDER_LINE_WI_TEXT = "line_width";

    inline const string RENDER_BUS_LABEL_F_S_TEXT = "bus_label_font_size";
    inline const string RENDER_BUS_LABEL_O_S_TEXT = "bus_label_offset";

    inline const string RENDER_STOP_LABEL_F_S_TEXT = "stop_label_font_size";
    inline const string RENDER_STOP_LABEL_O_S_TEXT = "stop_label_offset";
    inline const string RENDER_U_LAY_W_TEXT = "underlayer_width";

    inline const string RENDER_UN_LAYER_COL_TEXT = "underlayer_color";
    inline const string RENDER_PALETTE_TEXT = "color_palette";

    svg::Color Color_Node(const json::Node& node) {
        if (node.IsString()) return  { node.AsString() };
        if (node.IsArray()) {
            auto& node_arr = node.AsArray();
            if (node_arr.size() == 3)   return { svg::Rgb{
                static_cast<uint8_t>(node_arr[0].AsInt()), 
                static_cast<uint8_t>(node_arr[1].AsInt()), 
                static_cast<uint8_t>(node_arr[2].AsInt())} 
                };
            if (node_arr.size() == 4)   return { svg::Rgba{
                static_cast<uint8_t>(node_arr[0].AsInt()),
                static_cast<uint8_t>(node_arr[1].AsInt()),
                static_cast<uint8_t>(node_arr[2].AsInt()), 
                node_arr[3].AsDouble()} };
        }
        return  {};
    }

    Render::Render(const json::Document& doc) {
        auto& root = doc.GetRoot().AsMap();
        if (root.count(RENDER_SETTINGS_TEXT))
        {
            auto& property_map = root.at(RENDER_SETTINGS_TEXT).AsMap();

            if (property_map.count(RENDER_WIDTH_TEXT)) width_ = property_map.at(RENDER_WIDTH_TEXT).AsDouble();
            if (property_map.count(RENDER_HEIGHT_TEXT)) height_ = property_map.at(RENDER_HEIGHT_TEXT).AsDouble();
            if (property_map.count(RENDER_PADDING_TEXT)) padding_ = property_map.at(RENDER_PADDING_TEXT).AsDouble();
            if (property_map.count(RENDER_STOP_RA_TEXT)) stop_radius_ = property_map.at(RENDER_STOP_RA_TEXT).AsDouble();
            if (property_map.count(RENDER_LINE_WI_TEXT)) line_width_ = property_map.at(RENDER_LINE_WI_TEXT).AsDouble();

            if (property_map.count(RENDER_BUS_LABEL_F_S_TEXT)) bus_label_.font_size = property_map.at(RENDER_BUS_LABEL_F_S_TEXT).AsInt();
            if (property_map.count(RENDER_BUS_LABEL_O_S_TEXT)) {
                auto& arr = property_map.at(RENDER_BUS_LABEL_O_S_TEXT).AsArray();
                bus_label_.offset_x = arr[0].AsDouble();
                bus_label_.offset_y = arr[1].AsDouble();
            }
            if (property_map.count(RENDER_STOP_LABEL_F_S_TEXT)) stop_label_.font_size = property_map.at(RENDER_STOP_LABEL_F_S_TEXT).AsInt();
            if (property_map.count(RENDER_STOP_LABEL_O_S_TEXT)) {
                auto& arr = property_map.at(RENDER_STOP_LABEL_O_S_TEXT).AsArray();
                stop_label_.offset_x = arr[0].AsDouble();
                stop_label_.offset_y = arr[1].AsDouble();
            }
            if (property_map.count(RENDER_U_LAY_W_TEXT)) underlayer_width_ = property_map.at(RENDER_U_LAY_W_TEXT).AsDouble();
            if (property_map.count(RENDER_UN_LAYER_COL_TEXT)) {
                underlayer_ = Color_Node(property_map.at(RENDER_UN_LAYER_COL_TEXT));
            }
            if (property_map.count(RENDER_PALETTE_TEXT)) {
                auto& arr = property_map.at(RENDER_PALETTE_TEXT).AsArray();
                for (auto& el : arr)
                {
                    palette_.push_back(Color_Node(el));
                }
            }
        }
        else {
            std::cerr << "RENDER_SETTINGS not found" << std::endl;
        }
        //else  throw json::ParsingError("RENDER_SETTINGS not found"s);
    }
    Render::Render() :
        width_(600),
        height_(400),
        padding_(50),
        stop_radius_(5),
        line_width_(14),
        bus_label_({ 20, 7, 15 }),
        stop_label_({ 20, 7, -3 }),
        underlayer_(svg::Rgba{ 255, 255, 255, 0.85 }),
        underlayer_width_(3),
        palette_({ "green", svg::Rgb{255, 160, 0}, "red" }) {}

    svg::Text GetTextTemplateBus(const Render& render, bool underlayer = false) {
        svg::Text text;
        text.SetFontFamily("Verdana").SetFontWeight("bold").SetFontSize(render.bus_label_.font_size).SetOffset({ render.bus_label_.offset_x, render.bus_label_.offset_y });
        if (underlayer)
            text.SetStrokeColor(render.underlayer_).SetFillColor(render.underlayer_).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                SetStrokeWidth(render.underlayer_width_);
        return text;
    }

    svg::Document BusDepotMap(transport::BusDepot& depot, const Render& render) {
        svg::Document result;
        vector<geo::Coordinates> geo_coords;
        auto& buses = depot.GetAllBuses();
        for (auto& bus : buses)
        {
            for (auto& route : bus.bus_route_)
            {
                geo_coords.push_back(route->geo_);
            }
        }
        const SphereProjector proj{geo_coords.begin(), geo_coords.end(), static_cast<double>(render.width_), 
            static_cast<double>(render.height_), static_cast<double>(render.padding_)};
        //auto buses = depot.GetAllBuses();

        auto palette = render.palette_.begin();

        std::vector<const transport::Bus*> buses_ptr;
        buses_ptr.reserve(buses.size());
        for (auto& bus : buses) buses_ptr.push_back(&bus);
        std::sort(buses_ptr.begin(), buses_ptr.end(), [](auto left, auto right) { return left->name_ < right->name_; });

        for (auto& bus : buses_ptr)
        {
            svg::Polyline polyline;

            for (auto stop : bus->bus_route_)
            {
                polyline.AddPoint(proj(stop->geo_));
            }
            if ( ! bus->roundtrip_)
            {
                bool is_first = true;
                for (auto stop_it = bus->bus_route_.rbegin(); stop_it != bus->bus_route_.rend() ; ++stop_it)
                {
                    if (is_first)
                        is_first = false; 
                    else
                        polyline.AddPoint(proj((*stop_it)->geo_));
                }
            }
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
            SetStrokeWidth(render.line_width_).SetFillColor(svg::NoneColor).SetStrokeColor(*palette);
            if(++palette == render.palette_.end()) palette = render.palette_.begin();
            result.Add(polyline);
        }
        // выводим названия маршрутов
        palette = render.palette_.begin();
        for (auto& bus : buses_ptr)
        {
            if (bus->bus_route_.size())
            {
                auto& stop = *(bus->bus_route_.begin());
                { // подложка
                    auto text = GetTextTemplateBus(render, true);
                    text.SetData(bus->name_).SetPosition(proj(stop->geo_));
                    result.Add(text);
                }
                { // текст
                    auto text = GetTextTemplateBus(render);
                    text.SetData(bus->name_).SetPosition(proj(stop->geo_)).SetFillColor(*palette);
                    result.Add(text);
                }
                if ( (! bus->roundtrip_) && (stop != (*bus->bus_route_.rbegin())))
                {
                    auto& stop = *(bus->bus_route_.rbegin());
                    { // подложка
                        auto text = GetTextTemplateBus(render, true);
                        text.SetData(bus->name_).SetPosition(proj(stop->geo_));
                        result.Add(text);
                    }
                    { // текст
                        auto text = GetTextTemplateBus(render);
                        text.SetData(bus->name_).SetPosition(proj(stop->geo_)).SetFillColor(*palette);
                        result.Add(text);
                    }
                }
                if (++palette == render.palette_.end()) palette = render.palette_.begin();
            }
        }

        // готовим список остановок
        std::vector<const transport::Stop*> stops_ptr;
        stops_ptr.reserve(buses.size());
        for (auto& bus : buses)
            for (auto stop : bus.bus_route_)
                stops_ptr.push_back(stop);
        std::sort(stops_ptr.begin(), stops_ptr.end(), [](auto left, auto right) { return left->name_ < right->name_; });
        auto it_arr = std::unique(stops_ptr.begin(), stops_ptr.end());
        stops_ptr.erase(it_arr, stops_ptr.end());
        // круги остановок
        for (auto stop_ptr : stops_ptr)
        {
            result.Add(svg::Circle().SetCenter(proj(stop_ptr->geo_)).SetFillColor({ "white" }).SetRadius(render.stop_radius_));
        }
        for (auto stop_ptr : stops_ptr)
        {
            { // подложка
                svg::Text text;
                text.SetFontFamily("Verdana").SetData(stop_ptr->name_).SetPosition(proj(stop_ptr->geo_)).
                    SetFontSize(render.stop_label_.font_size).SetOffset({ render.stop_label_.offset_x, render.stop_label_.offset_y }).
                    SetFillColor(render.underlayer_).SetStrokeColor(render.underlayer_).SetStrokeWidth(render.underlayer_width_).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                result.Add(text);
            }
            { // текст
                svg::Text text;
                text.SetFontFamily("Verdana").SetData(stop_ptr->name_).SetPosition(proj(stop_ptr->geo_)).
                    SetFontSize(render.stop_label_.font_size).SetOffset({ render.stop_label_.offset_x, render.stop_label_.offset_y }).
                    SetFillColor("black");
                result.Add(text);
            }
        }


        return result;
    }
}

