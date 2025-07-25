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

                void post() override;
        };
    }
}


#endif // SPOTSERVICE_H_
