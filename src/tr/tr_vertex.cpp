#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <ranges>

#include "tr_vertex.h"

namespace tr {

GLenum data_format_to_gl(data_format t)
{
    switch(t)
    {
        case data_format::INT8:                  return GL_BYTE;
        case data_format::UINT8:                 return GL_UNSIGNED_BYTE;
        case data_format::INT16:                 return GL_SHORT;
        case data_format::UINT16:                return GL_UNSIGNED_SHORT;
        case data_format::INT32:                 return GL_INT;
        case data_format::UINT32:                return GL_UNSIGNED_INT;
        case data_format::FLOAT16:               return GL_HALF_FLOAT;
        case data_format::FLOAT32:               return GL_FLOAT;
        case data_format::FIXED16:               return GL_FIXED;
        case data_format::INT_2_10_10_10_REV:    return GL_INT_2_10_10_10_REV;
        case data_format::UINT_2_10_10_10_REV:   return GL_UNSIGNED_INT_2_10_10_10_REV;
        case data_format::UINT_11F_10F_10F_REV:  return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case data_format::BGRA:                  return GL_BGRA;
    }
    spdlog::critical("Unable to convert from the given vertex format {}", static_cast<unsigned>(t));
    std::exit(1);
    return GL_FALSE;
}

GLenum primitive_to_gl(primitive p)
{
    switch(p)
    {
        case primitive::points:             return GL_POINTS;
        case primitive::lines:              return GL_LINES;
        case primitive::line_strips:        return GL_LINE_STRIP;
        case primitive::line_loops:         return GL_LINE_LOOP;
        case primitive::triangles:          return GL_TRIANGLES;
        case primitive::triangle_strips:    return GL_TRIANGLE_STRIP;
        case primitive::patches:            return GL_PATCHES;
    }
    spdlog::critical("Unable to convert from the given primitive type {}", static_cast<unsigned>(p));
    std::exit(1);
    return GL_FALSE;
}

struct vertex_object_impl
{
    virtual bool build(bool indexed, size_t index_size_bytes, const std::vector<vertex_specifier>& fmts) { return false; }
    virtual void draw(bool indexed, size_t instance_count) {}
    virtual void update(vertex_object::update_type type, size_t index, const void* buffer, size_t length) {}
};


struct gl_vertex_object_impl : public vertex_object_impl
{
    std::vector<std::vector<uint8_t>> vertex_buffers_{ };
    std::vector<size_t> cumulative_length_{ };
    std::vector<uint8_t> index_buffer_{ };

    unsigned vao_{ 0 };
    std::vector<unsigned> vbo_{ };
    unsigned ibo_{ 0 };
    GLenum primitive_{ GL_TRIANGLES };
    GLenum index_format_{ GL_UNSIGNED_INT };
    size_t indicies_{ 0 };
    size_t vertex_count_{ 0 };

    /// @brief If the vertex buffers have been populated with data.
    bool buffers_populated_{ false };

    /// @brief If the index buffer has been modified since the last draw call.
    bool index_dirty_{ true };
    /// @brief If the vertex buffers have been modified since the last draw call.
    std::vector<bool> vertex_dirty_{ };

    gl_vertex_object_impl()
    {
        // is seperate attribute format supported.
        if(GLAD_GL_ARB_vertex_attrib_binding) {
            spdlog::info("Seperate attribute format support enabled.");
        }
    }
    ~gl_vertex_object_impl()
    {
        glDeleteBuffers(1, &ibo_);
        glDeleteBuffers(vertex_buffers_.size(), vbo_.data());
        glDeleteVertexArrays(1, &vao_);
    }

    // Abstract write structured data to the vertex buffer.
    void update(vertex_object::update_type type, size_t index, const void* buffer, size_t length) override
    {
        if(type == vertex_object::update_type::vertex) {
            // should think of a better abstraction for writing the indicies.
            vertex_buffers_[index].assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + length);
            vertex_dirty_[index] = true;
        } else if(type == vertex_object::update_type::index) {
            index_dirty_ = true;
            index_buffer_.assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + length);
            indicies_ = index;
        } else {
            spdlog::critical("Unknown update type specified: {}", static_cast<unsigned>(type));
            std::exit(1);
        }
    }

    void draw(bool indexed, size_t instance_count) override
    {
        // We need to validate that the vertex buffers have been built before we can draw.
        if(!buffers_populated_) {
            buffers_populated_ = true;
            for(size_t n = 0; n < vertex_buffers_.size(); ++n) {
                if(vertex_buffers_[n].empty()) {
                    spdlog::critical("Vertex buffer {} is empty, cannot draw.", n);
                    std::exit(1);
                }
                if(vertex_dirty_[n]) {
                    cumulative_length_.emplace_back(vertex_buffers_[n].size());
                } else {
                    buffers_populated_ = false;
                }
            }
        }

        if(!buffers_populated_) {
            spdlog::critical("Vertex buffers have not been populated, cannot draw.");
            std::exit(1);
        }

        // Update the opengl vertex if buffers have been modified.
        // Either glBufferSubData() or glNamedBufferSubData()
        for(size_t n= 0; n < vertex_buffers_.size(); ++n) {
            if(vertex_dirty_[n]) {
                // If we have GL_ARB_direct_state_access then we can use glNamedBufferSubData() to update the buffer.
                if(GLAD_GL_ARB_direct_state_access) {
                    if(GLAD_GL_ARB_vertex_attrib_binding) {
                        glNamedBufferSubData(vbo_[n], 0, vertex_buffers_[n].size(), vertex_buffers_[n].data());
                    } else {
                        glNamedBufferSubData(vbo_[0], cumulative_length_[n], vertex_buffers_[n].size(), vertex_buffers_[n].data());
                    }
                } else {
                    if(GLAD_GL_ARB_vertex_attrib_binding) {
                        glBindBuffer(GL_ARRAY_BUFFER, vbo_[n]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffers_[n].size(), vertex_buffers_[n].data());
                    } else {
                        glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
                        glBufferSubData(GL_ARRAY_BUFFER, cumulative_length_[n], vertex_buffers_[n].size(), vertex_buffers_[n].data());
                    }
                }
                vertex_dirty_[n] = false;
            }
        }

        // Update the index buffer if it has been modified.
        if(indexed && index_dirty_) {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_.size(), index_buffer_.data());
            index_dirty_ = false;
        }        

        // Draw call, n.b. all draw calls use indexing.
        if(instance_count > 0) {
            glBindVertexArray(vao_);
            if(indexed) {
                glDrawElementsInstanced(primitive_, indicies_, index_format_, nullptr, instance_count);
            } else {
                glDrawArraysInstanced(primitive_, 0, vertex_count_, instance_count);
            }
        } else {
            glBindVertexArray(vao_); 
            if(indexed) {
                glDrawElements(primitive_, indicies_, index_format_, nullptr);
            } else {
                glDrawArrays(primitive_, 0, vertex_count_);
            }
        }
    }

    bool build(bool indexed, size_t index_size_bytes, const std::vector<vertex_specifier>& fmts) override
    {
        // The calling function should garauntee that fmts isn't empty.
        // It's in the contract.

        // Generate buffers based on the number of format lists passed in.
        // should be one singular vertex buffer if no GLAD_GL_ARB_vertex_attrib_binding
        if(GLAD_GL_ARB_vertex_attrib_binding) {
            vertex_buffers_.resize(fmts.size());
            vertex_dirty_.resize(fmts.size());
            vbo_.resize(fmts.size());
            glGenBuffers(fmts.size(), vbo_.data());
        } else {
            vertex_buffers_.resize(1);
            vertex_dirty_.resize(1);
            vbo_.resize(1);
            glGenBuffers(1, vbo_.data());
        }

        // Create a Vertex Array
        glGenVertexArrays(1, &vao_);
        if(!GLAD_GL_ARB_direct_state_access) {
            glBindVertexArray(vao_);
        }
   
        // if we had GL_ARB_buffer_storage and we knew the size of the index buffer in advance we can
        // create the buffer and optionally assign data to it.
        // Would require passing in the data size from the interface api though, which is doable.
        // N.B. if we needed to change the buffer size then we would need to destroy the buffer index and re-create it.
        if(indexed) {
            if(GLAD_GL_ARB_buffer_storage) {
                glCreateBuffers(1, &ibo_);
                if(index_size_bytes > 0) {
                    if(GLAD_GL_ARB_direct_state_access) {
                        glNamedBufferStorage(ibo_, index_size_bytes, NULL, GL_DYNAMIC_STORAGE_BIT);
                    } else {
                        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, index_size_bytes, NULL, GL_DYNAMIC_STORAGE_BIT);
                    }
                }
            } else {
                glGenBuffers(1, &ibo_);
                if(GLAD_GL_ARB_direct_state_access) {
                    glVertexArrayElementBuffer(vao_, ibo_);
                } else {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
                }
            }

            // Resize the underlying index buffer if we know the size
            if(index_size_bytes > 0) {
                index_buffer_.resize(index_size_bytes);
            }
        }

        for (auto [fmt, buff, vbuffer] : std::views::zip(fmts, vbo_, vertex_buffers_)) {

            // Resize the individual buffers if the elements are specified.
            if(GLAD_GL_ARB_vertex_attrib_binding) {
                if(fmt.elements_ != 0) {
                    vbuffer.resize(fmt.elements_ * fmt.stride_);
                }
            }

            if(GLAD_GL_ARB_direct_state_access && GLAD_GL_ARB_vertex_attrib_binding) {
                // Direct state access means not binding the buffer before use.
                for(auto& attrib : fmt.vformats_) {
                    GLenum atype = data_format_to_gl(attrib.type_);
                    glEnableVertexArrayAttrib(vao_, attrib.attrib_);
                    if(attrib.conversion_ == vertex_format_conversion::integer) {
                        glVertexAttribIFormat(attrib.attrib_, 
                        attrib.count_, 
                        atype, 
                        attrib.offset_);
                    } else {
                        glVertexArrayAttribFormat(vao_, 
                            attrib.attrib_, 
                            attrib.count_, 
                            atype, 
                            attrib.conversion_ == vertex_format_conversion::float_range ? GL_TRUE : GL_FALSE, 
                            attrib.offset_);
                    }
                    glVertexArrayVertexBuffer(vao_, attrib.binding_index_, buff, 0, fmt.stride_);
                    glVertexArrayBindingDivisor(vao_, attrib.attrib_, attrib.divisor_);
                }

            } else if(GLAD_GL_ARB_vertex_attrib_binding) {
                // seperate attribute binding, but no direct state access.
                glBindVertexBuffer(0, buff, 0, fmt.stride_);
                for(auto& attrib : fmt.vformats_) {
                    GLenum atype = data_format_to_gl(attrib.type_);
                    glEnableVertexAttribArray(attrib.attrib_);
                    if(attrib.conversion_ == vertex_format_conversion::integer) {
                        glVertexAttribIFormat(attrib.attrib_, 
                        attrib.count_, 
                        atype, 
                        attrib.offset_);
                    } else {
                        glVertexAttribFormat(attrib.attrib_, 
                            attrib.count_, 
                            atype, 
                            attrib.conversion_ == vertex_format_conversion::float_range ? GL_TRUE : GL_FALSE, 
                            attrib.offset_);
                    }
                    glVertexBindingDivisor(attrib.attrib_, attrib.divisor_);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            } else if(GLAD_GL_ARB_direct_state_access) {
                // direct state access, but no seperate attribute binding.
            } else {
                // no direct state access and no seperate attribute binding.
                glBindBuffer(GL_ARRAY_BUFFER, buff);
                for(auto& attrib : fmt.vformats_) {
                    GLenum atype = data_format_to_gl(attrib.type_);
                    glEnableVertexAttribArray(attrib.attrib_);

                    if(attrib.conversion_ == vertex_format_conversion::integer) {
                        glVertexAttribIFormat(attrib.attrib_, 
                        attrib.count_, 
                        atype, 
                        attrib.offset_);
                    } else {
                        glVertexAttribFormat(attrib.attrib_, 
                        attrib.count_, 
                        atype, 
                        attrib.conversion_ == vertex_format_conversion::float_range ? GL_TRUE : GL_FALSE, 
                        attrib.offset_);
                    }
                    glVertexAttribDivisor(attrib.attrib_, attrib.divisor_);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        return true;
    }
};

vertex_object vertex_object::create(std::string_view pipeline)
{
    if(pipeline == "opengl") {
        auto impl = std::unique_ptr<gl_vertex_object_impl>(new gl_vertex_object_impl);
        return vertex_object(pipeline, std::move(impl));
    } else if(pipeline == "vulkan") {
        /// XXX
        return vertex_object(pipeline, nullptr);
    } else {
        spdlog::critical("Unrecognised graphics pipeline given {}.", pipeline);
        std::exit(1);
    }
}

vertex_object::vertex_object()
    : api_("none") 
    , pimpl_(new vertex_object_impl())
{
}

vertex_object::vertex_object(std::string_view api, std::unique_ptr<vertex_object_impl> impl)
    : api_(api)
    , pimpl_(std::move(impl))
{
}

vertex_object::~vertex_object()
{
}

vertex_object::vertex_object(vertex_object && rhs) noexcept = default;
vertex_object& vertex_object::operator=(vertex_object && rhs) noexcept = default;

vertex_object::vertex_object(const vertex_object& rhs)
    : pimpl_(new vertex_object_impl(*rhs.pimpl_))
{
}

vertex_object& vertex_object::operator=(const vertex_object& rhs)
{
    if(this != &rhs) {
        pimpl_.reset(new vertex_object_impl(*rhs.pimpl_));
    }

    return *this;
}

void vertex_object::bind() const
{
    // XXX
}

void vertex_object::add(size_t stride, const vertex_format_list_t& fmts, size_t elements)
{
    fmts_.emplace_back(stride, fmts, elements);
}

void vertex_object::draw(size_t instance_count) const
{
    pimpl_->draw(indexed_, instance_count);
}

bool vertex_object::build(bool indexed, data_format dfmt, size_t index_size)
{
    indexed_ = indexed;

    // No format data is an error.
    if(fmts_.empty()) {
        spdlog::critical("No formats specified for vertex object.");
        std::exit(1);
    }

    size_t index_size_bytes = 0;

    if(indexed_) {
        // Data format not unsigned 8, 16 or 32-bit is an error.
        switch(dfmt) {
            case data_format::UINT8:
                index_size_bytes = index_size = sizeof(uint8_t);
                break;
            case data_format::UINT16:
                index_size_bytes = index_size = sizeof(uint16_t);
                break;
            case data_format::UINT32:
                index_size_bytes = index_size = sizeof(uint32_t);
                break;
            default:
                spdlog::critical("Data format must be unsigned char, short or integer.");
                std::exit(1);
        }
    }
    data_format_ = dfmt;
    index_size_ = index_size;

    return pimpl_->build(indexed_, index_size_bytes, fmts_);
}

void vertex_object::update(update_type type, size_t index, const void *buffer, size_t length)
{
    if(type == update_type::vertex && index >= fmts_.size()) {
        spdlog::critical("Index {} exceeds maximum vertex index {}", index, fmts_.size());
        std::exit(1);
    }

    if(type != update_type::vertex && type != update_type::index) {
        spdlog::critical("Neither vertex or index was given for the update type: {}", static_cast<unsigned>(type));
        std::exit(1);
    }        
    pimpl_->update(type, index, buffer, length);
}


vertex_format::vertex_format(int attrib, int count, data_format type, int offset)
    : attrib_(attrib)
    , count_(count)
    , type_(type)
    , conversion_(vertex_format_conversion::float_direct)
    , offset_(offset)
{
    if(!(type == data_format::FLOAT16 || type == data_format::FLOAT32 || type == data_format::FIXED16)) {
        spdlog::critical("Non-supported floating type specified without normalisation.");
        std::exit(1);
    }
}

vertex_format::vertex_format(int attrib, int count, data_format type, vertex_format_conversion conversion, int offset)
    : attrib_(attrib)
    , count_(count)
    , type_(type)
    , conversion_(conversion)
    , offset_(offset)
{
    if(conversion_ == vertex_format_conversion::integer) {
        // direct integer use only supported for certain types.
        if(!(type == data_format::INT8 
            || type == data_format::UINT8 
            || type == data_format::INT16 
            || type == data_format::UINT16
            || type == data_format::INT32
            || type == data_format::UINT32)) {
            spdlog::critical("integer non-normalisation only supported for integer verticies.");
            std::exit(1);
        }
    }
}

vertex_specifier::vertex_specifier(size_t stride, const vertex_format_list_t& fmts, size_t elements)
    : stride_(stride)
    , vformats_(fmts)
    , elements_(elements)
{
}

}
