
layout( binding = 0 ) uniform sampler2D texture0;

layout( location = 0 ) in vec4 v_texCoord;
layout( location = 1 ) in vec4 v_color;
layout( location = 2 ) in vec4 v_normal;
out vec4 o_Color;

void main()
{
    vec2 uv = v_texCoord.xy;
    vec4 color0 = vec4( 0 );
    color0 = textureLod(texture0, uv, 0.0 );

    float rate = clamp( v_normal.y + 0.2 * 0.5, 0.1, 1.0 );
    rate = floor( rate * 10 ) / 10;

    color0 = color0 * v_color * vec4(vec3(rate), 1.0);
    o_Color = vec4( color0.rgb, 1.0f );
}
