struct VSInput {
    float2 inPos;
    float2 inTexCoord;
    uint instanceIndex : SV_InstanceID;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
    float4   cameraRight;
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
    float2 _pad;
};
[[vk::binding(0, 2)]]
StructuredBuffer<TextureInstance> texture_instances;

struct VSOutput
{
    float4 pos : SV_Position;
    float2 fragTexCoord;
    nointerpolation uint textureIndex : TEXCOORD1;
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
        uv       = float2(input.inTexCoord.x, 1.0 - input.inTexCoord.y);
    }
    else
    {
        // Flat quad lying in the world x/y plane (the ground); tile the texture.
        worldPos = t.pos + float3(input.inPos * t.scale, 0.0);
        uv       = input.inTexCoord * t.uvScale;
    }

    output.pos          = mul(ubo.proj, mul(ubo.view, float4(worldPos, 1.0)));
    output.fragTexCoord = uv;
    output.textureIndex = t.textureIndex;
    return output;
}

static const uint kMaxTextures = 256;
[[vk::binding(0, 1)]]
Sampler2D textures[kMaxTextures];

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
   return textures[NonUniformResourceIndex(vertIn.textureIndex)].Sample(vertIn.fragTexCoord);

   //return textures[nonuniform(textures[vertIn.textureIndex])].Sample(vertIn.fragTexCoord);
}