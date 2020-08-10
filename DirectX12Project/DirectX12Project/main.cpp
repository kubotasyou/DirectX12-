#include <Windows.h>
#include <d3d12.h>  //�t�@�C�������N(.lib)���K�v
#include <dxgi1_6.h>//�t�@�C�������N(.lib)���K�v
#include <vector>
#include <DirectXMath.h>
#include <d3dcompiler.h>//�V�F�[�_�[�̃R���p�C���p

//�����N�̐ݒ�
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef _DEBUG
#include < iostream >
#endif 

using namespace std;
using namespace DirectX;//DirectXMath�̖��O���

const int window_width = 1280;//��ʉ���
const int window_height = 720;//��ʏc��

//�󂯎M�ƂȂ�ϐ��̗p��
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

//���_��(���_�̏����͎��v���ɂ���)
XMFLOAT3 vertices[] = 
{
	{-0.5f, -0.7f, 0.0f},//����
    {+0.0f, +0.7f, 0.0f},//����
    {+0.5f, -0.7f, 0.0f},//�E��
};

//�V�F�[�_�[�I�u�W�F�N�g�����邽�߂̕ϐ���p��
ID3DBlob* _vsBlob = nullptr;
ID3DBlob* _psBlob = nullptr;
ID3DBlob* errorBlob = nullptr;

//�f�o�b�O���C���[(�u�����Ȃ������v�G���[�𕂂�����ɂ��Ă����)
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));

	//�f�o�b�O���C���[��L���ɂ���
	debugLayer->EnableDebugLayer();

	//�L����������A�C���^�[�t�F�[�X�����
	debugLayer->Release();
}

//�E�B���h�E�̐���
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		//OS�ɑ΂���[���̃A�v���͏I������]�Ɠ`����
		PostQuitMessage(0);
		return 0;
	}

	//����̏���
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main()
{

#pragma region ��{�I�ȏ���������

#pragma region �E�B���h�E�̐���
	//�E�B���h�E�N���X�̐���&�o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	//�R�[���o�b�N�֐��̎w��
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	//�A�v���P�[�V�����N���X���̐ݒ�
	w.lpszClassName = L"DirectXGame";
	//�n���h���̎擾
	w.hInstance = GetModuleHandle(nullptr);

	//�A�v���P�[�V�����N���X
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y�����߂�
	RECT wrc = { 0,0, window_width, window_height };

	//�֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(
		w.lpszClassName,      //�N���X���w��
		L"DX12�e�X�g",        //�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,  //�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,        //�\��X���W(OS�ɔC����)
		CW_USEDEFAULT,        //�\��Y���W(OS�ɔC����)
		wrc.right - wrc.left, //�E�B���h�E����
		wrc.bottom - wrc.top, //�E�B���h�E�c��
		nullptr,              //�e�E�B���h�E�n���h��
		nullptr,              //���j���[�n���h��
		w.hInstance,          //�Ăяo���A�v���P�[�V�����n���h��
		nullptr);             //�ǉ��p�����[�^�[

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);
#pragma endregion

#pragma region �f�o�b�O���C���[�̗L����&DXGI�̃G���[���b�Z�[�W�擾

#ifdef _DEBUG
	//�f�o�b�O���C���[���I���ɂ���
	EnableDebugLayer();
	////DXGI�̃G���[���b�Z�[�W���擾�ł���悤�ɂ���
	//CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,
	//	IID_PPV_ARGS(&_dxgiFactory));
#else
	CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));

#endif // _DEBUG

#pragma endregion

#pragma region Direct3D�f�o�C�X�̏�����
	//�g�p�ł���t���[�`���[���x�����P�����ׂ�
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL feautureLevel;

	for (auto lv : levels)
	{
		//levels�z��̒��g��S�Ē��ׂ�
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			feautureLevel = lv;
			break;//�����\�ȃo�[�W���������������烋�[�v���o��
		}
		//�ǂ���_���������ꍇ��_dev��nullptr������
	}
#pragma endregion

#pragma region �g�p����A�_�v�^�[�𖾎��I�ɗ񋓂���(��ŏ����Ă�����)
	//_dxgiFactory��DXGIFactory�I�u�W�F�N�g������B
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	//���Ŏ��s�����ꍇ��result�̌^��IDXGIFactory4*�ɁACreateDXGIFactory�ɂ���B

	//�A�_�v�^�[���
	std::vector<IDXGIAdapter*> adapters;

	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������!
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		//���p�\�ȃA�_�v�^�[���i�[����B
		adapters.push_back(tmpAdapter);
	}

	//�A�_�v�^�[�̎��ʏ��(DXGI_ADAPTER_DESC�\����)���擾
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//�A�_�v�^�[�̐����I�u�W�F�N�g�擾

		std::wstring strDesc = adesc.Description;

		//�T�������A�_�v�^�[�̖��O���m�F
		/*�����O��[NAVIDIA]���܂܂��A�_�v�^�[�������āA
		  tmpAdapter�Ɋi�[����B*/
		if (strDesc.find(L"NAVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

#pragma endregion

#pragma region �R�}���h�A���P�[�^�[�ƃR�}���h���X�g�̐���
	//�R�}���h�A���P�[�^�[�̐錾
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	//�R�}���h���X�g�̐錾
	ID3D12GraphicsCommandList* _cmdList = nullptr;

	//�R�}���h�A���P�[�^�[����(�f�o�C�X��AFactory�̐����̌�)
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	//�R�}���h���X�g����(�f�o�C�X��AFactory�̐����̌�)
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	/*commandList��GPU�ɖ��߂��郁�\�b�h�����A�C���^�[�t�F�[�X
	  commandList�͖��߂����ߍ��ނ����ŁA���s�͂ł��Ȃ��B*/
	  /*commandAllocator��commandList�̖��߂𗭂߂Ă�����(List)�̖���
		commandList�ɂ܂Ƃ߂�ꂽ���߂��A�ʂ̔��ɗ��߂Ă���*/
#pragma endregion

#pragma region �R�}���h�L���[�̐���
		//�R�}���h�L���[�̐錾
	ID3D12CommandQueue* _cmdQueue = nullptr;

	//�R�}���h�L���[�\���̂̐���
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	//�R�}���h�L���[�\���̂̐ݒ聫

	//�^�C���A�E�g����
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	//�A�_�v�^�[���P�����g��Ȃ��ꍇ��0������
	cmdQueueDesc.NodeMask = 0;

	//�v���C�I���e�B(�D�揇��)�͓��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	//�R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	//�R�}���h�L���[�\���̂̐ݒ聪

	//�R�}���h�L���[�̎��Ԑ���
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));


	/*�R�}���h�L���[��cmdList�ɗ��߂Ă������߂����s������́B
	  ���߂͓����ꂽ���Ԓʂ�Ɏ��s����Ă����B
	  ���߂����遨���s�����߂����� �ƌJ��Ԃ��A�����ɂ͂ł��Ȃ��B*/
#pragma endregion

#pragma region �X���b�v�`�F�[���̐���
	  //�X���b�v�`�F�[���\���̂̐錾
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	//�X���b�v�`�F�[���\���̂̐ݒ聫
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	//�o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	//��ʃt���b�v������͑f�����폜
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//�����͓��Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//�E�B���h�E�̃t���X�N���[����؂�ւ��\��
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//�X���b�v�`�F�[���\���̂̐ݒ聪

	//�X���b�v�`�F�[�����Ԑ���
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);

	/*�X���b�v�`�F�[���̓_�u���o�b�t�@�����O���邽�߂̂��́B
	  Dxlib���l�A�\�ʂŕ\�������ʂŕ`�揈�����\�ʂŕ\�� ���s���B
	  �\�ʁE���ʂ̃�������Ԃ��u�����_�[�^�[�Q�b�g�v�Ɛl�͌ĂԁB
	  �X���b�v�`�F�[�����s�����ƂŁA���炩�ɓ����Č�����B
	  ���X���b�v�`�F�[���͒ʏ�t���X�N���[���ōs�����̂����A
	  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH �ɂ����
	  �E�B���h�E���[�h�ň�����悤�ɐ؂�ւ��Ă���B*/

	  /*CreateSwapChainForHwnd�̊T�v
		����1.�ł̓R�}���h�L���[�I�u�W�F�N�g
		����2.�ł̓E�B���h�E�n���h��
		����3.�ł̓X���b�v�`�F�[���̐ݒ�p�\����
		����4.�ł�nullptr
		����5.�ł�nullptr
		����6.�ł̓X���b�v�`�F�[���I�u�W�F�N�g*/
#pragma endregion

#pragma region �f�B�X�N���v�^�q�[�v�̍쐬
		//�f�B�X�N���v�^�q�[�v�̐ݒ�\���̂̐錾
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	//�f�B�X�N���v�^�q�[�v�\���̂̐ݒ聫

	//�����_�[�^�[�Q�b�g�r���[���w��(RTV)
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	//GPU���P�����g�p���Ȃ��ꍇ��0
	heapDesc.NodeMask = 0;

	//�_�u���o�b�t�@�����O�Ȃ̂ŕ\���̓��
	heapDesc.NumDescriptors = 2;

	//���̓V�F�[�_�[���g�p���Ȃ�����NONE
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//�f�B�X�N���v�^�q�[�v�\���̂̐ݒ聪

	//�f�B�X�N���v�^�q�[�v��錾(RenderTargetView)
	ID3D12DescriptorHeap* rtvHeaps = nullptr;

	//�f�B�X�N���v�^�q�[�v�̐���
	result = _dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&rtvHeaps));

	/*�f�B�X�N���v�^�q�[�v�̓f�B�X�N���v�^(�������̉�̐��������Ă���f�[�^)��
	  �܂Ƃ߂ď������܂�Ă���[�������̈�]�B
	  �f�B�X�N���v�^�q�[�v�ɏ������܂ꂽ�����AGPU�ɋ�����B*/
#pragma endregion

#pragma region �t�F���X�̍쐬
	  //�t�F���X�̍쐬
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;

	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence));

	//�t�F���X�̎��s�̓��[�v�̒��ōs���܂��B

	/*�t�F���X�́A�R�}���h���X�g�̖��߂�
�@    ���ׂĊ����������ǂ����𒲂ׂ�
	  ���߂����ׂďI����Ă�������s�Ɉړ����A
	  ���s���I�������܂����߂Ɉړ�����B*/
#pragma endregion

#pragma region �f�B�X�N���v�^�ƃX���b�v�`�F�[�����֘A�t��+�����_�[�^�[�Q�b�g�r���[�̍쐬
	 //�X���b�v�`�F�[���̃p�����[�^�[���擾
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	result = _swapchain->GetDesc(&scDesc);

	//�o�b�t�@�[�̐��̐ݒ�
	std::vector<ID3D12Resource*> _backBuffers(scDesc.BufferCount);

	//�����_�[�^�[�Q�b�g�r���[�̐���
	D3D12_CPU_DESCRIPTOR_HANDLE handle =
		rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//�o�b�t�@�[�̐�����
	for (int i = 0; i < scDesc.BufferCount; ++i)
	{
		//�\�������ɐݒ�
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));


		//�����_�[�^�[�Q�b�g�r���[�̃T�C�Y���擾����
		//�|�C���^�[���P���炷�B(�\�ʂ��痠�ʂɍs���悤��)
		handle.ptr += i * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);
	}

	/*�o�b�N�o�b�t�@�[�ƃf�B�X�N���v�^�͂��ꂼ��������̂ŁA
	  for���Ńo�b�N�o�b�t�@�[�̐����񂵁A���ꂼ����֘A�t����悤�ɂ���B
	  �����_�[�^�[�Q�b�g�r���[�̓_�u���o�b�t�@�����O���s��������
	  �\�Ƃ����̓z*/
#pragma endregion

#pragma endregion




#pragma region ���_�o�b�t�@�[�̍쐬

	//���_�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty =
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference =
		D3D12_MEMORY_POOL_UNKNOWN;

	/*�q�[�v�̐ݒ�͂������Y�C�炵����*/

	//���\�[�X�ݒ�\����
	D3D12_RESOURCE_DESC resdesc = {};

	//�o�b�N�o�b�t�@�[�Ɏg���̂�Buffer���w��
	resdesc.Dimension =
		D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	//�摜�ł͂Ȃ��̂�unknown�ɂ���
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	//�A���`�G�C�W���O���s���p�����[�^�[�炵��
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	//�e�N�X�`�����C�A�E�g�ł͂Ȃ����߁����g���炵��
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	//���_�o�b�t�@�[�̐���
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	/*���_�o�b�t�@�[�́AGPU�ɒ��_�����󂯎�邽�߂̃��\�[�X(�̈�)*/

#pragma endregion

#pragma region ���_���̃}�b�v(�R�s�[)

	XMFLOAT3* vertMap = nullptr;

	//�o�b�t�@�[�ɒ��_���R�s�[����
	/*���_�o�b�t�@�[�̔ԍ�
	�@�}�b�v�������͈͂̎w��
	 �|�C���^�[�̃|�C���^�[*/
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	//���_�f�[�^���R�s�[����
	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	//�R�s�[���������閽��
	vertBuff->Unmap(0, nullptr);

	/*���_�o�b�t�@�[�����ł́AGPU�Ƀf�[�^��n�����Ƃ͂ł��Ȃ����߁A
	�@���_�����R�s�[����K�v������B
	 �R�s�[���邽�߂ɂ́A�|�C���^�[�̃A�h���X���K�v�ɂȂ�B
	 (���������w�肵�Ă����w�肷��)
	 (A���Q�Ƃ��Ă�B��C�Ŏ����Ă���)*/
#pragma endregion

#pragma region ���_�o�b�t�@�[�r���[�̍쐬

	D3D12_VERTEX_BUFFER_VIEW vbView = {};//VertexBufferView

	//�o�b�t�@�[�̉��z�A�h���X�擾(�|�C���^�[�̃|�C���^�[)
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//�S�o�C�g���𒲂ׂ�
	vbView.SizeInBytes = sizeof(vertices);
	//1���_������̃o�C�g���𒲂ׂ�
	vbView.StrideInBytes = sizeof(vertices[0]);

	/*���_�o�b�t�@�[�r���[�́A�f�[�^�̑傫����m�点�����*/
#pragma endregion

#pragma region ���_���C�A�E�g

	//���_���C�A�E�g�\����
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		"POSITION",                                //���W�̃f�[�^���w��
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,               //Float3�^���g�p����Ƃ����Ă���
		0,                                         //GPU�����_�f�[�^�����Ă�
		D3D12_APPEND_ALIGNED_ELEMENT,//�f�[�^�̏ꏊ
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,//1���_���Ƃ̃��C�A�E�g�������Ă���
		0                                          //��x�ɕ`�悷��C���X�^���V���O�̐�
	};

	/*���_���C�A�E�g�́AGPU�ɒ��_�f�[�^���ǂ�Ȃ��̂������������*/

#pragma endregion




#pragma region �V�F�[�_�[�̓ǂݍ��݂Ɛ���

	//���_�V�F�[�_�[�̓ǂݍ���
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",        //�V�F�[�_�[��
		nullptr,                          //define�͂��Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//�C���N���[�h�̓f�t�H���g
		"BasicVS",                        //�Ăяo���֐���BasicVS
		"vs_5_0",                         //�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION,     //�f�o�b�O�p�A�œK���͂��Ȃ�
		0,
		&_vsBlob,
		&errorBlob);                      //�G���[����errorBlob�Ƀ��b�Z�[�W������

	//�s�N�Z���V�F�[�_�[�̓ǂݍ���      
	result = D3DCompileFromFile(          //�V�F�[�_�[��
		L"BasicPixelShader.hlsl",		  //define�͂��Ȃ�
		nullptr,						  //�C���N���[�h�̓f�t�H���g
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//�Ăяo���֐���BasicVS
		"BasicPS",						  //�ΏۃV�F�[�_�[��vs_5_0
		"ps_5_0",
		D3DCOMPILE_DEBUG |				  //�f�o�b�O�p�A�œK���͂��Ȃ�
		D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob,						  //�G���[����errorBlob�Ƀ��b�Z�[�W������
		&errorBlob);

#pragma region �ǂݍ��ݎ��̃G���[���b�Z�[�W�̕\��

	if (FAILED(result))
	{
		//�t�@�C����������܂���̎�
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("�t�@�C����������܂���");

			return 0;
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";

			::OutputDebugStringA(errstr.c_str());
		}
	}

#pragma endregion

#pragma endregion



#pragma region �O���t�B�N�X�p�C�v���C���X�e�[�g�̍쐬

	//�O���t�B�N�X�p�C�v���C���X�e�[�g�\���̂̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	//���[�g�V�O�l�`���̐�����

	//���[�g�V�O�l�`���̐ݒ�\����
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	//��ł͂��邯�ǂ����_��񂾂��͑��݂��邱�Ƃ�`����
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//�s��`�̃I�u�W�F�N�g�m�ەϐ����쐬
	ID3DBlob* rootSigBlob = nullptr;

	//�o�C�i���R�[�h(�\�[�X�R�[�h���@�B��ɕϊ��������)�̍쐬
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,            //���[�g�V�O�l�`���̐ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0,//���[�g�V�O�l�`���̃o�[�W����
		&rootSigBlob,                  //�V�F�[�_�[�̎��Ɠ���
		&errorBlob);                   //�G���[�̎��͂����Ƀ��b�Z�[�W������


	ID3D12RootSignature* rootsignature = nullptr;
	//���[�g�V�O�l�`���I�u�W�F�N�g�̍쐬
	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootsignature)
	);

	rootSigBlob->Release();

	gpipeline.pRootSignature = rootsignature;

	//���[�g�V�O�l�`���̐�����

	//�V�F�[�_�[�̃Z�b�g��

	//���_�V�F�[�_�[�̃Z�b�g
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength  = _vsBlob->GetBufferSize();

	//�s�N�Z���V�F�[�_�[�̃Z�b�g
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength  = _psBlob->GetBufferSize();

	//�V�F�[�_�[�̃Z�b�g��
	//------------------------------------------------------------------------
	//�T���v���}�X�N�ƃ��X�^���C�U�[�X�e�[�g�̐ݒ聫

	//�f�t�H���g�̃T���v���}�X�N��\���萔
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//�A���`�G�C���A�X���g��Ȃ�����false�ɂ���
	gpipeline.RasterizerState.MultisampleEnable = false;

	//�J�����O(�K�v�̂Ȃ��|���S����`�悵�Ȃ��悤�ɂ���)�����Ȃ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//�}�`�̒��g��h��Ԃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	//�[�x�����̃N���b�s���O��L���ɂ���
	gpipeline.RasterizerState.DepthClipEnable = true;

	//�T���v���}�X�N�ƃ��X�^���C�U�[�X�e�[�g�̐ݒ聪
	//-----------------------------------------------------------------------
	//�u�����h�X�e�[�g�̐ݒ聫

	//���e�X�g(�����̐ݒ�𔽉f����)���s�����ǂ���
	gpipeline.BlendState.AlphaToCoverageEnable = false;

	//�����_�[�^�[�Q�b�g�����ꂼ��ʂŐݒ肷�邩
	gpipeline.BlendState.IndependentBlendEnable = false;


	//�����_�[�^�[�Q�b�g�̐ݒ�\����
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	//���l�Ȃǂ̃u�����h�����邩
	renderTargetBlendDesc.BlendEnable = false;

	//�_�����Z(1��0��1�ɂ���Ƃ�)���s�����ǂ���
	renderTargetBlendDesc.LogicOpEnable = false;
	/*BlendEnable��LogicOpEnable�͓�����true�ɂ��邱�Ƃ͂ł��Ȃ�*/

	//�S�Ă̐F�Ńu�����h���\�ɂ���
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//�u�����h�̔��f
	gpipeline.BlendState.RenderTarget[0] =
		renderTargetBlendDesc;

	/*�u�����h�X�e�[�g�͏ꍇ�ɂ���āA
	�@�g�p���郌���_�[�^�[�Q�b�g�̕������ʂɐݒ肪�K�v�ɂȂ�B*/

	//�u�����h�X�e�[�g�̐ݒ聪
	//---------------------------------------------------------------------------
	//���̓��C�A�E�g�̐ݒ聫

	//���C�A�E�g�̐擪�A�h���X���擾
	gpipeline.InputLayout.pInputElementDescs = inputLayout;

	//���C�A�E�g�z��̗v�f�����擾
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//���_�̃J�b�g���s��Ȃ�(6���_�Ŏl�p�`�����^�C�v)
	gpipeline.IBStripCutValue =
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	//�O�p�`���\������悤�ɂ���
	gpipeline.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//���̓��C�A�E�g�̐ݒ聪
	//---------------------------------------------------------------------------------
	//�����_�[�^�[�Q�b�g�̐ݒ聫

	//�����_�[�^�[�Q�b�g�͍��͈�����Ȃ���
	gpipeline.NumRenderTargets = 1;

	//0�`1�ɐ��K�����ꂽRGBA
	gpipeline.RTVFormats[0] =
		DXGI_FORMAT_R8G8B8A8_UNORM;

	//�����_�[�^�[�Q�b�g�̐ݒ聪
	//-------------------------------------------------------------------------
	//�A���`�G�C���A�V���O�̂��߂̃T���v�����ݒ聫

	//�T���v�����O��1�s�N�Z���ɂ��P��
	gpipeline.SampleDesc.Count = 1;

	//�N�I���e�B�͍Œ�
	gpipeline.SampleDesc.Quality = 0;

	//�A���`�G�C���A�V���O�̂��߂̃T���v�����ݒ聪

	//�O���t�B�b�N�X�p�C�v���C���X�e�[�g�I�u�W�F�N�g�̐�����

	//�I�u�W�F�N�g�̐錾
	ID3D12PipelineState* _pipelinestate = nullptr;

	//�I�u�W�F�N�g�̍쐬
	result = _dev->CreateGraphicsPipelineState(
		&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	//�O���t�B�b�N�X�p�C�v���C���X�e�[�g�I�u�W�F�N�g�̐�����

	/*�O���t�B�b�N�p�C�v���C���X�e�[�g�Ƃ�
	  �O���t�B�b�N�p�C�v���C���̐ݒ���`����\���́B
	  �O���t�B�b�N�p�C�v���C���ɂ������ݒ���ЂƂ܂Ƃ߂ɂ��āA
	  �����悭����̂��O���t�B�N�X�p�C�v���C���X�e�[�g*/

#pragma endregion

#pragma region �r���[�|�[�g�ƃV�U�[��`�̐ݒ�

	//�r���[�|�[�g�ݒ聫

	//�r���[�|�[�g�ݒ�\����
	D3D12_VIEWPORT viewport = {};

	//�o�͐�̉���(�s�N�Z��)
	viewport.Width = window_width;

	//�o�͐�̏c��(�s�N�Z��)
	viewport.Height = window_height;

	viewport.TopLeftX = 0;   //�o�͐�̍�����WX
	viewport.TopLeftY = 0;   //�o�͐�̍�����WY
	viewport.MaxDepth = 1.0f;//�[�x�ő�l
	viewport.MinDepth = 0.0f;//�[�x�ŏ��l

	/*�r���[�|�[�g�́A��ʂɑ΂���`����ǂ����邩�Ƃ������́B
	�@�o�͂����摜���r���[�|�[�g�Ɏ��܂�悤�ɕ\�������*/

	//�r���[�|�[�g�ݒ聪

	//�V�U�[��`�ݒ聫

	//�V�U�[��`�ݒ�\����
	D3D12_RECT scissorrect = {};

	scissorrect.top = 0; //�؂蔲������W
	scissorrect.left = 0;//�؂蔲�������W
	scissorrect.bottom = scissorrect.top + window_height;//�؂蔲�������W
	scissorrect.right = scissorrect.left + window_width; //�؂蔲���E���W

	/*�V�U�[��`�́A�r���[�|�[�g�ɏo�͂��ꂽ�摜�́A
	�@�ǂ�����ǂ��܂ł�\�����邩��ݒ肷����́B
	 �ꕔ��������\���������ꍇ�̓r���[�|�[�g�̒l��菬��������B*/

	//�V�U�[��`�ݒ聪

#pragma endregion


#pragma region ���b�Z�[�W���[�v
	  //���b�Z�[�W���[�v
	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}

#pragma region ��ʏ�����&���s

		//���̃t���[���ŕ\�������o�b�t�@�[�̃C���f�b�N�X(��)�\:0,��1 �݂����Ȋ���)
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();//bbIdx(BackBufferIndex)
		
		//----------------------------------------------------------------------------
		//���\�[�X�o���A�̐ݒ聫

		D3D12_RESOURCE_BARRIER BarrierDesc = {};

		//�o���A�̎��(�J�ڂ�transition)
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		//�w��Ȃ�
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//�o�b�N�o�b�t�@�[���\�[�X
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = 0;

		//���O��present(����)���
		BarrierDesc.Transition.StateBefore =
			D3D12_RESOURCE_STATE_PRESENT;
		//���̐悩�烌���_�[�^�[�Q�b�g���
		BarrierDesc.Transition.StateAfter =
			D3D12_RESOURCE_STATE_RENDER_TARGET;

		//�o���A�̎��s
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		//���\�[�X�o���A�̐ݒ聪
		//----------------------------------------------------------------------------
		//�����_�[�^�[�Q�b�g�̐ݒ聫

		//�����_�[�^�[�Q�b�g�r���[�̕`��
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		/*�����_�[�^�[�Q�b�g�r���[�̐�
		�@rtvHeaps�̃A�h���X
		  �}���`�����_�[�^�[�Q�b�g
		  �[�x*/
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//�����_�[�^�[�Q�b�g�̐ݒ聪
		//--------------------------------------------------------------------------------
		//��ʂ̃N���A��

		//��ʂ����̐F���w�肵�ăN���A
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };

		//�F�𔽉f������
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		//�p�C�v���C���X�e�[�g�̓ǂݍ���
		_cmdList->SetPipelineState(_pipelinestate);

		//�r���[�|�[�g�̓ǂݍ���
		_cmdList->RSSetViewports(1, &viewport);

		//�V�U�[��`�̓ǂݍ���
		_cmdList->RSSetScissorRects(1, &scissorrect);

		//���[�g�V�O�l�`���̓ǂݍ���
		_cmdList->SetGraphicsRootSignature(rootsignature);

		//���_���ǂ��g�ݍ��킹�邩���O�p�`
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//���_�o�b�t�@�[���Z�b�g����
		/*�X���b�g�ԍ�
		�@���_�o�b�t�@�[�r���[�̐�
		  ���_�o�b�t�@�[�r���[�̔z��*/
		_cmdList->IASetVertexBuffers(0, 1, &vbView);

		//�`��R�}���h�̌Ăяo��
		/*���_��
		  �C���X�^���X(�\������|���S����)��
		  ���_�f�[�^�̃I�t�Z�b�g
		  �C���X�^���X�̃I�t�Z�b�g*/
		_cmdList->DrawInstanced(3, 1, 0, 0);


		//��ʂ̃N���A��
		//---------------------------------------------------------------------
		//���݂̏�Ԃ��A�����_�[�^�[�Q�b�g����Present�Ɉڍs���遫

		//�O�ゾ�������ւ���
		BarrierDesc.Transition.StateBefore =
			D3D12_RESOURCE_STATE_RENDER_TARGET;//���ꂪ���݂̏��
		BarrierDesc.Transition.StateAfter =
			D3D12_RESOURCE_STATE_PRESENT;//�����������ꂩ��Ȃ���

		_cmdList->ResourceBarrier(1, &BarrierDesc);//���s

		//���݂̏�Ԃ��A�����_�[�^�[�Q�b�g����Present�Ɉڍs���遪
		//-------------------------------------------------------------------------
		//���ߍ��񂾖��߂̎��s��

		//���߂̃N���[�Y
		/*���߂��I�����āA���s�t�F�[�Y�Ɉڍs����*/
		_cmdList->Close();

		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//�t�F���X�̌Ăяo��
		_cmdQueue->Signal(_fence, ++_fenceVal);//�҂��܂�
		if (_fence->GetCompletedValue() != _fenceVal)
		{
			//�C�x���g�n���h���̎擾
			auto event = CreateEvent(nullptr, false, false, nullptr);

			/*���̒l�ɂȂ�����C�x���g�𔭐�������
			�@����������C�x���g�����߂�*/
			_fence->SetEventOnCompletion(_fenceVal, event);

			//�C�x���g����������܂ő҂�������(INFINITE)
			WaitForSingleObject(event, INFINITE);

			//�C�x���g�n���h�������
			CloseHandle(event);
		}

		/*���s���I�������R�}���h���X�g�͕s�v�ɂȂ邽�߁A
		  ���g���N���A����*/
		  //�L���[���N���A
		_cmdAllocator->Reset();
		//���s���I�����A���߃t�F�[�Y�Ɉڍs����
		_cmdList->Reset(_cmdAllocator, nullptr);

		//���ߍ��񂾖��߂̎��s��
		//-------------------------------------------------------------------
		//��ʂ̃X���b�v
		/*���߂̎��s������������A�t���b�v���s��*/
		_swapchain->Present(1, 0);

#pragma endregion
	}
#pragma endregion

	//�����N���X�͎g��Ȃ��̂œo�^����������
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}

