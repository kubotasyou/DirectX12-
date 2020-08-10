//ピクセルシェーダー
//頂点シェーダーから受け取った座標を受け取り、色を返す

struct Input
{
	float4 pos : POSITION;
	float4 svpos : SV_POSITION;
};

float4 BasicPS(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(0.5f, 0, 1, 0.1f);
}