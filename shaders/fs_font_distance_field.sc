$input v_color0, v_texcoord0

#include "common.sh"

SAMPLER2D(u_texColor, 0);


void main()
{	
    float distance = texture2D(u_texColor, v_texcoord0.xy).r;
  	
  	//float smoothing = fwidth(distance);
  	// Perform anisotropic analytic antialiasing (fwidth() is slightly wrong)
    //float smoothing = length(vec2(dFdx(distance), dFdy(distance)));
    float smoothing = distance/16.0;
    //const float smoothing = 1.0/16.0;
    float alpha = smoothstep(0.5 - smoothing, 0.5 , distance);

    //if(distance < 0.5) alpha = 0.0;
    //else alpha = 1.0;
    //const float smooth_min = 0.45;
    //const float smooth_max = 0.55;    
    //float alpha = smoothstep(smooth_min, smooth_max, distance);
    gl_FragColor = vec4(v_color0.rgb, alpha);
}
