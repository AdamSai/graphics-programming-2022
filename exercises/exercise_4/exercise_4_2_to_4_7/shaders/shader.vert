#version 330 core
layout (location = 0) in vec2 pos;// the position variable has attribute position 0
// TODO 4.2 add velocity and timeOfBirth as vertex attributes
layout (location = 1) in vec2 velocity;
layout (location = 2) in float timeOfBirth;
// TODO 4.3 create and use a float uniform for currentTime
uniform float currentTime;
// TODO 4.6 create out variable to send the age of the particle to the fragment shader
out float age;
float maxAge = 10;

void main()
{
    age = currentTime - timeOfBirth;
    float currentScale = mix(0.1, 20, mod(age, maxAge) / 20);
    // TODO 4.3 use the currentTime to control the particle in different stages of its lifetime
    if (timeOfBirth == 0)
    {
        gl_Position = vec4(-10, -10, 0.0, 1.0);
    }
    else if (timeOfBirth > 0)
    {
        vec2 newPos =  pos + (age * velocity);
        if (age > maxAge)
        {
            newPos = vec2(-10, -10);
        }
        gl_Position =  vec4(newPos, 0.0, 1.0);
    }

    // TODO 4.6 send the age of the particle to the fragment shader using the out variable you have created

    // this is the output position and and point size (this time we are rendering points, instad of triangles!)
    gl_PointSize = currentScale;
    //    gl_PointSize = 10;
    //    gl_Position = vec4(pos, 0.0, 1.0);

}