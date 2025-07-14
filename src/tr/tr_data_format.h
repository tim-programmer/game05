#pragma once

namespace tr {

enum class data_format
{
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    FLOAT16,
    FLOAT32,
    // doubles are not supported in core 3.3. Could support via
    // glVertexAttribLPointer, but would require 4.1 version.
    // FLOAT64,
    FIXED16,
    INT_2_10_10_10_REV,
    UINT_2_10_10_10_REV,
    UINT_11F_10F_10F_REV,
    BGRA,
};

enum class primitive
{
    points,
    lines,
    line_strips,
    line_loops,
    triangles,
    triangle_strips,
    patches,
};

}
