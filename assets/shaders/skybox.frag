#version 450 core
out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 u_sunDir;

void main()
{
    vec3 dir = normalize(TexCoords);
    
    vec3 zenithColor = vec3(0.08, 0.35, 0.75);
    vec3 horizonColor = vec3(0.55, 0.78, 0.95);
    
    float val = max(dir.y, 0.0);
    vec3 skyColor = mix(horizonColor, zenithColor, pow(val, 0.6));
    
    if (dir.y < 0.0) {
        skyColor = mix(horizonColor, vec3(0.15, 0.15, 0.18), min(-dir.y * 8.0, 1.0));
    }
    
    float sunIntensity = max(dot(dir, normalize(u_sunDir)), 0.0);
    
    float sunDisk = smoothstep(0.993, 0.996, sunIntensity);
    
    float sunGlow = pow(sunIntensity, 32.0) * 0.4 + pow(sunIntensity, 128.0) * 0.6;
    
    vec3 sunColor = vec3(1.0, 0.98, 0.90);
    vec3 glowColor = vec3(1.0, 0.85, 0.60);
    
    vec3 finalColor = skyColor + sunDisk * sunColor + sunGlow * glowColor;
    
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    
    FragColor = vec4(finalColor, 1.0);
}
