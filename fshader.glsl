#version 150

uniform sampler2D textureG;
uniform sampler2D textureR;
uniform sampler2D textureS;

in vec2 v_texcoord;
in vec4 v_position;
//! [0]
void main()
{
    // Set fragment color from texture
    if(v_position.z < 0.07){
        gl_FragColor = texture2D(textureG, v_texcoord);
    }
    else if(v_position.z >= 0.07 && v_position.z < 0.12){
        gl_FragColor = texture2D(textureR, v_texcoord);
    }
    else{
        gl_FragColor = texture2D(textureS, v_texcoord);
    }
}
//! [0]

