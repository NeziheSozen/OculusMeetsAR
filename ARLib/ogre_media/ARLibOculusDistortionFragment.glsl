#version 130

uniform sampler2D undistorted;

in vec2 oUVR;
in vec2 oUVG;
in vec2 oUVB;
in vec4 oColor;

out vec4 fragColor;

void main()
{
	float r = texture(undistorted, oUVR).r;
	float g = texture(undistorted, oUVG).g;
	float b = texture(undistorted, oUVB).b;
	fragColor = vec4(r, g, b, 1.0) * oColor;
}
