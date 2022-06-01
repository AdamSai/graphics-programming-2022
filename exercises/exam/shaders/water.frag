#version 330 core

//uniform float a;
//uniform float amplitude;
//uniform float grid_points;
//
//uniform sampler2D z_tex;
//uniform sampler2D old_z_tex;
//
//uniform sampler2D collision_texture;
//uniform sampler2D old_collision_texture;
//
//uniform sampler2D land_texture;

uniform sampler2D heightmapTexture;
uniform sampler2D old_heightmapTexture;
uniform vec2 mousePos;
uniform float time;

in vec4 FragPos;
layout(location = 0) out vec3 color;
in vec2 UV;


void main()
{

    color = texture(heightmapTexture, UV).rgb;


}

