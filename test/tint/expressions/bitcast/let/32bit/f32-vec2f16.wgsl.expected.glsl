#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec2 tint_bitcast_to_f16(float src) {
  uint r = floatBitsToUint(src);
  return unpackFloat2x16(r);
}

void f() {
  float a = 2.003662109375f;
  f16vec2 b = tint_bitcast_to_f16(a);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
