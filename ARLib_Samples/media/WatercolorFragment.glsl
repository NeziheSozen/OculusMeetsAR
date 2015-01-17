#version 130

uniform sampler2D Blurred;
uniform sampler2D Original;
uniform vec2 PixelSize;
uniform float EdgeThreshold;

in vec2 oUV0;
in vec2 oUV1;

out vec4 fragColor;

vec3 sample(vec2 uv, vec2 offset)
{
	return texture(Original, uv + PixelSize * offset).rgb;
}

float isEdge(vec2 uv)
{
	uv = oUV1 + PixelSize * uv;
	
#if 0
	// sobel
	vec3 sumX = vec3(0);
	sumX += sample(uv, vec2(-1, -1)) * 1.0;
	sumX += sample(uv, vec2(-1,  0)) * 2.0;
	sumX += sample(uv, vec2(-1,  1)) * 1.0;
	sumX -= sample(uv, vec2( 1, -1)) * 1.0;
	sumX -= sample(uv, vec2( 1,  0)) * 2.0;
	sumX -= sample(uv, vec2( 1,  1)) * 1.0;
	vec3 sumY = vec3(0);
	sumY += sample(uv, vec2(-1, -1)) * 1.0;
	sumY += sample(uv, vec2( 0, -1)) * 2.0;
	sumY += sample(uv, vec2( 1, -1)) * 1.0;
	sumY -= sample(uv, vec2(-1,  1)) * 1.0;
	sumY -= sample(uv, vec2( 0,  1)) * 2.0;
	sumY -= sample(uv, vec2( 1,  1)) * 1.0;
	
	vec3 result = sqrt(sumX * sumX + sumY * sumY);
#else
	vec3 sumX = vec3(0);
	sumX += sample(uv, vec2(-1, -1));
	sumX += sample(uv, vec2(-1,  0));
	sumX -= sample(uv, vec2( 0, -1));
	sumX -= sample(uv, vec2( 0,  0));
	vec3 sumY = vec3(0);
	sumY += sample(uv, vec2(-1, -1));
	sumY += sample(uv, vec2( 0, -1));
	sumY -= sample(uv, vec2(-1,  0));
	sumY -= sample(uv, vec2( 0,  0));
	
	vec3 result = sqrt(sumX * sumX + sumY * sumY);
#endif

	bool edge = any(greaterThan(result, vec3(EdgeThreshold)));
	return edge ? 1.0 : 0.0;
}

void main(void)
{
	/*float amount = 1.0 - 0.2 * (
		isEdge(vec2( 0,  0)) +
		isEdge(vec2( 0, -1)) +
		isEdge(vec2(-1,  0)) +
		isEdge(vec2( 0,  1)) +
		isEdge(vec2( 1,  0)));
	
	if (amount < 0.1)
		amount = 0.0;
	else
		amount = 1.0;*/
		
	float amount = 1.0 - isEdge(vec2( 0,  0));
	
	fragColor = vec4(amount * texture(Blurred, oUV0).rgb, 1.0);
}
