//頂点シェーダー
//頂点データが1頂点ごとに呼び出され、posに渡される

struct Output
{
	float4 pos : POSITION;
	float4 svpos : SV_POSITION;
};

//POSITIONは座標・セマンティクス
float4 BasicVS(float4 pos : POSITION):  SV_POSITION
{
	return pos;
}