#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization

// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform int color_xy;
uniform vec4 solid_color;

void main()
{
	switch(color_xy){
	case 1:
		fragColor = vec4(tc.xxx,1);
		break;
	case 2:
		fragColor = vec4(tc.yyy,1);
		break;
	case 3:
		fragColor = vec4(tc.xy,0,1);
		break;
	default:
		fragColor = vec4(1,0,0,1);
	}
}