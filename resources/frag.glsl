#version 120

varying vec3 vertPos; // passed from the vertex shader
varying vec3 normal;  // passed from the vertex shader

void main()
{
    float intensity = 0.8f;
    vec3 ka = vec3(0.5, 0.5, 0.5);
    
    vec3 n = normalize(normal);
    vec3 e = normalize(vec3(0.0f, 0.0f, 0.0f) - vertPos);    // camera position - vertex position
    vec4 ca = vec4(vec3(0.05f, 0.4f, 0.2f), 1.0);
    
    vec3 l = normalize(vec3(0.0f, 0.0f, 0.0f) - vertPos);
    vec4 cd = vec4(vec3(0.0f, 0.1f, 0.8f) * max(0, dot(l, n)), 1.0);
    vec3 h = normalize(l + e);
    vec4 cs = vec4(vec3(1.0f, 0.9f, 0.8f) * pow(max(0, dot(h, n)), 200.0f), 1.0);

    gl_FragColor = intensity * (ca + cd + cs);
}
