#include <fstream>
#include <sstream>

/////
#include <cassert>
/////

#include "json_reader.h"
#include "svg.h"
#include "map_renderer.h"


using namespace std::literals;

void JsonReader::BaseRequestsCommands() {
    base_requests_ = loaded_requests_.at("base_requests"s).AsArray();

    BaseRequestsStops();
    BaseRequestsDistances();
    BaseRequestsBuses();
    BaseRequestsRenderSettings();
}

//load render settings
void JsonReader::BaseRequestsRenderSettings() {
    render_settings_ = loaded_requests_.at("render_settings"s).AsMap();
}

//load stops from json to transport guide
void JsonReader::BaseRequestsStops() {
    for (const auto& request : base_requests_) {
        const auto& data_node = request.AsMap();
        if (data_node.at("type"s).AsString() == "Stop"s) {
            std::string temp_stop_name = data_node.at("name").AsString();
            Coordinates temp_stop_coordinates {
                data_node.at("latitude"s).AsDouble(),
                data_node.at("longitude"s).AsDouble()
            };
            trans_guide_.AddStop(temp_stop_name, temp_stop_coordinates);
        }
    }
}

//load buses from json to transport guide
void JsonReader::BaseRequestsBuses() {
    for (const auto& request : base_requests_) {
        std::vector<std::string> temp_stops_vec;
        const auto& data_node = request.AsMap();
        if (data_node.at("type"s).AsString() == "Bus"s) {
            std::string temp_bus_name = data_node.at("name").AsString();
            bool is_circle = data_node.at("is_roundtrip").AsBool();
            for (const auto& stop : data_node.at("stops"s).AsArray()) {
                temp_stops_vec.push_back(stop.AsString());
            }

            trans_guide_.AddBus(temp_bus_name, temp_stops_vec, is_circle );
        }
    }
}

//load distances between stops from json to transport guide
void JsonReader::BaseRequestsDistances() {
    for (const auto& request : base_requests_) {
        const auto& description = request.AsMap();
        if (description.at("type"s).AsString() == "Stop"s) {
            const auto from = trans_guide_.FindStop(description.at("name"s).AsString());
            for (const auto& [stop_name, distance] : description.at("road_distances"s).AsMap()) {
                trans_guide_.SetStopsDistance(from->name, stop_name, (distance.AsInt()));
            }
        }
    }
}

void JsonReader::StatRequestsCommands(std::ostream& output) {
    stat_requests_ = loaded_requests_.at("stat_requests"s).AsArray();
    json::Array result;

    for (const auto& request : stat_requests_) {
        const auto& request_info = request.AsMap();
        const auto& type = request_info.at("type"s).AsString();
        const auto& id = request_info.at("id").AsInt();
        if (type == "Stop"s) {
            result.push_back(StatRequestsStop(request_info, id));
        }
        else if (type == "Bus"s) {
            result.push_back(StatRequestsBus(request_info, id));
        }
        else if (type == "Map"s) {
            result.push_back(StatRequestsMap(id));
        }
    }

    const json::Document answer(result);
    Print(answer, output);
}

json::Node JsonReader::StatRequestsStop(const json::Dict& query, const int id) {
    const Stop* search_result = trans_guide_.FindStop(query.at("name"s).AsString());

    if (search_result == nullptr) 
        return  { json::Dict { {"request_id", id},
                    {"error_message"s, "not found"s} } };

    auto tmp_buses_set = trans_guide_.FindAllBusesToStop(search_result);
    json::Array buses_node_array;
    
    //buses_node_array now is empty so we can return it to have [] buses result
    if (tmp_buses_set.empty())
        return { json::Dict { {"buses", buses_node_array},
                {"request_id"s, id} } };

    //We need to sort buses 
    std::vector<Bus*> vector_of_buses;
    for (auto bus : tmp_buses_set) {
        vector_of_buses.push_back(std::move(bus));
    }

    std::sort(vector_of_buses.begin(), vector_of_buses.end(),
        [](Bus* s1, Bus* s2) { return s1->name < s2->name; });


    for (const auto bus : vector_of_buses) {
        buses_node_array.push_back({ bus->name });
    }



    return { json::Dict { {"buses", buses_node_array},
    {"request_id"s, id} } };
}

json::Node JsonReader::StatRequestsBus(const json::Dict& query, const int id) {
    RequestHandler request_handler(trans_guide_);
    auto optional_bus_info = request_handler.GetBusStat(query.at("name"s).AsString());

    if (optional_bus_info) {
        return { json::Dict {
        {"curvature"s, optional_bus_info->curvature },
        {"request_id"s, id},
        {"route_length"s, optional_bus_info->route_length},
        {"stop_count"s, optional_bus_info->stops},
        {"unique_stop_count"s, optional_bus_info->unique_stops} } };
    }

    return { json::Dict {
    {"request_id"s, id },
    {"error_message", "not found"s} } };
}

svg::Color ColorFromJsonMaker(const json::Array& color_array) {
    //string color
    if (color_array.size() == 3) {
        return render::MakeColor( color_array[0].AsInt(), color_array[1].AsInt(),
            color_array[2].AsInt());
    } //rgba
    else {
        return render::MakeColor(
            color_array[0].AsInt(), color_array[1].AsInt(),
            color_array[2].AsInt(), color_array[3].AsDouble());
    }
}

svg::Color ColorFromJsonMaker(const std::string& color) {
    return render::MakeColor(color);
}

json::Node JsonReader::StatRequestsMap(const int id) {
    render::MapRenderer renderer;

    svg::Color underlayer_color;
   
    if (render_settings_.at("underlayer_color"s).IsString())
        underlayer_color = ColorFromJsonMaker(render_settings_.at("underlayer_color"s).AsString());
    else if (render_settings_.at("underlayer_color"s).IsArray())
        underlayer_color = ColorFromJsonMaker(render_settings_.at("underlayer_color"s).AsArray());

    json::Array color_palette_temp_vector = render_settings_.at("color_palette"s).AsArray();
    std::vector<svg::Color> color_palette;

    for (const auto color : color_palette_temp_vector) {
        if(color.IsString())
            color_palette.push_back(ColorFromJsonMaker(color.AsString()));
        else if (color.IsArray())
            color_palette.push_back(ColorFromJsonMaker(color.AsArray()));
    }

    render::Settings settings{
        render_settings_.at("width"s).AsDouble(),
        render_settings_.at("height"s).AsDouble(),
        render_settings_.at("padding"s).AsDouble(),
        render_settings_.at("line_width"s).AsDouble(),
        render_settings_.at("stop_radius"s).AsDouble(),
        render_settings_.at("bus_label_font_size"s).AsInt(),

        { render_settings_.at("bus_label_offset"s).AsArray()[0].AsDouble(),
        render_settings_.at("bus_label_offset"s).AsArray()[1].AsDouble() },

        render_settings_.at("stop_label_font_size"s).AsInt(),

        { render_settings_.at("stop_label_offset"s).AsArray()[0].AsDouble(),
        render_settings_.at("stop_label_offset"s).AsArray()[1].AsDouble()},

        underlayer_color,
        render_settings_.at("underlayer_width"s).AsDouble(),
        color_palette
    };

    renderer.SetSettings(settings);

    //
    const Stops set_of_stops = trans_guide_.GetStopsSharedPtrs();
    Stops ptr_set_with_stops_that_have_buses;
    for (auto bus_ptr : set_of_stops) {
        if (trans_guide_.IsStopDontHaveBuses(bus_ptr))
            ptr_set_with_stops_that_have_buses.insert(bus_ptr);
    }

    renderer.SetBorder(ptr_set_with_stops_that_have_buses);
    renderer.SetBusRoute(trans_guide_.GetBusesSharedPtrs());
    renderer.SetStation(ptr_set_with_stops_that_have_buses);

    svg::Document doc = renderer.GetDocument();

    std::ostringstream render_stream;

    /*std::ofstream file;
    file.open("text.txt", std::ios::out);
    doc.Render(file);*/

    doc.Render(render_stream);

    return { json::Dict {
    {"request_id"s, id },
    {"map", render_stream.str()} } };
}



void JsonReader::RunCommands(std::istream& input, std::ostream& output) {
    loaded_requests_ = json::Load(input).GetRoot().AsMap();
    BaseRequestsCommands();
    StatRequestsCommands(output);
}