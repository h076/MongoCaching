#ifndef SPOTSERVICE_H_
#define SPOTSERVICE_H_

#include "Service.hpp"

namespace hjw {

    namespace mongo {

        // A dedicated service class to return historical price spots for different stocks
        class SpotService : public Service {

            public:
                SpotService(std::shared_ptr<Connector> conn)
                    : Service(conn) {};

                utils::series * get(const std::string& symbol, const std::string& from, const std::string& to) override;

                utils::series * get(const std::string& symbol, const uint64_t from, const uint64_t to) override;

                utils::series * get(const std::string& symbol, const std::chrono::system_clock::time_point from,
                                    const std::chrono::system_clock::time_point to) override;

                void post() override;
        };
    }
}


#endif // SPOTSERVICE_H_
