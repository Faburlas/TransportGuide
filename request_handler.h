#pragma once

#include <unordered_set>
#include <optional>
#include "transport_catalogue.h"


//https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:

    RequestHandler(const tg::TransportGuide& db);

    std::optional<BusStatistics> GetBusStat(const std::string& bus_name) const;

private:
    const tg::TransportGuide& db_;
};
