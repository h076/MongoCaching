#include <iostream>

#include <mongo/Connector.hpp>
#include <mongo/SpotService.hpp>

int main(int argc, char *argv[]) {
    hjw::mongo::Connector conn("statsCluster");
    std::shared_ptr<hjw::mongo::Connector> conn_ptr = std::make_shared<hjw::mongo::Connector>(std::move(conn));

    hjw::mongo::SpotService ss(conn_ptr);

    ss.set("StockData", "Spots");
    ss.get("SOFI", "2023-01-05T00:00:00Z", "2023-01-06T00:00:00Z");

    return 0;
}
