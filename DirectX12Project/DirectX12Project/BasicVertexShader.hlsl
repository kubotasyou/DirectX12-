//���_�V�F�[�_�[
//���_�f�[�^��1���_���ƂɌĂяo����Apos�ɓn�����

struct Output
{
	float4 pos : POSITION;
	float4 svpos : SV_POSITION;
};

//POSITION�͍��W�E�Z�}���e�B�N�X
float4 BasicVS(float4 pos : POSITION):  SV_POSITION
{
	return pos;
}