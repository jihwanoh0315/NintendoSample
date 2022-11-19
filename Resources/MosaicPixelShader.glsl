
layout( std140, binding = 1 ) uniform Model
{
    uniform mat4 u_userMatrix;
    uniform vec4 u_color0;
    uniform vec4 u_color1;
    uniform vec2 u_uv_src;
    uniform vec2 u_uv_size;
    uniform vec4 u_layer;
    float rate;
};


// ユーザー定義のコンスタントバッファ
layout(std140, binding = 3) uniform TextureParam
{
    uniform vec2        u_pixel_size;
};

layout( binding = 0 ) uniform sampler2D texture0;

layout( location = 0 ) in vec4 v_texCoord;
layout( location = 1 ) in vec4 v_color;
layout( location = 2 ) in vec4 v_normal;

out vec4 o_Color;

void main()
{
    vec4 out_color = vec4( 0 );
    vec2 uv = v_texCoord.xy;
    vec2 baseUv = floor( uv / (u_pixel_size * 4.0) );
    vec4 color0 = vec4( 0 );
    for (float x = 0.5; x < 4.0; x+=1.0) {
        for (float y = 0.5; y < 4.0; y+=1.0) {
            color0 += textureLod(texture0, u_pixel_size * (baseUv * 4.0 + vec2(x, y)), 0.0 );
        }
    }
    color0 /= 16.0;
    o_Color = color0;
}
