struct VSInput {
    float2 inPos;
    uint instanceIndex : SV_InstanceID;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
};
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct LineInstance {
    float3 pos;
    float  _pad0;
    float  rotation;
    float  length;
    float  width;
    float  _pad1;
    float4 color;
};
[[vk::binding(0, 1)]]
StructuredBuffer<LineInstance> lines;

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    LineInstance l = lines[input.instanceIndex];

    float2 scaled = float2(input.inPos.x * l.length, input.inPos.y * l.width);

    float c = cos(l.rotation);
    float s = sin(l.rotation);
    float2 rotated = float2(
        scaled.x * c - scaled.y * s,
        scaled.x * s + scaled.y * c);

    float4 worldPos = float4(rotated + l.pos.xy, l.pos.z, 1.0);

    output.pos = mul(ubo.proj, mul(ubo.view, worldPos));
    output.color = l.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return vertIn.color;
}