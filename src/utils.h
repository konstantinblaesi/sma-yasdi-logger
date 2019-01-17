#pragma once

#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <limits>
#include <optional>

namespace utils {
    static const std::pair<const std::string, const short> getAndValidate(const std::string &envKey) {
        std::stringstream error;
        int value;
        try {
            value = std::stoi(getenv(envKey.c_str()));
            if (value >= std::numeric_limits<short>::max()) {
                error << envKey << " has to be between " << std::numeric_limits<short>::min()
                      << " and " << std::numeric_limits<short>::max() << ".";
            }
        } catch (const std::invalid_argument &e) {
            error << e.what();
        } catch (const std::out_of_range &e) {
            error << e.what();
        }
        return make_pair(error.str(), value);
    }

    static const std::pair<const std::string, const std::string>
    getAndValidateFile(const std::string &envKey) {
        std::stringstream error;
        std::string file;
        file = getenv(envKey.c_str());
        bool exists = (access(file.c_str(), F_OK) != -1);
        if (!exists) {
            error << "Invalid file " << file << "." << endl;
        }
        return make_pair(error.str(), file);
    }
}