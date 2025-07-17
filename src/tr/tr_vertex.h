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
    enum class update_type
    {
        /// @brief Update the vertex data.
        vertex,
        /// @brief Update the index data.
        index,
    };
    static vertex_object create(std::string_view pipeline);
    
    virtual ~vertex_object();
    void bind() const;
    void draw(size_t instance_count = 0) const;

    void add(size_t stride, const vertex_format_list_t& fmts, size_t elements_ = 0);
    bool build(bool indexed, tr::data_format dfmt = tr::data_format::UINT32, size_t index_size = 0);

    template<typename T>
    void update(size_t index, const std::vector<T>& data)
    {
        update(update_type::vertex, index, data.data(), data.size() * sizeof(T));
    }
    template<typename T>
    void update(const std::vector<T>& data)
    {
        update(update_type::index, data.size(), data.data(), data.size() * sizeof(T));
    }

    void update(update_type type, size_t index, const void *buffer, size_t length);

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
    /// @brief The vertex formats that are used by this vertex object.
    /// @note The vertex formats are used to describe the vertex data that is going to be used by the vertex object.
    std::vector<vertex_specifier> fmts_{ };
    /// @brief If the vertex object is indexed or not.
    /// @note If this is true, the \c index_size_ must be set to a non-zero value.
    bool indexed_ = false;
    /// @brief The size of the index buffer, in bytes.
    size_t index_size_ = 0;
    /// @brief The data format of the index buffer.
    tr::data_format data_format_ = tr::data_format::UINT32;
};


}