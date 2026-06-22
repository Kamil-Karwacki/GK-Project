#version 450 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoords;
layout (location = 3) in vec3 a_color;

out vec2 v_texCoords;
out vec3 v_color;
out vec3 v_fragPos;
out vec3 v_normal;
out vec4 v_fragPosLightSpace;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_lightSpaceMatrix;

void main()
{
    v_texCoords = a_texCoords;    
    v_color = a_color;
    v_fragPos = vec3(u_model * vec4(a_pos, 1.0f));
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_fragPosLightSpace = u_lightSpaceMatrix * vec4(v_fragPos, 1.0);

    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
}

