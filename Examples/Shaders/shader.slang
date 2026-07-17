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

struct TextureInstance {
    float3 pos;
    float  tilt;
    float2 scale;
    float  rotation;
    uint   textureIndex;
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

    float c = cos(t.rotation);
    float s = sin(t.rotation);

    float3 localPos = float3(input.inPos, 0.0);

    // yaw: existing in-plane rotation, now expressed as a 3x3 around Z
    float cy = cos(t.rotation), sy = sin(t.rotation);
    float3x3 yawMat = float3x3(
        cy, -sy, 0.0,
        sy,  cy, 0.0,
        0.0, 0.0, 1.0);

    // tilt: new rotation around X, tips the quad up out of the XY plane
    float ct = cos(t.tilt), st = sin(t.tilt);
    float3x3 tiltMat = float3x3(
        1.0, 0.0,  0.0,
     0.0,  ct, -st,
        0.0,  st,  ct);

    float3 scaled  = localPos * float3(t.scale, 1.0);
    float3 rotated = mul(yawMat, mul(tiltMat, scaled));

    float4 worldPos = float4(rotated + t.pos, 1.0);

    output.pos = mul(ubo.proj, mul(ubo.view,  worldPos));
    output.fragTexCoord = input.inTexCoord;
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