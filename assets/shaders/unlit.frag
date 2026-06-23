#version 450 core
out vec4 FragColor;

in vec2 v_texCoords;
in vec3 v_color;

uniform sampler2D u_textureDiffuse1;

void main()
{
    vec4 texColor = texture(u_textureDiffuse1, v_texCoords) * vec4(v_color, 1.0);
    if (texColor.a < 0.1)
        discard;
    FragColor = texColor;
}
