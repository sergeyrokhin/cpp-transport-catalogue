#include "map_renderer.h"
#include <algorithm>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace std;

namespace renderer {

    static constexpr double EPSILON = 1e-6;
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    inline const string RENDER_SETTINGS_TEXT = "render_settings"s;
    inline const string RENDER_WIDTH_TEXT = "width"s;
    inline const string RENDER_HEIGHT_TEXT = "height"s;
    inline const string RENDER_PADDING_TEXT = "padding"s;
    inline const string RENDER_STOP_RA_TEXT = "stop_radius"s;
    inline const string RENDER_LINE_WI_TEXT = "line_width"s;

    inline const string RENDER_BUS_LABEL_F_S_TEXT = "bus_label_font_size"s;
    inline const string RENDER_BUS_LABEL_O_S_TEXT = "bus_label_offset"s;

    inline const string RENDER_STOP_LABEL_F_S_TEXT = "stop_label_font_size"s;
    inline const string RENDER_STOP_LABEL_O_S_TEXT = "stop_label_offset"s;
    inline const string RENDER_U_LAY_W_TEXT = "underlayer_width"s;

    inline const string RENDER_UN_LAYER_COL_TEXT = "underlayer_color"s;
    inline const string RENDER_PALETTE_TEXT = "color_palette"s;

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

    MapRenderer::MapRenderer(const json::Document& doc) {
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
                bus_label_.offset.x = arr[0].AsDouble();
                bus_label_.offset.y = arr[1].AsDouble();
            }
            if (property_map.count(RENDER_STOP_LABEL_F_S_TEXT)) stop_label_.font_size = property_map.at(RENDER_STOP_LABEL_F_S_TEXT).AsInt();
            if (property_map.count(RENDER_STOP_LABEL_O_S_TEXT)) {
                auto& arr = property_map.at(RENDER_STOP_LABEL_O_S_TEXT).AsArray();
                stop_label_.offset.x = arr[0].AsDouble();
                stop_label_.offset.y = arr[1].AsDouble();
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
            std::cerr << "RENDER_SETTINGS not found\n"s;
        }
        //else  throw json::ParsingError("RENDER_SETTINGS not found"s);
    }
    MapRenderer::MapRenderer() :
        width_(600),
        height_(400),
        padding_(50),
        stop_radius_(5),
        line_width_(14),
        bus_label_({ 20, {7, 15}}),
        stop_label_({ 20,{7, -3}}),
        underlayer_(svg::Rgba{ 255, 255, 255, 0.85 }),
        underlayer_width_(3),
        palette_({ "green", svg::Rgb{255, 160, 0}, "red" }) {}

} //namespace renderer
