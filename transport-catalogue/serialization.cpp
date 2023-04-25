#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "serialization.h"

#include <fstream>
#include <utility>

using namespace std;
namespace transport {

	enum class ColorVariant { monostate = 1, string = 2, Rgb = 3, Rgba = 4 };

	transport_catalogue_serialize::FontSet CreateFontSet(const renderer::FontSet font) {
		transport_catalogue_serialize::FontSet font_set;
		font_set.set_font_size(font.font_size);
		font_set.set_x(font.offset.x);
		font_set.set_y(font.offset.y);
		return font_set;
	}

	renderer::FontSet CreateFontSet(transport_catalogue_serialize::FontSet font) {
		renderer::FontSet font_set;

		font_set.font_size = font.font_size();
		font_set.offset.x = font.x();
		font_set.offset.y = font.y();
		return font_set;
	}

	transport_catalogue_serialize::Color CreateColorSet(const svg::Color color) {
		transport_catalogue_serialize::Color color_set;
		if (std::holds_alternative<monostate>(color)) {
			color_set.set_variant((uint32_t)ColorVariant::monostate);
		}
		else if (std::holds_alternative<string>(color)) {
			color_set.set_variant((uint32_t)ColorVariant::string);
			color_set.set_c_strinr(get<string>(color));
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			color_set.set_variant((uint32_t)ColorVariant::Rgb);
			transport_catalogue_serialize::Rgb new_color;
			new_color.set_blue(get<svg::Rgb>(color).blue);
			new_color.set_green(get<svg::Rgb>(color).green);
			new_color.set_red(get<svg::Rgb>(color).red);
			*color_set.mutable_c_rgb() = move(new_color);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			color_set.set_variant((uint32_t)ColorVariant::Rgba);
			transport_catalogue_serialize::Rgba new_color;
			new_color.set_blue(get<svg::Rgba>(color).blue);
			new_color.set_green(get<svg::Rgba>(color).green);
			new_color.set_red(get<svg::Rgba>(color).red);
			new_color.set_opacity(get<svg::Rgba>(color).opacity);
			*color_set.mutable_c_rgba() = move(new_color);
		}
		return color_set;
	}

	svg::Color CreateColorSet(const transport_catalogue_serialize::Color color) {
		svg::Color color_set;
		const auto variant = (ColorVariant)color.variant();
		if (variant == ColorVariant::monostate) {
			color_set = monostate{};
		}
		else if (variant == ColorVariant::string) {
			color_set = color.c_strinr();
		}
		else if (variant == ColorVariant::Rgb) {
			color_set = move(svg::Rgb{ static_cast<uint8_t>(color.c_rgb().red()),
										static_cast<uint8_t>(color.c_rgb().green()), 
										static_cast<uint8_t>(color.c_rgb().blue()) });
		}
		else if (variant == ColorVariant::Rgba) {
			color_set = move(svg::Rgba{ static_cast<uint8_t>(color.c_rgba().red()),
										static_cast<uint8_t>(color.c_rgba().green()),
										static_cast<uint8_t>(color.c_rgba().blue()),
										color.c_rgba().opacity() });
		}
		return color_set;
	}
	void TransportCatalogue::Serialize(transport_catalogue_serialize::TransportCatalogue& s_cataloque) const
	{
		s_cataloque.set_bus_velocity_(bus_velocity_);
		s_cataloque.set_bus_wait_time_(bus_wait_time_);
		for (const auto& stop : all_stops_)
		{
			transport_catalogue_serialize::Stop& new_stop = *s_cataloque.add_all_stops();
			new_stop.set_name(stop.name_);
			new_stop.set_lat(stop.geo_.lat);
			new_stop.set_lng(stop.geo_.lng);
		}
		for (const auto& bus : all_buses_)
		{
			transport_catalogue_serialize::Bus& new_bus = *s_cataloque.add_all_buses();
			new_bus.set_name(bus.name_);
			new_bus.set_roundtrip(bus.roundtrip_);
			for (const auto& route : bus.bus_route_) {
				std::string& new_bus_route = *new_bus.add_bus_route();
				new_bus_route = route->name_;
			}
		}
		for (const auto& distance : all_distances_) {
			transport_catalogue_serialize::Distance& new_distance = *s_cataloque.add_all_distances();
			new_distance.set_stop1(distance.first.first->name_);
			new_distance.set_stop2(distance.first.second->name_);
			new_distance.set_distance(distance.second);
		}
	}
	void SerializeMapRenderer(const renderer::MapRenderer& map_renderer, transport_catalogue_serialize::MapRenderer& s_map_renderer)
	{
		s_map_renderer.set_height_(map_renderer.height_);
		s_map_renderer.set_width_(map_renderer.width_);

		s_map_renderer.set_padding_(map_renderer.padding_);
		s_map_renderer.set_stop_radius_(map_renderer.stop_radius_);
		s_map_renderer.set_line_width_(map_renderer.line_width_);
		s_map_renderer.set_underlayer_width_(map_renderer.underlayer_width_);

		*s_map_renderer.mutable_bus_label_() = move(CreateFontSet(map_renderer.bus_label_));
		*s_map_renderer.mutable_stop_label_() = move(CreateFontSet(map_renderer.stop_label_));

		*s_map_renderer.mutable_underlayer_() = move(CreateColorSet(map_renderer.underlayer_));
		for (const auto& color : map_renderer.palette_) {
			transport_catalogue_serialize::Color* new_color = s_map_renderer.add_palette_();
			*new_color = move(CreateColorSet(color));
		}
	}

	void DeSerializeMapRenderer(renderer::MapRenderer& map_renderer, const transport_catalogue_serialize::MapRenderer& load_map_renderer)
	{
		map_renderer.height_ = load_map_renderer.height_();
		map_renderer.width_ = load_map_renderer.width_();

		map_renderer.padding_ = load_map_renderer.padding_();
		map_renderer.stop_radius_ = load_map_renderer.stop_radius_();
		map_renderer.line_width_ = load_map_renderer.line_width_();
		map_renderer.underlayer_width_ = load_map_renderer.underlayer_width_();

		map_renderer.bus_label_ = move(CreateFontSet(load_map_renderer.bus_label_()));
		map_renderer.stop_label_ = move(CreateFontSet(load_map_renderer.stop_label_()));

		map_renderer.underlayer_ = move(CreateColorSet(load_map_renderer.underlayer_()));
		map_renderer.palette_.clear();
		for (const auto& color : load_map_renderer.palette_()) {
			map_renderer.palette_.emplace_back(move(CreateColorSet(color)));
		}
	}

	void TransportCatalogue::SaveTo(const renderer::MapRenderer& map_renderer, const string& file_name) const
	{
		ofstream output(string(file_name), ios::out | ios::binary);

		transport_catalogue_serialize::CataloguePack catalogue_pack;

		Serialize(*catalogue_pack.mutable_catalogue());
		SerializeMapRenderer(map_renderer, *catalogue_pack.mutable_renderer());
		catalogue_pack.SerializeToOstream((ostream*)( &output));
	}

	void DeserializeCatalogue(TransportCatalogue& catalogue, const transport_catalogue_serialize::TransportCatalogue& load_catalogue) {
		catalogue.SetBusVelocity(load_catalogue.bus_velocity_());
		catalogue.SetBusWaitTime(load_catalogue.bus_wait_time_());
		for (const auto& load_stop : load_catalogue.all_stops())
		{
			auto& stop = catalogue.GetAddStop(load_stop.name());
			catalogue.SetStop(stop, load_stop.lat(), load_stop.lng());
		}
		for (const auto& load_bus : load_catalogue.all_buses()) {
			auto& new_bus = catalogue.GetAddBus(load_bus.name());
			new_bus.roundtrip_ = load_bus.roundtrip();
			for (const auto& load_bus_route : load_bus.bus_route()) {
				catalogue.AddStep(new_bus, &catalogue.GetAddStop(load_bus_route));
			}
		}
		for (const auto& load_distance : load_catalogue.all_distances())
		{
			catalogue.AddDistance({ &catalogue.GetAddStop(load_distance.stop1()), &catalogue.GetAddStop(load_distance.stop2()) }, load_distance.distance());
		}
	}

	void DeserializeFrom(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer, const string& file_name) {
		ifstream input(file_name, ios::in | ios::binary);

		transport_catalogue_serialize::CataloguePack catalogue_pack;

		if (!catalogue_pack.ParseFromIstream((istream*)(&input))) {
			return;
		}
		DeserializeCatalogue(catalogue, catalogue_pack.catalogue());
		DeSerializeMapRenderer(map_renderer, catalogue_pack.renderer());
	}
}
