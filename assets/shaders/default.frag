#version 450 core
out vec4 FragColor;

in vec2 v_texCoords;
in vec3 v_color;
in vec3 v_fragPos;
in vec3 v_normal;

in vec4 v_fragPosLightSpace;

struct DirLight 
{
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
};

uniform sampler2D u_textureDiffuse1;
uniform sampler2D u_shadowMap;
uniform DirLight u_dirLight;
uniform vec3 u_viewPos;

vec3 calculateDirLight();
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

void main()
{    
    vec3 directionalLight = calculateDirLight();
    FragColor = vec4(directionalLight, 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        return 0.0;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    return shadow;
}

vec3 calculateDirLight()
{
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(u_dirLight.direction);
    vec3 viewDir = normalize(u_viewPos - v_fragPos);

    vec3 objectColor = (texture(u_textureDiffuse1, v_texCoords) * vec4(v_color, 1.0)).rgb;

    // ambient
    vec3 ambient = u_dirLight.ambient * u_dirLight.color;

    // toon diffuse
    float diffIntensity = max(dot(normal, lightDir), 0.0);
    float diff;
    if (diffIntensity > 0.5)      diff = 1.0;
    else if (diffIntensity > 0.1) diff = 0.6;
    else                          diff = 0.3;
    vec3 diffuse = u_dirLight.diffuse * diff * u_dirLight.color;

    // toon specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specIntensity = max(dot(normal, halfwayDir), 0.0);
    float spec = step(0.9, specIntensity); // sharp threshold
    vec3 specular = u_dirLight.specular * spec * u_dirLight.color;

    float shadow = ShadowCalculation(v_fragPosLightSpace, normal, lightDir);
    vec3 finalLight = ambient + (1.0 - shadow) * (diffuse + specular);
    
    return finalLight * objectColor;
}

