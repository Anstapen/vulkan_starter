struct VSInput {
    float2 inPos;
    float2 inTexCoord;
    uint instanceIndex : SV_InstanceID;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
    float4   cameraRight;
    float4   cameraPos;
};
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct TextureInstance {
    float3 pos;
    float  tilt;
    float2 scale;
    float  rotation;
    uint   textureIndex;
    uint   isBillboard;
    float  uvScale;
    uint   emitsLight;
    float  pad0;
};
[[vk::binding(0, 2)]]
StructuredBuffer<TextureInstance> texture_instances;

struct VSOutput
{
    float4 pos : SV_Position;
    float2 fragTexCoord;
    nointerpolation uint textureIndex : TEXCOORD1;
    uint emitsLight;
    float3 worldPos;
    float3 worldNormal;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    TextureInstance t = texture_instances[input.instanceIndex];

    float3 worldPos;
    float2 uv = input.inTexCoord;

    if (t.isBillboard != 0)
    {
        // Upright (cylindrical) billboard: width runs along the camera's screen-right axis,
        // height runs straight up (world +Z), with the sprite's base planted at t.pos. The height is
        // NOT flipped here (local +y -> world +z) so the CCW winding keeps the front face toward the
        // camera and back-face culling doesn't hide it; the texture is flipped in uv instead to keep
        // image-up = world-up.
        float3 right = ubo.cameraRight.xyz;
        float3 up    = float3(0.0, 0.0, 1.0);

        float halfWidth = input.inPos.x * t.scale.x;         // centered on x, in [-0.5,0.5]*scale
        float height    = (input.inPos.y + 0.5) * t.scale.y; // 0 at base .. scale.y at top

        worldPos = t.pos + halfWidth * right + height * up;
        output.worldPos = worldPos;
        output.worldNormal = normalize(cross(right, up));
        uv       = float2(input.inTexCoord.x, 1.0 - input.inTexCoord.y);
    }
    else
    {
        // Flat quad lying in the world x/y plane (the ground); tile the texture.
        worldPos = t.pos + float3(input.inPos * t.scale, 0.0);
        output.worldPos = worldPos;
        output.worldNormal = float3(0.0, 0.0, 1.0);
        uv       = input.inTexCoord * t.uvScale;
    }

    output.pos          = mul(ubo.proj, mul(ubo.view, float4(worldPos, 1.0)));
    output.fragTexCoord = uv;
    output.textureIndex = t.textureIndex;
    output.emitsLight = t.emitsLight;
    return output;
}

static const uint kMaxTextures = 256;
[[vk::binding(0, 1)]]
Sampler2D textures[kMaxTextures];

struct LightInstance {
    float3 color;
    float  ambientLight;
    float3 pos;
    float _pad1;
};
[[vk::binding(0, 3)]]
StructuredBuffer<LightInstance> light_instances;

struct LightParams {
	uint numLights;
	float	 pad0;
	float	 pad1;
    float    pad2;
};
[[vk::binding(0, 4)]]
ConstantBuffer<LightParams> light_params;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
   
    if(vertIn.emitsLight != 0)
    {
        // the fragment is part of a geometry that emits light, skip light calculations
        return textures[NonUniformResourceIndex(vertIn.textureIndex)].Sample(vertIn.fragTexCoord);
    } else
    {
        float4 tex_color = textures[NonUniformResourceIndex(vertIn.textureIndex)].Sample(vertIn.fragTexCoord);
        var ambient = float3(0.0f);
        var diffuse = float3(0.0f);
        var specular = float3(0.0f);
        for(uint i = 0; i < light_params.numLights; i++)
        {
            //float3 ambient = light_instances[i].color;
            ambient  += light_instances[i].color * light_instances[i].ambientLight;

            float3 lightDir = normalize(light_instances[i].pos - vertIn.worldPos);
            float diff = max(dot(vertIn.worldNormal, lightDir), 0.0);
            diffuse += light_instances[i].color * diff;

            float3 viewDir = normalize(ubo.cameraPos.xyz - vertIn.worldPos);
            float3 reflectDir = reflect(-lightDir, vertIn.worldNormal);

            specular += pow(max(dot(viewDir, reflectDir), 0.0), 32);
        }

        tex_color.xyz = (ambient + diffuse + specular) * tex_color.xyz;


        
        //return float4(rgb_color, tex_color.w);
        return float4(tex_color.rgb, tex_color.a);
    }

   //return textures[nonuniform(textures[vertIn.textureIndex])].Sample(vertIn.fragTexCoord);
}