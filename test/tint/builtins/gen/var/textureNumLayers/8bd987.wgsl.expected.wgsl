@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32sint, write>;

fn textureNumLayers_8bd987() {
  var res : u32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_8bd987();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_8bd987();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_8bd987();
}