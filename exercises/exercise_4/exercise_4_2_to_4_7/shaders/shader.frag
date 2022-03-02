#version 330 core

out vec4 fragColor;

// TODO 4.6: should receive the age of the particle as an input variable
in float age;
float maxAge = 10;
void main()
{
    // TODO 4.4 set the alpha value to 0.2 (alpha is the 4th value of the output color)
    // TODO 4.5 and 4.6: improve the particles appearance
    vec2 mid = vec2(0.5, 0.5);
    vec2 direction = mid - gl_PointCoord;
    float distance = sqrt(pow(direction.x, 2) + pow(direction.y, 2));

    float progress = age / maxAge;
    float alphaPct = clamp(1.0 - (age / maxAge), 0.0, 1.0);
    float alpha = mod(distance + 1, 1.5) * alphaPct;

    vec3 initialColor = vec3(1.0, 1.0, 0.05);
    vec3 midColor = vec3(1.0, 0.5, 0.05);
    vec3 color;
    if (age <= (maxAge / 2))
    {
        color = mix(initialColor, midColor, age / (maxAge / 2));
    }
    else
    {
        color = mix(midColor, vec3(0.0, 0.0, 0.0), age / maxAge);
    }
    // remember to replace the default output (vec4(1.0,1.0,1.0,1.0)) with the color and alpha values that you have computed
    fragColor = vec4(color, alpha);

}