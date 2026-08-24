Texture2D sourceTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer OpticalParams : register(b0)
{
    float sphere;
    float cylinder;
    float axisRad;
    float distanceCm;
    float deconv;
    float contrast;
    float marginRatio;
    float reserved;
};

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    float distFactor = 60.0 / max(30.0, distanceCm);
    float sphereScale = clamp(1.0 + (-sphere) * 0.010 * distFactor, 0.86, 1.14);
    float cylRatio = clamp(1.0 + abs(cylinder) * 0.022 * distFactor, 0.90, 1.12);

    float scaleAlongAxis = marginRatio * sphereScale * cylRatio;
    float scaleAcrossAxis = marginRatio * sphereScale / cylRatio;

    float2 p = uv - 0.5;
    float s = sin(axisRad);
    float c = cos(axisRad);

    float2 q;
    q.x = c * p.x - s * p.y;
    q.y = s * p.x + c * p.y;

    q.x /= scaleAlongAxis;
    q.y /= scaleAcrossAxis;

    float2 src;
    src.x = c * q.x + s * q.y;
    src.y = -s * q.x + c * q.y;
    src += 0.5;

    if (src.x < 0.0 || src.x > 1.0 || src.y < 0.0 || src.y > 1.0) {
        return float4(0.0, 0.0, 0.0, 1.0);
    }

    float4 color = sourceTexture.Sample(linearSampler, src);
    color.rgb = saturate((color.rgb - 0.5) * contrast + 0.5);
    return color;
}
