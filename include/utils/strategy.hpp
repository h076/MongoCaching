#ifndef STRATEGY_H_
#define STRATEGY_H_

#include <iostream>

#include <sstream>
#include <rapidjson/document.h>

namespace hjw {

    namespace utils {

        // A strategy test report should hold
        // - Test ID
        // - Strategy ID
        // - P/L
        struct testReport {
            int tId;
            int sId;
            double profLoss;

            testReport(double t, int s, double pl) : tId(t), sId(s), profLoss(pl) {}
        };

        inline std::string reportToJsonString(testReport* r) {
            std::ostringstream oss;
            oss << "{\"tId\":" << r->tId
                << ",\"sId\":" << r->sId
                << ",\"profLoss\":" << r->profLoss << "}";
            return oss.str();
        }

        inline testReport* stringToReport(const std::string& json) {
            rapidjson::Document doc;
            doc.Parse(json.c_str());

            // Check for parse errors
            if (doc.HasParseError()) {
                std::cerr << "Error parsing json string to testReport json : " << doc.GetParseError() << std::endl;
                return nullptr;
            }

            return new testReport(doc["tId"].GetInt(), doc["sId"].GetInt(), doc["profLoss"].GetDouble());
        }
    }
}

#endif // STRATEGY_H_
