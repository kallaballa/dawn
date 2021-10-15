// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/transform/renamer.h"

#include <memory>
#include <unordered_set>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/member_accessor_expression.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Renamer);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Renamer::Data);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Renamer::Config);

namespace tint {
namespace transform {

namespace {

// This list is used for a binary search and must be kept in sorted order.
const char* kReservedKeywordsGLSL[] = {
    "active",
    "asm",
    "atomic_uint",
    "attribute",
    "bool",
    "break",
    "buffer",
    "bvec2",
    "bvec3",
    "bvec4",
    "case",
    "cast",
    "centroid",
    "class",
    "coherent",
    "common",
    "const",
    "continue",
    "default",
    "discard",
    "dmat2",
    "dmat2x2",
    "dmat2x3",
    "dmat2x4",
    "dmat3",
    "dmat3x2",
    "dmat3x3",
    "dmat3x4",
    "dmat4",
    "dmat4x2",
    "dmat4x3",
    "dmat4x4",
    "do",
    "double",
    "dvec2",
    "dvec3",
    "dvec4",
    "else",
    "enum",
    "extern",
    "external",
    "false",
    "filter",
    "fixed",
    "flat",
    "float",
    "for",
    "fvec2",
    "fvec3",
    "fvec4",
    "gl_BaseInstance",
    "gl_BaseVertex",
    "gl_ClipDistance",
    "gl_DepthRangeParameters",
    "gl_DrawID",
    "gl_FragCoord",
    "gl_FragDepth",
    "gl_FrontFacing",
    "gl_GlobalInvocationID",
    "gl_InstanceID",
    "gl_LocalInvocationID",
    "gl_LocalInvocationIndex",
    "gl_NumSamples",
    "gl_NumWorkGroups",
    "gl_PerVertex",
    "gl_PointCoord",
    "gl_PointSize",
    "gl_Position",
    "gl_PrimitiveID",
    "gl_SampleID",
    "gl_SampleMask",
    "gl_SampleMaskIn",
    "gl_SamplePosition",
    "gl_VertexID",
    "gl_WorkGroupID",
    "gl_WorkGroupSize",
    "goto",
    "half",
    "highp",
    "hvec2",
    "hvec3",
    "hvec4",
    "if",
    "iimage1D",
    "iimage1DArray",
    "iimage2D",
    "iimage2DArray",
    "iimage2DMS",
    "iimage2DMSArray",
    "iimage2DRect",
    "iimage3D",
    "iimageBuffer",
    "iimageCube",
    "iimageCubeArray",
    "image1D",
    "image1DArray",
    "image2D",
    "image2DArray",
    "image2DMS",
    "image2DMSArray",
    "image2DRect",
    "image3D",
    "imageBuffer",
    "imageCube",
    "imageCubeArray",
    "in",
    "inline",
    "inout",
    "input",
    "int",
    "interface",
    "invariant",
    "isampler1D",
    "isampler1DArray",
    "isampler2D",
    "isampler2DArray",
    "isampler2DMS",
    "isampler2DMSArray",
    "isampler2DRect",
    "isampler3D",
    "isamplerBuffer",
    "isamplerCube",
    "isamplerCubeArray",
    "ivec2",
    "ivec3",
    "ivec4",
    "layout",
    "long",
    "lowp",
    "main",
    "mat2",
    "mat2x2",
    "mat2x3",
    "mat2x4",
    "mat3",
    "mat3x2",
    "mat3x3",
    "mat3x4",
    "mat4",
    "mat4x2",
    "mat4x3",
    "mat4x4",
    "mediump",
    "namespace",
    "noinline",
    "noperspective",
    "out",
    "output",
    "partition",
    "patch",
    "precise",
    "precision",
    "public",
    "readonly",
    "resource",
    "restrict",
    "return",
    "sample",
    "sampler1D",
    "sampler1DArray",
    "sampler1DArrayShadow",
    "sampler1DShadow",
    "sampler2D",
    "sampler2DArray",
    "sampler2DArrayShadow",
    "sampler2DMS",
    "sampler2DMSArray",
    "sampler2DRect",
    "sampler2DRectShadow",
    "sampler2DShadow",
    "sampler3D",
    "sampler3DRect",
    "samplerBuffer",
    "samplerCube",
    "samplerCubeArray",
    "samplerCubeArrayShadow",
    "samplerCubeShadow",
    "shared",
    "short",
    "sizeof",
    "smooth",
    "static",
    "struct",
    "subroutine",
    "superp",
    "switch",
    "template",
    "this",
    "true",
    "typedef",
    "uimage1D",
    "uimage1DArray",
    "uimage2D",
    "uimage2DArray",
    "uimage2DMS",
    "uimage2DMSArray",
    "uimage2DRect",
    "uimage3D",
    "uimageBuffer",
    "uimageCube",
    "uimageCubeArray",
    "uint",
    "uniform",
    "union",
    "unsigned",
    "usampler1D",
    "usampler1DArray",
    "usampler2D",
    "usampler2DArray",
    "usampler2DMS",
    "usampler2DMSArray",
    "usampler2DRect",
    "usampler3D",
    "usamplerBuffer",
    "usamplerCube",
    "usamplerCubeArray",
    "using",
    "uvec2",
    "uvec3",
    "uvec4",
    "varying",
    "vec2",
    "vec3",
    "vec4",
    "void",
    "volatile",
    "while",
    "writeonly",
};

// This list is used for a binary search and must be kept in sorted order.
const char* kReservedKeywordsHLSL[] = {
    "AddressU",
    "AddressV",
    "AddressW",
    "AllMemoryBarrier",
    "AllMemoryBarrierWithGroupSync",
    "AppendStructuredBuffer",
    "BINORMAL",
    "BLENDINDICES",
    "BLENDWEIGHT",
    "BlendState",
    "BorderColor",
    "Buffer",
    "ByteAddressBuffer",
    "COLOR",
    "CheckAccessFullyMapped",
    "ComparisonFunc",
    "CompileShader",
    "ComputeShader",
    "ConsumeStructuredBuffer",
    "D3DCOLORtoUBYTE4",
    "DEPTH",
    "DepthStencilState",
    "DepthStencilView",
    "DeviceMemoryBarrier",
    "DeviceMemroyBarrierWithGroupSync",
    "DomainShader",
    "EvaluateAttributeAtCentroid",
    "EvaluateAttributeAtSample",
    "EvaluateAttributeSnapped",
    "FOG",
    "Filter",
    "GeometryShader",
    "GetRenderTargetSampleCount",
    "GetRenderTargetSamplePosition",
    "GroupMemoryBarrier",
    "GroupMemroyBarrierWithGroupSync",
    "Hullshader",
    "InputPatch",
    "InterlockedAdd",
    "InterlockedAnd",
    "InterlockedCompareExchange",
    "InterlockedCompareStore",
    "InterlockedExchange",
    "InterlockedMax",
    "InterlockedMin",
    "InterlockedOr",
    "InterlockedXor",
    "LineStream",
    "MaxAnisotropy",
    "MaxLOD",
    "MinLOD",
    "MipLODBias",
    "NORMAL",
    "NULL",
    "Normal",
    "OutputPatch",
    "POSITION",
    "POSITIONT",
    "PSIZE",
    "PixelShader",
    "PointStream",
    "Process2DQuadTessFactorsAvg",
    "Process2DQuadTessFactorsMax",
    "Process2DQuadTessFactorsMin",
    "ProcessIsolineTessFactors",
    "ProcessQuadTessFactorsAvg",
    "ProcessQuadTessFactorsMax",
    "ProcessQuadTessFactorsMin",
    "ProcessTriTessFactorsAvg",
    "ProcessTriTessFactorsMax",
    "ProcessTriTessFactorsMin",
    "RWBuffer",
    "RWByteAddressBuffer",
    "RWStructuredBuffer",
    "RWTexture1D",
    "RWTexture1DArray",
    "RWTexture2D",
    "RWTexture2DArray",
    "RWTexture3D",
    "RasterizerState",
    "RenderTargetView",
    "SV_ClipDistance",
    "SV_Coverage",
    "SV_CullDistance",
    "SV_Depth",
    "SV_DepthGreaterEqual",
    "SV_DepthLessEqual",
    "SV_DispatchThreadID",
    "SV_DomainLocation",
    "SV_GSInstanceID",
    "SV_GroupID",
    "SV_GroupIndex",
    "SV_GroupThreadID",
    "SV_InnerCoverage",
    "SV_InsideTessFactor",
    "SV_InstanceID",
    "SV_IsFrontFace",
    "SV_OutputControlPointID",
    "SV_Position",
    "SV_PrimitiveID",
    "SV_RenderTargetArrayIndex",
    "SV_SampleIndex",
    "SV_StencilRef",
    "SV_Target",
    "SV_TessFactor",
    "SV_VertexArrayIndex",
    "SV_VertexID",
    "Sampler",
    "Sampler1D",
    "Sampler2D",
    "Sampler3D",
    "SamplerCUBE",
    "SamplerComparisonState",
    "SamplerState",
    "StructuredBuffer",
    "TANGENT",
    "TESSFACTOR",
    "TEXCOORD",
    "Texcoord",
    "Texture",
    "Texture1D",
    "Texture1DArray",
    "Texture2D",
    "Texture2DArray",
    "Texture2DMS",
    "Texture2DMSArray",
    "Texture3D",
    "TextureCube",
    "TextureCubeArray",
    "TriangleStream",
    "VFACE",
    "VPOS",
    "VertexShader",
    "abort",
    "allow_uav_condition",
    "asdouble",
    "asfloat",
    "asint",
    "asm",
    "asm_fragment",
    "asuint",
    "auto",
    "bool",
    "bool1",
    "bool1x1",
    "bool1x2",
    "bool1x3",
    "bool1x4",
    "bool2",
    "bool2x1",
    "bool2x2",
    "bool2x3",
    "bool2x4",
    "bool3",
    "bool3x1",
    "bool3x2",
    "bool3x3",
    "bool3x4",
    "bool4",
    "bool4x1",
    "bool4x2",
    "bool4x3",
    "bool4x4",
    "branch",
    "break",
    "call",
    "case",
    "catch",
    "cbuffer",
    "centroid",
    "char",
    "class",
    "clip",
    "column_major",
    "compile",
    "compile_fragment",
    "const",
    "const_cast",
    "continue",
    "countbits",
    "ddx",
    "ddx_coarse",
    "ddx_fine",
    "ddy",
    "ddy_coarse",
    "ddy_fine",
    "default",
    "degrees",
    "delete",
    "discard",
    "do",
    "double",
    "double1",
    "double1x1",
    "double1x2",
    "double1x3",
    "double1x4",
    "double2",
    "double2x1",
    "double2x2",
    "double2x3",
    "double2x4",
    "double3",
    "double3x1",
    "double3x2",
    "double3x3",
    "double3x4",
    "double4",
    "double4x1",
    "double4x2",
    "double4x3",
    "double4x4",
    "dst",
    "dword",
    "dword1",
    "dword1x1",
    "dword1x2",
    "dword1x3",
    "dword1x4",
    "dword2",
    "dword2x1",
    "dword2x2",
    "dword2x3",
    "dword2x4",
    "dword3",
    "dword3x1",
    "dword3x2",
    "dword3x3",
    "dword3x4",
    "dword4",
    "dword4x1",
    "dword4x2",
    "dword4x3",
    "dword4x4",
    "dynamic_cast",
    "else",
    "enum",
    "errorf",
    "explicit",
    "export",
    "extern",
    "f16to32",
    "f32tof16",
    "false",
    "fastopt",
    "firstbithigh",
    "firstbitlow",
    "flatten",
    "float",
    "float1",
    "float1x1",
    "float1x2",
    "float1x3",
    "float1x4",
    "float2",
    "float2x1",
    "float2x2",
    "float2x3",
    "float2x4",
    "float3",
    "float3x1",
    "float3x2",
    "float3x3",
    "float3x4",
    "float4",
    "float4x1",
    "float4x2",
    "float4x3",
    "float4x4",
    "fmod",
    "for",
    "forcecase",
    "frac",
    "friend",
    "fxgroup",
    "goto",
    "groupshared",
    "half",
    "half1",
    "half1x1",
    "half1x2",
    "half1x3",
    "half1x4",
    "half2",
    "half2x1",
    "half2x2",
    "half2x3",
    "half2x4",
    "half3",
    "half3x1",
    "half3x2",
    "half3x3",
    "half3x4",
    "half4",
    "half4x1",
    "half4x2",
    "half4x3",
    "half4x4",
    "if",
    "in",
    "inline",
    "inout",
    "int",
    "int1",
    "int1x1",
    "int1x2",
    "int1x3",
    "int1x4",
    "int2",
    "int2x1",
    "int2x2",
    "int2x3",
    "int2x4",
    "int3",
    "int3x1",
    "int3x2",
    "int3x3",
    "int3x4",
    "int4",
    "int4x1",
    "int4x2",
    "int4x3",
    "int4x4",
    "interface",
    "isfinite",
    "isinf",
    "isnan",
    "lerp",
    "line",
    "lineadj",
    "linear",
    "lit",
    "log10",
    "long",
    "loop",
    "mad",
    "matrix",
    "min10float",
    "min10float1",
    "min10float1x1",
    "min10float1x2",
    "min10float1x3",
    "min10float1x4",
    "min10float2",
    "min10float2x1",
    "min10float2x2",
    "min10float2x3",
    "min10float2x4",
    "min10float3",
    "min10float3x1",
    "min10float3x2",
    "min10float3x3",
    "min10float3x4",
    "min10float4",
    "min10float4x1",
    "min10float4x2",
    "min10float4x3",
    "min10float4x4",
    "min12int",
    "min12int1",
    "min12int1x1",
    "min12int1x2",
    "min12int1x3",
    "min12int1x4",
    "min12int2",
    "min12int2x1",
    "min12int2x2",
    "min12int2x3",
    "min12int2x4",
    "min12int3",
    "min12int3x1",
    "min12int3x2",
    "min12int3x3",
    "min12int3x4",
    "min12int4",
    "min12int4x1",
    "min12int4x2",
    "min12int4x3",
    "min12int4x4",
    "min16float",
    "min16float1",
    "min16float1x1",
    "min16float1x2",
    "min16float1x3",
    "min16float1x4",
    "min16float2",
    "min16float2x1",
    "min16float2x2",
    "min16float2x3",
    "min16float2x4",
    "min16float3",
    "min16float3x1",
    "min16float3x2",
    "min16float3x3",
    "min16float3x4",
    "min16float4",
    "min16float4x1",
    "min16float4x2",
    "min16float4x3",
    "min16float4x4",
    "min16int",
    "min16int1",
    "min16int1x1",
    "min16int1x2",
    "min16int1x3",
    "min16int1x4",
    "min16int2",
    "min16int2x1",
    "min16int2x2",
    "min16int2x3",
    "min16int2x4",
    "min16int3",
    "min16int3x1",
    "min16int3x2",
    "min16int3x3",
    "min16int3x4",
    "min16int4",
    "min16int4x1",
    "min16int4x2",
    "min16int4x3",
    "min16int4x4",
    "min16uint",
    "min16uint1",
    "min16uint1x1",
    "min16uint1x2",
    "min16uint1x3",
    "min16uint1x4",
    "min16uint2",
    "min16uint2x1",
    "min16uint2x2",
    "min16uint2x3",
    "min16uint2x4",
    "min16uint3",
    "min16uint3x1",
    "min16uint3x2",
    "min16uint3x3",
    "min16uint3x4",
    "min16uint4",
    "min16uint4x1",
    "min16uint4x2",
    "min16uint4x3",
    "min16uint4x4",
    "msad4",
    "mul",
    "mutable",
    "namespace",
    "new",
    "nointerpolation",
    "noise",
    "noperspective",
    "numthreads",
    "operator",
    "out",
    "packoffset",
    "pass",
    "pixelfragment",
    "pixelshader",
    "point",
    "precise",
    "printf",
    "private",
    "protected",
    "public",
    "radians",
    "rcp",
    "refract",
    "register",
    "reinterpret_cast",
    "return",
    "row_major",
    "rsqrt",
    "sample",
    "sampler",
    "sampler1D",
    "sampler2D",
    "sampler3D",
    "samplerCUBE",
    "sampler_state",
    "saturate",
    "shared",
    "short",
    "signed",
    "sincos",
    "sizeof",
    "snorm",
    "stateblock",
    "stateblock_state",
    "static",
    "static_cast",
    "string",
    "struct",
    "switch",
    "tbuffer",
    "technique",
    "technique10",
    "technique11",
    "template",
    "tex1D",
    "tex1Dbias",
    "tex1Dgrad",
    "tex1Dlod",
    "tex1Dproj",
    "tex2D",
    "tex2Dbias",
    "tex2Dgrad",
    "tex2Dlod",
    "tex2Dproj",
    "tex3D",
    "tex3Dbias",
    "tex3Dgrad",
    "tex3Dlod",
    "tex3Dproj",
    "texCUBE",
    "texCUBEbias",
    "texCUBEgrad",
    "texCUBElod",
    "texCUBEproj",
    "texture",
    "texture1D",
    "texture1DArray",
    "texture2D",
    "texture2DArray",
    "texture2DMS",
    "texture2DMSArray",
    "texture3D",
    "textureCube",
    "textureCubeArray",
    "this",
    "throw",
    "transpose",
    "triangle",
    "triangleadj",
    "true",
    "try",
    "typedef",
    "typename",
    "uint",
    "uint1",
    "uint1x1",
    "uint1x2",
    "uint1x3",
    "uint1x4",
    "uint2",
    "uint2x1",
    "uint2x2",
    "uint2x3",
    "uint2x4",
    "uint3",
    "uint3x1",
    "uint3x2",
    "uint3x3",
    "uint3x4",
    "uint4",
    "uint4x1",
    "uint4x2",
    "uint4x3",
    "uint4x4",
    "uniform",
    "union",
    "unorm",
    "unroll",
    "unsigned",
    "using",
    "vector",
    "vertexfragment",
    "vertexshader",
    "virtual",
    "void",
    "volatile",
    "while",
};

// This list is used for a binary search and must be kept in sorted order.
const char* kReservedKeywordsMSL[] = {
    "HUGE_VALF",
    "HUGE_VALH",
    "INFINITY",
    "MAXFLOAT",
    "MAXHALF",
    "M_1_PI_F",
    "M_1_PI_H",
    "M_2_PI_F",
    "M_2_PI_H",
    "M_2_SQRTPI_F",
    "M_2_SQRTPI_H",
    "M_E_F",
    "M_E_H",
    "M_LN10_F",
    "M_LN10_H",
    "M_LN2_F",
    "M_LN2_H",
    "M_LOG10E_F",
    "M_LOG10E_H",
    "M_LOG2E_F",
    "M_LOG2E_H",
    "M_PI_2_F",
    "M_PI_2_H",
    "M_PI_4_F",
    "M_PI_4_H",
    "M_PI_F",
    "M_PI_H",
    "M_SQRT1_2_F",
    "M_SQRT1_2_H",
    "M_SQRT2_F",
    "M_SQRT2_H",
    "NAN",
    "access",
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "array",
    "array_ref",
    "as_type",
    "asm",
    "atomic",
    "atomic_bool",
    "atomic_int",
    "atomic_uint",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "bool2",
    "bool3",
    "bool4",
    "break",
    "buffer",
    "case",
    "catch",
    "char",
    "char16_t",
    "char2",
    "char3",
    "char32_t",
    "char4",
    "class",
    "compl",
    "const",
    "const_cast",
    "const_reference",
    "constant",
    "constexpr",
    "continue",
    "decltype",
    "default",
    "delete",
    "depth2d",
    "depth2d_array",
    "depth2d_ms",
    "depth2d_ms_array",
    "depthcube",
    "depthcube_array",
    "device",
    "discard_fragment",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "extern",
    "false",
    "final",
    "float",
    "float2",
    "float2x2",
    "float2x3",
    "float2x4",
    "float3",
    "float3x2",
    "float3x3",
    "float3x4",
    "float4",
    "float4x2",
    "float4x3",
    "float4x4",
    "for",
    "fragment",
    "friend",
    "goto",
    "half",
    "half2",
    "half2x2",
    "half2x3",
    "half2x4",
    "half3",
    "half3x2",
    "half3x3",
    "half3x4",
    "half4",
    "half4x2",
    "half4x3",
    "half4x4",
    "if",
    "imageblock",
    "infinity",
    "inline",
    "int",
    "int16_t",
    "int2",
    "int3",
    "int32_t",
    "int4",
    "int64_t",
    "int8_t",
    "kernel",
    "long",
    "long2",
    "long3",
    "long4",
    "main",
    "matrix",
    "metal",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "override",
    "packed_bool2",
    "packed_bool3",
    "packed_bool4",
    "packed_char2",
    "packed_char3",
    "packed_char4",
    "packed_float2",
    "packed_float3",
    "packed_float4",
    "packed_half2",
    "packed_half3",
    "packed_half4",
    "packed_int2",
    "packed_int3",
    "packed_int4",
    "packed_short2",
    "packed_short3",
    "packed_short4",
    "packed_uchar2",
    "packed_uchar3",
    "packed_uchar4",
    "packed_uint2",
    "packed_uint3",
    "packed_uint4",
    "packed_ushort2",
    "packed_ushort3",
    "packed_ushort4",
    "patch_control_point",
    "private",
    "protected",
    "ptrdiff_t",
    "public",
    "r16snorm",
    "r16unorm",
    "r8unorm",
    "reference",
    "register",
    "reinterpret_cast",
    "return",
    "rg11b10f",
    "rg16snorm",
    "rg16unorm",
    "rg8snorm",
    "rg8unorm",
    "rgb10a2",
    "rgb9e5",
    "rgba16snorm",
    "rgba16unorm",
    "rgba8snorm",
    "rgba8unorm",
    "sampler",
    "short",
    "short2",
    "short3",
    "short4",
    "signed",
    "size_t",
    "sizeof",
    "srgba8unorm",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "template",
    "texture",
    "texture1d",
    "texture1d_array",
    "texture2d",
    "texture2d_array",
    "texture2d_ms",
    "texture2d_ms_array",
    "texture3d",
    "texture_buffer",
    "texturecube",
    "texturecube_array",
    "this",
    "thread",
    "thread_local",
    "threadgroup",
    "threadgroup_imageblock",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "uchar",
    "uchar2",
    "uchar3",
    "uchar4",
    "uint",
    "uint16_t",
    "uint2",
    "uint3",
    "uint32_t",
    "uint4",
    "uint64_t",
    "uint8_t",
    "ulong2",
    "ulong3",
    "ulong4",
    "uniform",
    "union",
    "unsigned",
    "ushort",
    "ushort2",
    "ushort3",
    "ushort4",
    "using",
    "vec",
    "vertex",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
};

}  // namespace

Renamer::Data::Data(Remappings&& r) : remappings(std::move(r)) {}
Renamer::Data::Data(const Data&) = default;
Renamer::Data::~Data() = default;

Renamer::Config::Config(Target t) : target(t) {}
Renamer::Config::Config(const Config&) = default;
Renamer::Config::~Config() = default;

Renamer::Renamer() = default;
Renamer::~Renamer() = default;

Output Renamer::Run(const Program* in, const DataMap& inputs) {
  ProgramBuilder out;
  // Disable auto-cloning of symbols, since we want to rename them.
  CloneContext ctx(&out, in, false);

  // Swizzles, intrinsic calls and builtin structure members need to keep their
  // symbols preserved.
  std::unordered_set<ast::IdentifierExpression*> preserve;
  for (auto* node : in->ASTNodes().Objects()) {
    if (auto* member = node->As<ast::MemberAccessorExpression>()) {
      auto* sem = in->Sem().Get(member);
      if (!sem) {
        TINT_ICE(Transform, out.Diagnostics())
            << "MemberAccessorExpression has no semantic info";
        continue;
      }
      if (sem->Is<sem::Swizzle>()) {
        preserve.emplace(member->member);
      } else if (auto* str_expr = in->Sem().Get(member->structure)) {
        if (auto* ty = str_expr->Type()->UnwrapRef()->As<sem::Struct>()) {
          if (ty->Declaration() == nullptr) {  // Builtin structure
            preserve.emplace(member->member);
          }
        }
      }
    } else if (auto* call = node->As<ast::CallExpression>()) {
      auto* sem = in->Sem().Get(call);
      if (!sem) {
        TINT_ICE(Transform, out.Diagnostics())
            << "CallExpression has no semantic info";
        continue;
      }
      if (sem->Target()->Is<sem::Intrinsic>()) {
        preserve.emplace(call->func);
      }
    }
  }

  Data::Remappings remappings;

  Target target = Target::kAll;

  if (auto* cfg = inputs.Get<Config>()) {
    target = cfg->target;
  }

  ctx.ReplaceAll([&](Symbol sym_in) {
    auto name_in = ctx.src->Symbols().NameFor(sym_in);
    switch (target) {
      case Target::kAll:
        // Always rename.
        break;
      case Target::kGlslKeywords:
        if (!std::binary_search(
                kReservedKeywordsGLSL,
                kReservedKeywordsGLSL +
                    sizeof(kReservedKeywordsGLSL) / sizeof(const char*),
                name_in)) {
          // No match, just reuse the original name.
          return ctx.dst->Symbols().New(name_in);
        }
        break;
      case Target::kHlslKeywords:
        if (!std::binary_search(
                kReservedKeywordsHLSL,
                kReservedKeywordsHLSL +
                    sizeof(kReservedKeywordsHLSL) / sizeof(const char*),
                name_in)) {
          // No match, just reuse the original name.
          return ctx.dst->Symbols().New(name_in);
        }
        break;
      case Target::kMslKeywords:
        if (!std::binary_search(
                kReservedKeywordsMSL,
                kReservedKeywordsMSL +
                    sizeof(kReservedKeywordsMSL) / sizeof(const char*),
                name_in)) {
          // No match, just reuse the original name.
          return ctx.dst->Symbols().New(name_in);
        }
        break;
    }

    auto sym_out = ctx.dst->Sym();
    remappings.emplace(name_in, ctx.dst->Symbols().NameFor(sym_out));
    return sym_out;
  });

  ctx.ReplaceAll(
      [&](ast::IdentifierExpression* ident) -> ast::IdentifierExpression* {
        if (preserve.count(ident)) {
          auto sym_in = ident->symbol;
          auto str = in->Symbols().NameFor(sym_in);
          auto sym_out = out.Symbols().Register(str);
          return ctx.dst->create<ast::IdentifierExpression>(
              ctx.Clone(ident->source), sym_out);
        }
        return nullptr;  // Clone ident. Uses the symbol remapping above.
      });
  ctx.Clone();

  return Output(Program(std::move(out)),
                std::make_unique<Data>(std::move(remappings)));
}

}  // namespace transform
}  // namespace tint
