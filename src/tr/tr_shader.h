#pragma once

#include <string>
#include <vector>
#include <memory>
#include <string_view>
#include <ryml.hpp>

namespace tr {

struct shader;
typedef std::shared_ptr<shader> shader_ptr;

class tr_shader 
{
public:
    explicit tr_shader(std::string_view name, const std::vector<shader_ptr>& shaders);
    ~tr_shader();
    void apply() const;
private:
    std::string name_;
    unsigned program_{ 0 };
};

typedef std::vector<tr_shader> tr_shader_list;

tr_shader_list load_shaders(const ryml::NodeRef& yml);

}
