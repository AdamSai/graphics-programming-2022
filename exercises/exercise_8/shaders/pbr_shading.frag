#version 330 core

uniform vec3 camPosition;// so we can compute the view vector
out vec4 FragColor;// the output color of this fragment

// light uniform variables
uniform vec4 ambientLightColor;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRadius;

// material properties
uniform vec3 reflectionColor;
uniform float roughness;
uniform float metalness;
// legacy uniforms, not needed for PBR
uniform float ambientReflectance;
uniform float diffuseReflectance;
uniform float specularReflectance;
uniform float specularExponent;

// material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ambient1;
uniform sampler2D texture_specular1;
uniform samplerCube skybox;
uniform sampler2D shadowMap;

// 'in' variables to receive the interpolated Position and Normal from the vertex shader
in vec4 worldPos;
in vec3 worldNormal;
in vec3 worldTangent;
in vec2 textureCoordinates;

// TODO 8.1 : Add an 'in' variable for vertex position in light space
in vec4 lightspacePos;


// Constant Pi
const float PI = 3.14159265359;


// Schlick approximation of the Fresnel term
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
    // TODO 8.4 : Implement the formula here
    vec3 fresnel = F0 + (1 - F0) * pow(1 - cosTheta, 5);
    return fresnel;
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
    // TODO 8.5 : Implement the formula here

    //    pow(N * H, 2)
    //    (pow(a, 2) - 1) + 1
    return pow(a, 2) / (PI * pow(pow(dot(N, H), 2) * (pow(a, 2) - 1) + 1, 2));
}

float GeometrySchlickGGX(float cosAngle, float a)
{
    // TODO 8.6 : Implement the formula here
    float aSquared = pow(a, 2);

    return 2 * cosAngle / (cosAngle + sqrt(aSquared + (1 - aSquared) * pow(cosAngle, 2)));
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float a)
{
    // TODO 8.6 : Implement the formula here, using GeometrySchlickGGX
    //    return
    return GeometrySchlickGGX(dot(N, L), a) * GeometrySchlickGGX(dot(N, V), a);
}

vec3 GetCookTorranceSpecularLighting(vec3 N, vec3 L, vec3 V)
{
    vec3 H = normalize(L + V);

    // Remap alpha parameter to roughness^2
    float a = roughness * roughness;

    float D = DistributionGGX(N, H, a);
    float G = GeometrySmith(N, V, L, a);

    float cosI = max(dot(N, L), 0.0);
    float cosO = max(dot(N, V), 0.0);

    // Important! Notice that Fresnel term (F) is not here because we apply it later when mixing with diffuse
    float specular = (D * G) / (4.0f * cosO * cosI + 0.0001f);

    return vec3(specular);
}



vec3 GetNormalMap()
{
    //NEW! Normal map

    // Sample normal map
    vec3 normalMap = texture(texture_normal1, textureCoordinates).rgb;
    // Unpack from range [0, 1] to [-1 , 1]
    normalMap = normalMap * 2.0 - 1.0;

    // This time we are storing Z component in the texture, no need to compute it. Instead we normalize just in case
    normalMap = normalize(normalMap);

    // Create tangent space matrix
    vec3 N = normalize(worldNormal);
    vec3 B = normalize(cross(worldTangent, N));// Orthogonal to both N and T
    vec3 T = cross(N, B);// Orthogonal to both N and B. Since N and B are normalized and orthogonal, T is already normalized
    mat3 TBN = mat3(T, B, N);

    // Transform normal map from tangent space to world space
    return TBN * normalMap;
}

vec3 GetAmbientLighting(vec3 albedo, vec3 normal)
{
    // TODO 8.2 : Get the ambient color by sampling the environment mapping using the normal.
    vec3 ambient = textureLod(texture_ambient1, textureCoordinates * normal.xy, 6).rgb;
    // TODO 8.2 : Scale the light by the albedo, considering also that it gets reflected equally in all directions
    ambient *= albedo;
    ambient /= PI;
    // Only apply ambient during the first light pass
    ambient *= ambientLightColor.a;

    // NEW! Ambient occlusion (try disabling it to see how it affects the visual result)
    float ambientOcclusion = texture(texture_ambient1, textureCoordinates).r;
    ambient *= ambientOcclusion;

    return ambient;
}

vec3 GetEnvironmentLighting(vec3 N, vec3 V)
{
    //NEW! Environment reflection

    // Compute reflected light vector (R)
    vec3 R = reflect(-V, N);

    // Sample cubemap
    // HACK: We sample a different mipmap depending on the roughness. Rougher surface will have blurry reflection
    vec3 reflection = textureLod(skybox, R, roughness * 5.0f).rgb;

    // We packed the amount of reflection in ambientLightColor.a
    // Only apply reflection (and ambient) during the first light pass
    reflection *= ambientLightColor.a;

    return reflection;
}

vec3 GetLambertianDiffuseLighting(vec3 N, vec3 L, vec3 albedo)
{
    //    vec3 diffuse = diffuseReflectance * albedo;

    // TODO 8.3 : Scale the diffuse light, considering that it gets reflected equally in all directions
    vec3 diffuse = albedo / PI;

    return diffuse;
}

vec3 GetBlinnPhongSpecularLighting(vec3 N, vec3 L, vec3 V)
{
    vec3 H = normalize(L + V);

    float specModulation = pow(max(dot(H, N), 0.0), specularExponent);
    vec3 specular = vec3(specularReflectance) * specModulation;

    return specular;
}

float GetAttenuation(vec4 P)
{
    float distToLight = distance(lightPosition, P.xyz);
    float attenuation = 1.0f / (distToLight * distToLight);

    float falloff = smoothstep(lightRadius, lightRadius*0.5f, distToLight);

    return attenuation * falloff;
}
// https://stackoverflow.com/questions/33975576/shadow-mapping-transforming-a-view-space-position-to-the-shadow-map-space
float GetShadow()
{
    // TODO 8.1 : Transform the position in light space to shadow map space: from range (-1, 1) to range (0, 1)
    vec4 shadowMapSpace = lightspacePos * vec4(0.5f) + vec4(0.5f);
    shadowMapSpace.z = clamp(shadowMapSpace.z, -1, 1);
    // TODO 8.1 : Sample the shadow map texture using the XY components of the light in shadow map space
    float depth = texture(shadowMap, shadowMapSpace.xy).r + 0.01;

    // TODO 8.1 : Compare the depth value obtained with the Z component of the light in shadow map space. Return 0 if depth is smaller or equal, 1 otherwise
    if (depth <= shadowMapSpace.z){
        return 0.0f;
    }
    else {
        return 1.0f;
    }
    return 1.0f;
}


void main()
{
    vec4 P = worldPos;

    vec3 N = GetNormalMap();

    vec3 albedo = texture(texture_diffuse1, textureCoordinates).xyz;
    albedo *= reflectionColor;

    bool positional = lightRadius > 0;

    vec3 L = normalize(lightPosition - (positional ? P.xyz : vec3(0.0f)));
    vec3 V = normalize(camPosition - P.xyz);

    vec3 ambient = GetAmbientLighting(albedo, N);
    vec3 environment = GetEnvironmentLighting(N, V);

    vec3 diffuse = GetLambertianDiffuseLighting(N, L, albedo);

    // TODO 8.5 : Replace the Blinn-Phong with a call to the GetCookTorranceSpecularLighting function
    vec3 specular = GetCookTorranceSpecularLighting(N, L, V);

    // This time we get the lightColor outside the diffuse and specular terms (we are multiplying later)
    vec3 lightRadiance = lightColor;

    // Modulate lightRadiance by distance attenuation (only for positional lights)
    float attenuation = positional ? GetAttenuation(P) : 1.0f;
    lightRadiance *= attenuation;

    // Modulate lightRadiance by shadow (only for directional light)
    float shadow = positional ? 1.0f : GetShadow();
    lightRadiance *= shadow;

    // Modulate the radiance with the angle of incidence
    lightRadiance *= max(dot(N, L), 0.0);

    // We use a fixed value of 0.04f for F0. The range in dielectrics is usually in the range (0.02, 0.05)
    vec3 F0 = vec3(0.04f);

    // TODO 8.7 : Compute the new F0 as a mix between dielectric F0 and albedo using the metalness parameter
    F0 = mix(F0, albedo, metalness);

    // TODO 8.7 : Compute the new diffuse as a mix between diffuse and 0 using the metalness parameter. Same for ambient (diffuse indirect)
    diffuse = mix(diffuse, vec3(0), metalness);
    ambient = mix(ambient, vec3(0), metalness);

    // TODO 8.4 : Compute the Fresnel term for indirect light, using the clamped cosine of the angle formed by the NORMAL vector and the view vector
    float cosAngle = max(dot(N, V), 0);
    vec3 fresnelIndirect = FresnelSchlick(F0, cosAngle);

    // TODO 8.4 : Mix ambient and environment using the fresnel you just computed as blend factor
    vec3 indirectLight = mix(ambient, environment, fresnelIndirect);

    // TODO 8.4 : Compute the Fresnel term for the light, using the clamped cosine of the angle formed by the HALF vector and the view vector
    vec3 H = normalize(L + V);
    float cosAngleHalf = max(dot(H, V), 0);
    vec3 fresnelLight = FresnelSchlick(F0, cosAngleHalf);

    // TODO 8.4 : Use the fresnel you just computed as blend factor, instead of roughness. Pay attention to the order of the parameters in mix
    // TODO 8.3 : Instead of adding them, mix the specular and diffuse lighting using, for now, the roughness.
    vec3 directLight = mix(diffuse, specular, fresnelLight);
    directLight *= lightRadiance;

    // lighting = indirect lighting (ambient + environment) + direct lighting (diffuse + specular)
    vec3 lighting = indirectLight + directLight;

    FragColor = vec4(lighting, 1.0f);
}
