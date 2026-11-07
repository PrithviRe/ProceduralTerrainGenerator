#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in float Height;

out vec4 FragColor;

uniform vec3 u_CamPos;
uniform vec3 u_SunDir;
uniform float u_Amplitude;
uniform int u_WireframeMode;

void main() {
    if (u_WireframeMode == 1) {
        FragColor = vec4(0.2, 0.85, 0.3, 1.0);
        return;
    }

    vec3 N = normalize(Normal);
    vec3 L = normalize(u_SunDir);
    vec3 V = normalize(u_CamPos - FragPos);
    vec3 H = normalize(L + V);

    // Height ratio between -amplitude and +amplitude
    float hNorm = clamp((Height + u_Amplitude * 0.8) / (u_Amplitude * 1.6), 0.0, 1.0);

    // Steepness: N.y is 1.0 for flat ground, approaches 0.0 for vertical cliffs
    float slope = clamp(N.y, 0.0, 1.0);

    // Biome colors
    vec3 waterColor = vec3(0.12, 0.35, 0.65);
    vec3 sandColor  = vec3(0.85, 0.78, 0.55);
    vec3 grassColor = vec3(0.28, 0.58, 0.22);
    vec3 rockColor  = vec3(0.48, 0.44, 0.40);
    vec3 snowColor  = vec3(0.95, 0.96, 0.98);

    // Blend based on elevation
    vec3 biomeColor = sandColor;
    if (hNorm < 0.22) {
        biomeColor = mix(waterColor, sandColor, smoothstep(0.05, 0.22, hNorm));
    } else if (hNorm < 0.55) {
        biomeColor = mix(sandColor, grassColor, smoothstep(0.22, 0.35, hNorm));
    } else if (hNorm < 0.78) {
        biomeColor = mix(grassColor, rockColor, smoothstep(0.55, 0.72, hNorm));
    } else {
        biomeColor = mix(rockColor, snowColor, smoothstep(0.78, 0.88, hNorm));
    }

    // Rocky cliffs on steep slopes (rock exposed regardless of height)
    if (slope < 0.72 && hNorm > 0.25) {
        float cliffFactor = 1.0 - smoothstep(0.45, 0.72, slope);
        biomeColor = mix(biomeColor, rockColor * 0.85, cliffFactor);
    }

    // Directional Sun Lighting
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 ambient = vec3(0.22, 0.28, 0.38) * biomeColor;
    vec3 sunLight = vec3(1.0, 0.95, 0.85) * diff * biomeColor;
    vec3 specularLight = vec3(0.9, 0.95, 1.0) * spec * (hNorm > 0.8 ? 0.4 : 0.05);

    vec3 finalColor = ambient + sunLight + specularLight;

    // Atmospheric depth fog
    float dist = length(u_CamPos - FragPos);
    float fog = 1.0 - exp(-0.00008 * dist * dist);
    vec3 fogColor = vec3(0.65, 0.75, 0.88);

    finalColor = mix(finalColor, fogColor, clamp(fog, 0.0, 1.0));

    // Subtle gamma correction
    finalColor = pow(finalColor, vec3(1.0 / 2.0));

    FragColor = vec4(finalColor, 1.0);
}

