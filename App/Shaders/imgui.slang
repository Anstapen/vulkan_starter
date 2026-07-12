struct VSInput {
    float2 pos;
    float2 uv;
    float4 color;
};

struct VSOutput {
    float4 pos : SV_Position;
    float4 color;
    float2 uv;
};

struct PushConstants {
    float2 scale;
    float2 translate;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.color = input.color;
    output.uv = input.uv;
    output.pos = float4(input.pos * pc.scale + pc.translate, 0.0, 1.0);
    return output;
}

[[vk::binding(0, 0)]]
Sampler2D fontTexture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return vertIn.color * fontTexture.Sample(vertIn.uv);
}
