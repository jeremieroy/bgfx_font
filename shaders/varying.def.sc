vec2 a_position  : POSITION;
vec4 a_color0    : COLOR0;
vec4 a_texcoord0 : TEXCOORD0;
//float a_shift  : TEXCOORD1;
//float a_gamma  : TEXCOORD2;

vec4 v_color0      : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);
vec4 v_texcoord0   : TEXCOORD0 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_sampleLeft  : TEXCOORD1 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_sampleRight : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);

//float v_shift : TEXCOORD1 = 0.5;
//float v_gamma : TEXCOORD2 = 0.5;

