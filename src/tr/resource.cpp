#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <ryml_std.hpp>
#include <c4/format.hpp>
#include "resource.h"

namespace tr {
namespace resource {

namespace fs = std::filesystem;

namespace {
    std::string resource_path;
}

void set_resource_path(std::string_view base_path)
{
    resource_path = base_path;
}

std::vector<uint8_t> load_binary(std::string_view filename)
{
    fs::path p;
    if(!resource_path.empty()) {
        p = fs::path(resource_path) / filename;
    } else {
        p = fs::path(filename);
    }

    std::error_code ec;
    if(!fs::exists(p, ec)) {
        spdlog::critical("File \"{}\" does not exist.", p.string());
        std::exit(1);
    }

    // determine size of the file.
    const size_t f_size = fs::file_size(p, ec);
    if(ec) {
        spdlog::critical("Unable to determine size of file \"{}\"", p.string());
        std::exit(1);
    }

    std::vector<uint8_t> vec;
    std::ifstream f{ p, std::ios::in | std::ios::binary };
    if(!f.is_open() || f.bad()) {
        spdlog::critical("File \"{}\" could not be opened or was bad.", p.string());
        std::exit(1);
    }
    // Clear skipping whitespace
    f.unsetf(std::ios::skipws);
    // Reserve space for the file data
    vec.reserve(f_size);
    // Read the file into the vector
    vec.insert(vec.begin(), std::istream_iterator<uint8_t>(f), std::istream_iterator<uint8_t>());
    return vec;
}

std::string load(std::string_view filename)
{
    fs::path p;
    if(!resource_path.empty()) {
        p = fs::path(resource_path) / filename;
    } else {
        p = fs::path(filename);
    }

    std::error_code ec;
    if(!fs::exists(p, ec)) {
        spdlog::critical("File \"{}\" does not exist.", p.string());
        std::exit(1);
    }

    std::ifstream f{ p };
    std::stringstream ss; 
    ss << f.rdbuf();
    return ss.str();
}

nlohmann::json load_json(std::string_view filename)
{
    // Wrapping asserts from parsing the json to provide more consistent reporting of errors.
    try {
        return nlohmann::json::parse(load(filename));
    } catch(std::exception& e) {
        spdlog::critical("Error parsing json file \"{}\". Error was: {}", filename, e.what());
        std::exit(1);
    }
}

ryml::Tree load_structured(std::string_view filename)
{
    ryml::Tree t;
    ryml::parse_in_arena(ryml::to_csubstr(load(filename)), &t);
    return t;
}

}
}
