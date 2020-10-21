#version 150

uniform mat4 mvp_matrix;
uniform sampler2D heightmap;

in vec4 a_position;
in vec2 a_texcoord;

out vec2 v_texcoord;
out vec4 v_position;

//! [0]
void main()
{
    v_texcoord = a_texcoord;
    vec4 position;
    position = a_position;
    position.z = texture2D(heightmap, v_texcoord).z;
    // Calculate vertex position in screen space
    gl_Position = mvp_matrix * position;

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_position = position;
}
//! [0]
