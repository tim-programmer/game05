#include <fstream>
#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include "tr_shader.h"
#include "resource.h"

namespace tr {

using json = nlohmann::json;

struct shader
{
    shader() = delete;
    virtual ~shader();
    explicit shader(std::string_view shader_name, const std::string& shader, unsigned type);
    /// @brief Construct a shader from a binary blob.
    explicit shader(std::string_view shader_name, const std::vector<uint8_t>& shader, unsigned type);
    void compile();
    unsigned index_ = 0;
    /// @brief Default entry point for shaders, can be overridden.
    std::string entry_point_ = "main";
};

shader_ptr shader_factory(std::string_view shader_name, const std::string& content, std::string_view type)
{
    if(content.empty() || content.size() == 0) {
        spdlog::critical("No shader source found for \"{}\"", shader_name);
        std::exit(1);
    }

    if(type == "vertex" || type == "vert" || type == "v") {
        return std::make_shared<shader>(shader_name, content, GL_VERTEX_SHADER);
    } else if(type == "fragment" || type == "frag" || type == "f") {
        return std::make_shared<shader>(shader_name, content, GL_FRAGMENT_SHADER);
    } else if(type == "geometry" || type == "geo" || type == "g") {
        return std::make_shared<shader>(shader_name, content, GL_GEOMETRY_SHADER);
    } else if(type == "compute" || type == "comp" || type == "c") {
        return std::make_shared<shader>(shader_name, content, GL_COMPUTE_SHADER);
    } else if(type == "tesselsation_evaluation" || type == "tess_eval" || type == "evaluation" || type == "eval") {
        return std::make_shared<shader>(shader_name, content, GL_TESS_EVALUATION_SHADER);
    } else if(type == "tesselsation_control" || type == "tess_ctrl" || type == "control" || type == "ctrl") {
        return std::make_shared<shader>(shader_name, content, GL_TESS_CONTROL_SHADER);
    }
    return nullptr;
}

shader_ptr binary_shader_factory(std::string_view shader_name, const std::vector<uint8_t>& content, std::string_view type)
{
    if(content.empty() || content.size() == 0) {
        spdlog::critical("No shader binary data found for \"{}\"", shader_name);
        std::exit(1);
    }

    if(type == "vertex" || type == "vert" || type == "v") {
        return std::make_shared<shader>(shader_name, content, GL_VERTEX_SHADER);
    } else if(type == "fragment" || type == "frag" || type == "f") {
        return std::make_shared<shader>(shader_name, content, GL_FRAGMENT_SHADER);
    } else if(type == "geometry" || type == "geo" || type == "g") {
        return std::make_shared<shader>(shader_name, content, GL_GEOMETRY_SHADER);
    } else if(type == "compute" || type == "comp" || type == "c") {
        return std::make_shared<shader>(shader_name, content, GL_COMPUTE_SHADER);
    } else if(type == "tesselsation_evaluation" || type == "tess_eval" || type == "evaluation" || type == "eval") {
        return std::make_shared<shader>(shader_name, content, GL_TESS_EVALUATION_SHADER);
    } else if(type == "tesselsation_control" || type == "tess_ctrl" || type == "control" || type == "ctrl") {
        return std::make_shared<shader>(shader_name, content, GL_TESS_CONTROL_SHADER);
    }
    return nullptr;
}


shader::~shader()
{
    glDeleteShader(index_);
}

shader::shader(std::string_view shader_name, const std::string& shader, unsigned type)
{
    spdlog::debug("Compiling shader \"{}\" with the following source: {}", shader_name, shader);

    index_ = glCreateShader(type);
    const char* shader_src = shader.c_str();
    glShaderSource(index_, 1, &shader_src, NULL);
    glCompileShader(index_);

    GLint success = 0;
    glGetShaderiv(index_, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLint log_length;
        glGetShaderiv(index_, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log;
        log.resize(log_length);
        glGetShaderInfoLog(index_, log_length, NULL, log.data());
        spdlog::critical("Shader compilation of \"{}\"failed.\n{}", shader_name,  std::string{ log.begin(), log.end() });
        std::exit(1);
    }
}

shader::shader(std::string_view shader_name, const std::vector<uint8_t>& shader, unsigned type)
{
    spdlog::debug("Compiling shader \"{}\" with the following source: {} bytes", shader_name, shader.size());

    index_ = glCreateShader(type);
    glShaderBinary(1, &index_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, shader.data(), static_cast<GLsizei>(shader.size()));
    glSpecializeShaderARB(index_, entry_point_.c_str(), 0, nullptr, nullptr);
    
    GLint success = 0;
    glGetShaderiv(index_, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLint log_length;
        glGetShaderiv(index_, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log;
        log.resize(log_length);
        glGetShaderInfoLog(index_, log_length, NULL, log.data());
        spdlog::critical("Shader compilation of \"{}\" failed.\n{}", shader_name, std::string{ log.begin(), log.end() });
        std::exit(1);
    }
}

void shader::compile()
{
}

tr_shader::tr_shader(std::string_view name, const std::vector<shader_ptr>& shader_list)
    : name_(name)
{
    program_ = glCreateProgram();
    for(auto& shader : shader_list) {
        shader->compile();
        glAttachShader(program_, shader->index_);
    }

    spdlog::debug("Linking shader program \"{}\"", name_);

    glLinkProgram(program_);
    GLint success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if(!success) {
        GLint log_length;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log;
        log.resize(log_length);
        glGetProgramInfoLog(program_, log_length, NULL, log.data());
        spdlog::critical("Shader program linking of \"{}\" failed.\n{}", name_, std::string{ log.begin(), log.end() });
        std::exit(1);
    }

    glValidateProgram(program_);

    // Always detach shaders after a successful link.
    for(auto& shader : shader_list) {
        glDetachShader(program_, shader->index_);
    }
}

void tr_shader::apply() const
{
    glUseProgram(program_);
}

tr_shader::~tr_shader()
{
    glDeleteProgram(program_);
}


// Expects objects, each object having a 'name' and 'shaders' attribute.
// The name is the name of the shader program and the shaders is a list of objects
// for the individual shaders.
tr_shader_list load_shaders(const ryml::NodeRef& cfg)
{
    tr_shader_list programs;

    for(const auto& program : cfg.children()) {
        std::string program_name{ program.key().str, program.key().len };
        std::vector<shader_ptr> shaders;
        for(const auto& shader : program.children()) {
            if(!shader.is_container()) {
                spdlog::critical("Expected shader to be an object, was {}.", shader.type().type_str());
                std::exit(1);
            }

            const auto& type = shader["type"].val();
            const std::string shader_type{ type.str, type.len };

            const std::string shader_name = program_name  + '_' + shader_type;

            shader_ptr shd = nullptr;

            if(shader.has_child("entry_point")) {
                const auto& ep = shader["entry_point"].val();
                const std::string end_point{ ep.str, ep.len };
                shd->entry_point_ = end_point;
                spdlog::debug("Setting entry point for shader \"{}\" to \"{}\"", program_name, shd->entry_point_);
            }

            if(shader.has_child("file"))
            {
                const auto& fn = shader["file"].val();
                const std::string filename{ fn.str, fn.len };

                const bool is_binary_shader = filename.contains(".spv") ? true : false;

                if(is_binary_shader)
                {
                    shd = binary_shader_factory(shader_name, resource::load_binary(filename), shader_type);
                }
                else
                {
                    shd = shader_factory(shader_name, resource::load(filename), shader_type);
                }

                if(!shd) {
                    spdlog::critical("Unable to convert shader type ({}) to a shader.", shader_type);
                    std::exit(1);
                }
            }
            else if(shader.has_child("shader"))
            {
                // shader["shader"]
                const std::string shader_code{ shader["shader"].val().str, shader["shader"].val().len };
                shd = shader_factory(shader_name, shader_code, shader_type);
            }
            else
            {
                spdlog::critical("Expected shader to specify \"file\" and \"type\" fields.");
                std::exit(1);
            }

            shaders.emplace_back(shd);
        }

        programs.emplace_back(program_name, shaders);
    }
    return programs;
}

}
