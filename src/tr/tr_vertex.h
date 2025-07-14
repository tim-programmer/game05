#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <utility>

#include "tr_data_format.h"

namespace tr {

enum class vertex_format_conversion
{
    // no normalisation is applied, an integer is an integer.
    integer,
    // conversion from 0-1.0 range is applied.
    float_range,
    // conversion from 255 to 255.0 is applied, no scaling.
    float_direct,
};

class vertex_format
{
public:
    // Non-normalised format, only supports float types.
    explicit vertex_format(int attrib, int count, data_format type, int offset);
    explicit vertex_format(int attrib, int count, data_format type, vertex_format_conversion conversion, int offset);
    ~vertex_format() {}
    /// @brief Must match the attribute defined in the shader.
    int attrib_ = 0;
    /// @brief Count of the number of elements \c data_format that are represented.
    int count_ = 0;
    /// @brief The basic data type of the vertex.
    data_format type_ = data_format::FLOAT32;
    /// @brief How the data is converted/normaised, if anything is to be applied.
    vertex_format_conversion conversion_ = vertex_format_conversion::float_direct;
    /// @brief Offset, in bytes, to the vertex attribute data.
    int offset_ = 0;
    /// @brief How many verticies to advance when doing instanced rendering.
    int divisor_ = 0;
    /// @brief Binding index to use, if supported.
    int binding_index_ = 0;
private:
    vertex_format() = delete;
};

typedef struct std::vector<vertex_format> vertex_format_list_t;

struct vertex_specifier
{
    vertex_specifier(size_t stride, const vertex_format_list_t& fmts, size_t elements = 0);
    /// @brief The distance, in bytes, from one element to the next.
    size_t stride_ = 0;
    /// @brief The number of vertex that are going to be used.
    size_t elements_ = 0;
    vertex_format_list_t vformats_{ };
};

struct vertex_object_impl;

class vertex_object
{
public:
    static vertex_object create(std::string_view pipeline);
    
    virtual ~vertex_object();
    void bind() const;
    void draw() const;

    void add(size_t stride, const vertex_format_list_t& fmts, size_t elements_ = 0);
    bool build(tr::data_format dfmt = tr::data_format::INT32, size_t index_size = 0);

    // moveable
    vertex_object(vertex_object && rhs) noexcept;   
    vertex_object& operator=(vertex_object && rhs) noexcept;

    // and copyable
    vertex_object(const vertex_object& rhs);
    vertex_object& operator=(const vertex_object& rhs);
private:
    vertex_object();
    explicit vertex_object(std::string_view api, std::unique_ptr<vertex_object_impl> impl);
    std::string api_{ };
    std::unique_ptr<vertex_object_impl> pimpl_{ };
    std::vector<vertex_specifier> fmts_{ };
    size_t index_size_ = 0;
    tr::data_format data_format_ = tr::data_format::UINT32;
};


}