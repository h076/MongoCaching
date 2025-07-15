#include <mongo/Connector.hpp>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>

using namespace hjw::mongo;

// using cluster name as index for txt file containing collection strings
void Connector::setConnectionString(const std::string& cluster) {
    std::ifstream txt("connectionStrings.txt");
    std::unordered_map<std::string, std::string> clusterStrings;
    if (txt.is_open()) {
        std::string l;
        std::string uri;
        std::string c;
        while(getline(txt, l)) {
            c = l.substr(0, l.find(':'));
            uri = l.substr(l.find(':'), l.length());
            if (clusterStrings.find(c) != clusterStrings.end()) {
                std::cout << "Connector::setConnectionString [WARNING] Duplicate found" << std::endl;
            }else {
                clusterStrings[c] = uri;
            }
        }
    }else {
        throw std::invalid_argument("connectionStrings cannot be opened ...");
    }

    if (clusterStrings.find(cluster) == clusterStrings.end()) {
        std::cout << "Connector::setConnectionString [ERROR] cluster not found";
    } else {
        connection_string = clusterStrings[cluster];
        std::cout << "Connection string set." << std::endl;
    }
}

// Attempt to connect to the mongoDB cluster
void Connector::connect() {
    try {
        uri = std::make_unique<mongocxx::uri>(connection_string);
        client = std::make_unique<mongocxx::client>(*uri);

        auto admin = (*client)["admin"];

        admin.run_command(bsoncxx::from_json(R"({ "ping": 1 })"));

        std::cout << "Successfully pinged the MongoDB server." << std::endl;
    } catch (const mongocxx::exception &e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
    }

}
