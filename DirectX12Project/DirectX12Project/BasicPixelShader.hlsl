//�s�N�Z���V�F�[�_�[
//���_�V�F�[�_�[����󂯎�������W���󂯎��A�F��Ԃ�

struct Input
{
	float4 pos : POSITION;
	float4 svpos : SV_POSITION;
};

float4 BasicPS(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(0.5f, 0, 1, 0.1f);
}