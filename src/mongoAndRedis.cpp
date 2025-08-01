#include <iostream>

#include <mongo/Connector.hpp>
#include <mongo/SpotService.hpp>

#include <redis/connectionPool.hpp>
#include <redis/TimeSeriesService.hpp>

#include <utils/time.hpp>

using namespace hjw;

auto co_main(utils::series * s) -> boost::asio::awaitable<void>;

int main(int argc, char* argv[]) {

    mongo::Connector conn("statsCluster");
    std::shared_ptr<mongo::Connector> conn_ptr = std::make_shared<mongo::Connector>(std::move(conn));

    mongo::SpotService ss(conn_ptr);

    ss.set("StockData", "Spots");
    utils::series * series = ss.get("SOFI", "2023-01-05T00:00:00Z", "2023-01-06T00:00:00Z");

    boost::asio::io_context io;
    boost::asio::ip::tcp::resolver resolver(io);

    // spawn co_main
    boost::asio::co_spawn(io, co_main(series), boost::asio::detached);
    io.run();
}

auto co_main(utils::series * s) -> boost::asio::awaitable<void> {
    redis::connectionPool conn_pool(co_await boost::asio::this_coro::executor, "127:0:0:1", "6379", 1);

    std::shared_ptr<boost::redis::connection> conn_ptr = std::move(co_await conn_pool.acquire());

    redis::TimeSeriesService tsService(std::move(conn_ptr));

    co_await tsService.co_addSeries(s);

    auto from = utils::ISO8601ToUint64_t("2023-01-05T00:00:00Z");

    auto to = utils::ISO8601ToUint64_t("2023-01-06T00:00:00Z");

    co_await tsService.co_getSeries("SOFI", from, to);

    conn_pool.release(conn_ptr);

    co_return;

}
