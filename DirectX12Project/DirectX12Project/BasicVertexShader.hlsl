//頂点シェーダー
//頂点データが1頂点ごとに呼び出され、posに渡される

//POSITIONは座標・セマンティクス
float4 BasicVS(float4 pos : POSITION):  SV_POSITION
{
	return pos;
}