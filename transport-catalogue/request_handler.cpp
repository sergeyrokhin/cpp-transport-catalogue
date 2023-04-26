#include "request_handler.h"
#include "transport_router.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 //roundtrip = true - если нужно посчитать не сумму всего маршрута, а только рассояние
 //между двумя остановками. Для круговых маршрутов это будет тоже самое
 //но для некруговых, считаем и обратный маршрут

namespace transport {
    Length DistCalculate(const transport::TransportCatalogue& catalogue, transport::Stop_Stop stop_stop, bool roundtrip) {

        if (!stop_stop.first)         return { 0, 0 };
        auto g_length = ComputeDistance(stop_stop.first->geo_, stop_stop.second->geo_);
        auto length_l = catalogue.GetFixDistance(stop_stop);
        double length = length_l ? length_l : g_length; //усли не определена фиксированна, то берем гео

        if (!roundtrip)
        {
            auto length_l = catalogue.GetFixDistance({ stop_stop.second, stop_stop.first });
            length += length_l ? length_l : length;
            g_length *= 2;
        }
        return { length, g_length };
    }

    RequestHandler::RequestHandler(const transport::TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer)
    {
    }

    svg::Document RequestHandler::RenderMap() const
    {
        return transport::CatalogueMap(db_, renderer_);
    }

}