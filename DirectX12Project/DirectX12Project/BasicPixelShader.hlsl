//ピクセルシェーダー
//頂点シェーダーから受け取った座標を受け取り、色を返す

float4 BasicPS(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(1,1,1,1);
}