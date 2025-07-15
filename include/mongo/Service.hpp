#ifndef SERVICE_H_
#define SERVICE_H_

#include "Connector.hpp"

namespace hjw {

    namespace mongo {

        class Service {

            public:
                // Many services could own a connection
                // So it is best to make connection a shared pointer
                Service(std::shared_ptr<const Connector> c) : connection(c) {};

            private:

                std::shared_ptr<const Connector> connection;
        };
    }
}

#endif // SERVICE_H_
