#version 130

in vec4 vertex;
in vec2 uv0;

out vec2 oUV;

void main(void)
{
	gl_Position = vertex;
    oUV = uv0;
}
