#include <vector>

#include "domain.h"
#include "geo.h"
#include "transport_catalogue.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 */

using namespace std;

namespace transport {

    svg::Document BusDepotMap(const TransportCatalogue& depot, const renderer::MapRenderer& renderer) {
        svg::Document result;
        vector<geo::Coordinates> geo_coords;
        const auto& buses = depot.GetAllBuses();
        for (auto& bus : buses)
        {
            for (auto& route : bus.bus_route_)
            {
                geo_coords.push_back(route->geo_);
            }
        }
        const renderer::SphereProjector proj{ geo_coords.begin(), geo_coords.end(), renderer.width_,
            renderer.height_, renderer.padding_ };

        auto palette = renderer.palette_.begin();

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
            if (!bus->roundtrip_)
            {
                bool is_first = true;
                for (auto stop_it = bus->bus_route_.rbegin(); stop_it != bus->bus_route_.rend(); ++stop_it)
                {
                    if (is_first)
                        is_first = false;
                    else
                        polyline.AddPoint(proj((*stop_it)->geo_));
                }
            }
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                SetStrokeWidth(renderer.line_width_).SetFillColor(svg::NoneColor).SetStrokeColor(*palette);
            if (++palette == renderer.palette_.end()) palette = renderer.palette_.begin();
            result.Add(polyline);
        }
        // выводим названия маршрутов
        palette = renderer.palette_.begin();
        for (auto& bus : buses_ptr)
        {
            if (bus->bus_route_.size())
            {
                auto& stop = *(bus->bus_route_.begin());
                { // подложка
                    auto text = GetTextTemplateBus(renderer, true);
                    text.SetData(bus->name_).SetPosition(proj(stop->geo_));
                    result.Add(text);
                }
                { // текст
                    auto text = GetTextTemplateBus(renderer);
                    text.SetData(bus->name_).SetPosition(proj(stop->geo_)).SetFillColor(*palette);
                    result.Add(text);
                }
                if ((!bus->roundtrip_) && (stop != (*bus->bus_route_.rbegin())))
                {
                    auto& stop = *(bus->bus_route_.rbegin());
                    { // подложка
                        auto text = GetTextTemplateBus(renderer, true);
                        text.SetData(bus->name_).SetPosition(proj(stop->geo_));
                        result.Add(text);
                    }
                    { // текст
                        auto text = GetTextTemplateBus(renderer);
                        text.SetData(bus->name_).SetPosition(proj(stop->geo_)).SetFillColor(*palette);
                        result.Add(text);
                    }
                }
                if (++palette == renderer.palette_.end()) palette = renderer.palette_.begin();
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
            result.Add(svg::Circle().SetCenter(proj(stop_ptr->geo_)).SetFillColor({ "white" }).SetRadius(renderer.stop_radius_));
        }
        for (auto stop_ptr : stops_ptr)
        {
            { // подложка
                svg::Text text;
                text.SetFontFamily("Verdana").SetData(stop_ptr->name_).SetPosition(proj(stop_ptr->geo_)).
                    SetFontSize(renderer.stop_label_.font_size).SetOffset(renderer.stop_label_.offset).
                    SetFillColor(renderer.underlayer_).SetStrokeColor(renderer.underlayer_).SetStrokeWidth(renderer.underlayer_width_).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                result.Add(text);
            }
            { // текст
                svg::Text text;
                text.SetFontFamily("Verdana").SetData(stop_ptr->name_).SetPosition(proj(stop_ptr->geo_)).
                    SetFontSize(renderer.stop_label_.font_size).SetOffset(renderer.stop_label_.offset).
                    SetFillColor("black");
                result.Add(text);
            }
        }
        return result;
    }

    svg::Text GetTextTemplateBus(const renderer::MapRenderer& renderer, bool underlayer) {
        svg::Text text;
        text.SetFontFamily("Verdana").SetFontWeight("bold").SetFontSize(renderer.bus_label_.font_size).SetOffset(renderer.bus_label_.offset);
        if (underlayer)
            text.SetStrokeColor(renderer.underlayer_).SetFillColor(renderer.underlayer_).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
            SetStrokeWidth(renderer.underlayer_width_);
        return text;
    }

} //namespace transport