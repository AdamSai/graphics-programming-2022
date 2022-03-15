#version 330 core

uniform vec3 camPosition;// so we can compute the view vector
out vec4 FragColor;// the output color of this fragment

// TODO exercise 5.4 setup the 'uniform' variables needed for lighting
// light uniforms
uniform vec3 ambientLightColor;
uniform vec3 reflectionColor;
uniform float ambientReflectance;

uniform vec3 light1Position;
uniform vec3 light2Position;
uniform vec3 light1Color;
uniform vec3 light2Color;
uniform float diffuseReflectance;

uniform float specularReflectance;
uniform float specularExponent;
// material uniforms

// TODO exercise 5.4 add the 'in' variables to receive the interpolated Position and Normal from the vertex shader
in vec3 N;
in vec4 P;

void main()
{

    // TODO exercise 5.4 - phong shading (i.e. Phong reflection model computed in the fragment shader)
    // ambient component
    vec3 ambient = ambientLightColor * ambientReflectance * reflectionColor;
    // diffuse component for light 1
    vec3 L = normalize(light1Position - P.xyz);
    float angle = dot(N, L);
    vec3 diffuse = light1Color * diffuseReflectance * angle;

    vec3 L2 = normalize(light2Position - P.xyz);
    float angle2 = dot(N, L2);
    vec3 diffuse2 = light2Color * diffuseReflectance * angle2;

    // specular component for light 1
    vec3 R = 2 * N * angle - L;
    vec3 V = normalize(camPosition - P.xyz);

    vec3 H = normalize(L + V);
    vec3 specular = light1Color * specularReflectance * (pow(dot(N, R), specularExponent));

    vec3 R2 = 2 * N * angle2 - L2;
    vec3 H2 = normalize(L2 + V);
    vec3 specular2 = light2Color * specularReflectance * (pow(dot(N, R2), specularExponent));
    // TODO exercuse 5.5 - attenuation - light 1
    float light1Distance = distance(light1Position, P.xyz);
    float light1Attenuation = 1 / pow(light1Distance, 2);
    // TODO exercise 5.6 - multiple lights, compute diffuse, specular and attenuation of light 2
    float light2Distance = distance(light2Position, P.xyz);
    float light2Attenuation = 1 / pow(light2Distance, 2);

    // TODO compute the final shaded color (e.g. add contribution of the attenuated lights 1 and 2)


    // TODO set the output color to the shaded color that you have computed
    FragColor = vec4(ambient + (diffuse + specular) * light1Attenuation + (diffuse2 + specular2) * light2Attenuation, 1.0);
}
