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
    virtual bool build(size_t index_size_bytes, const std::vector<vertex_specifier>& fmts) { return false; }
    virtual void draw() {}
};

struct gl_vertex_object_impl : public vertex_object_impl
{
    std::vector<std::vector<uint8_t>> vertex_buffers_{ };
    std::vector<uint8_t> index_buffer_{ };

    unsigned vao_{ 0 };
    std::vector<unsigned> vbo_{ };
    unsigned ibo_{ 0 };
    GLenum primitive_{ GL_TRIANGLES };
    GLenum index_format_{ GL_UNSIGNED_INT };
    size_t indicies_{ 0 };

    bool index_dirty_{ true };

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
    void write(unsigned index, const void* buffer, size_t length)
    {
        if(index >= vertex_buffers_.size()) {
            spdlog::critical("index supplied {} exceeds maximum buffer index {}", index, vertex_buffers_.size());
            std::exit(1);
        }
        // should think of a better abstraction for writing the indicies.
        vertex_buffers_[index].assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + length);
    }

    void write_index(size_t count, const void* buffer, size_t length)
    {
        index_dirty_ = true;
        index_buffer_.assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + length);
        indicies_ = count;
    }

    void draw(size_t instance_count)
    {
        // Update the opengl vertex if buffers have been modified.
        // XXX Either glBufferSubData() or glNamedBufferSubData()

        // Update the index buffer if it has been modified.
        if(index_dirty_) {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_.size(), index_buffer_.data());
            index_dirty_ = false;
        }

        // Draw call, n.b. all draw calls use indexing.
        if(instance_count > 0) {
            glBindVertexArray(vao_); 
            glDrawElementsInstanced(primitive_, indicies_, index_format_, nullptr, instance_count);
        } else {
            glBindVertexArray(vao_); 
            glDrawElements(primitive_, indicies_, index_format_, nullptr);
        }
    }

    bool build(size_t index_size_bytes, const std::vector<vertex_specifier>& fmts) override
    {
        // The calling function should garauntee that fmts isn't empty.
        // It's in the contract.
        
        // Generate buffers based on the number of format lists passed in.
        // should be one singular vertex buffer if no GLAD_GL_ARB_vertex_attrib_binding
        if(GLAD_GL_ARB_vertex_attrib_binding) {
            vertex_buffers_.resize(fmts.size());
            vbo_.resize(fmts.size());
            glGenBuffers(fmts.size(), vbo_.data());
        } else {
            vertex_buffers_.resize(1);
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

void vertex_object::draw() const
{
    pimpl_->draw();
}

bool vertex_object::build(data_format dfmt, size_t index_size)
{
    // No format data is an error.
    if(fmts_.empty()) {
        spdlog::critical("No formats specified for vertex object.");
        std::exit(1);
    }

    size_t index_size_bytes = 0;

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
    data_format_ = dfmt;
    index_size_ = index_size;

    return pimpl_->build(index_size_bytes, fmts_);
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
