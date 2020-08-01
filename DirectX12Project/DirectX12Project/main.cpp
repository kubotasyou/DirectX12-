#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>

//�����N�̐ݒ�
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#ifdef _DEBUG
#include < iostream >
#endif 

using namespace std;

const int window_width = 1280;//��ʉ���
const int window_height = 720;//��ʏc��

//�󂯎M�ƂȂ�ϐ��̗p��
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;


//// @brief �R���\�[�� ��� �� �t�H�[�}�b�g �t�� ������ �� �\��
//// @param format �t�H�[�}�b�g�i% d �Ƃ�% f �Ƃ� �́j
//// @param �� �� ����
//// @remarks ���� �֐� �� �f�o�b�O �p �ł��B �f�o�b�O �� �ɂ��� ���� �� �܂� ��
//void DebugOutputFormatString( const char* format, ...) 
//{
//#ifdef _DEBUG 
//	va_list valist;
//	va_start( valist, format);
//	printf( format, valist);
//	va_end( valist);
//#endif
//} 
//#ifdef _DEBUG
//int main()
//{
//
//#else
//int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int) 
//{ 
//#endif 
//	DebugOutputFormatString(" Show window test.");
//getchar();
//return 0; 
//}


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

	
#pragma region �f�B�X�N���v�^�ƃX���b�v�`�F�[�����֘A�t��+�����_�[�^�[�Q�b�g�r���[�̍쐬
	  //�X���b�v�`�F�[���̃p�����[�^�[���擾
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	result = _swapchain->GetDesc(&scDesc);

	//�o�b�t�@�[�̐��̐ݒ�
	std::vector<ID3D12Resource*> _backBuffers(scDesc.BufferCount);

	//�o�b�t�@�[�̐�����
	for (int i = 0; i < scDesc.BufferCount; ++i)
	{
		//�\�������ɐݒ�
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));

		//�����_�[�^�[�Q�b�g�r���[�̐���
		D3D12_CPU_DESCRIPTOR_HANDLE handle =
			rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		//�����_�[�^�[�Q�b�g�r���[�̃T�C�Y���擾����
		//�|�C���^�[���P���炷�B(�\�ʂ��痠�ʂɍs���悤��)
		handle.ptr += i * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);
	}

	/*�o�b�N�o�b�t�@�[�ƃf�B�X�N���v�^�͂��ꂼ��������̂ŁA
	  for���Ńo�b�N�o�b�t�@�[�̐����񂵁A���ꂼ����֘A�t����悤�ɂ���B*/
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

#pragma region �X���b�v�`�F�[���̎��s

#pragma endregion

		//���t���[���N���A
		result = _cmdAllocator->Reset();

		//���݂̃o�b�N�o�b�t�@�[���w���C���f�b�N�X���擾(��)�\:0,��1 �݂����Ȋ���)
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();//bbIdx(BackBufferIndex)


	}
#pragma endregion

	//�����N���X�͎g��Ȃ��̂œo�^����������
	UnregisterClass(w.lpszClassName, w.hInstance);
}

