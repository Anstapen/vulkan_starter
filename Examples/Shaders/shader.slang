struct VSInput {
    float2 inPos;
    float2 inTexCoord;
    uint instanceIndex : SV_InstanceID;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
};
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct Transform {
    float3 pos;
    float  _pad0;
    float2 scale;
    float  rotation;
    float  _padding;
};
[[vk::binding(0, 2)]]
StructuredBuffer<Transform> transforms;

struct VSOutput
{
    float4 pos : SV_Position;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    Transform t = transforms[input.instanceIndex];

    float c = cos(t.rotation);
    float s = sin(t.rotation);

    float2 scaled = input.inPos * t.scale;
    float2 rotated = float2(
        scaled.x * c - scaled.y * s,
        scaled.x * s + scaled.y * c);

    float4 worldPos = float4(rotated + t.pos.xy, t.pos.z, 1.0);

    output.pos = mul(ubo.proj, mul(ubo.view,  worldPos));
    output.fragTexCoord = input.inTexCoord;
    return output;
}

[[vk::binding(0, 1)]]
Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
   return texture.Sample(vertIn.fragTexCoord);
}