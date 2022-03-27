#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "domain.h"
#include "request_handler.h"
#include "map_renderer.h"

class JsonReader {
public:
    JsonReader() = delete;

    JsonReader(tg::TransportGuide& trans_guide) :
        trans_guide_(trans_guide)  {}

    void BaseRequestsCommands();
    void StatRequestsCommands(std::ostream& output = std::cout);

    void RunCommands(std::istream& input = std::cin, std::ostream& output = std::cout);

private:
    void BaseRequestsStops();
    void BaseRequestsBuses();
    void BaseRequestsDistances();
    void BaseRequestsRenderSettings();

    json::Node StatRequestsStop(const json::Dict&, const int id);
    json::Node StatRequestsBus(const json::Dict&, const int id);
    json::Node StatRequestsMap(const int id);

    /*The base_requests array contains
    information about bus routesand stops in no particular order.*/
    json::Array base_requests_;
    json::Array stat_requests_;
    tg::TransportGuide& trans_guide_;
    json::Dict loaded_requests_;
    json::Dict render_settings_;
};

svg::Color ColorFromJsonMaker(const json::Array& color_array);
svg::Color ColorFromJsonMaker(const std::string& color);