#pragma once

#include <string>
#include <string_view>
#include <json.hpp>
#include <ryml.hpp>

namespace tr {
namespace resource {

void set_resource_path(std::string_view base_path);
std::string load(std::string_view filename);
std::vector<uint8_t> load_binary(std::string_view filename);
nlohmann::json load_json(std::string_view filename);
ryml::Tree load_structured(std::string_view filename);

}
}
