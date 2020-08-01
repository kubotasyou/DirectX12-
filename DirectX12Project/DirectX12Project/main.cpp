#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>

//リンクの設定
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#ifdef _DEBUG
#include < iostream >
#endif 

using namespace std;

const int window_width = 1280;//画面横幅
const int window_height = 720;//画面縦幅

//受け皿となる変数の用意
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;


//// @brief コンソール 画面 に フォーマット 付き 文字列 を 表示
//// @param format フォーマット（% d とか% f とか の）
//// @param 可変 長 引数
//// @remarks この 関数 は デバッグ 用 です。 デバッグ 時 にしか 動作 し ませ ん
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


//ウィンドウの生成
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		//OSに対して[このアプリは終了した]と伝える
		PostQuitMessage(0);
		return 0;
	}

	//既定の処理
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main()
{
#pragma region 基本的な初期化処理

#pragma region ウィンドウの生成
	//ウィンドウクラスの生成&登録
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	//コールバック関数の指定
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	//アプリケーションクラス名の設定
	w.lpszClassName = L"DirectXGame";
	//ハンドルの取得
	w.hInstance = GetModuleHandle(nullptr);

	//アプリケーションクラス
	RegisterClassEx(&w);
	//ウィンドウサイズを決める
	RECT wrc = { 0,0, window_width, window_height };

	//関数を使ってウィンドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(
		w.lpszClassName,      //クラス名指定
		L"DX12テスト",        //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,  //タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,        //表示X座標(OSに任せる)
		CW_USEDEFAULT,        //表示Y座標(OSに任せる)
		wrc.right - wrc.left, //ウィンドウ横幅
		wrc.bottom - wrc.top, //ウィンドウ縦幅
		nullptr,              //親ウィンドウハンドル
		nullptr,              //メニューハンドル
		w.hInstance,          //呼び出しアプリケーションハンドル
		nullptr);             //追加パラメーター

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
#pragma endregion

#pragma region Direct3Dデバイスの初期化
	//使用できるフューチャーレベルを１つずつ調べる
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL feautureLevel;

	for (auto lv : levels)
	{
		//levels配列の中身を全て調べる
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			feautureLevel = lv;
			break;//生成可能なバージョンが見つかったらループを出る
		}
		//どれもダメだった場合は_devにnullptrが入る
	}
#pragma endregion

#pragma region 使用するアダプターを明示的に列挙する(後で消してもいい)
	//_dxgiFactoryにDXGIFactoryオブジェクトが入る。
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	//↑で失敗した場合はresultの型をIDXGIFactory4*に、CreateDXGIFactoryにする。

	//アダプターを列挙
	std::vector<IDXGIAdapter*> adapters;

	//ここに特定の名前を持つアダプターオブジェクトが入る!
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		//利用可能なアダプターを格納する。
		adapters.push_back(tmpAdapter);
	}

	//アダプターの識別情報(DXGI_ADAPTER_DESC構造体)を取得
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//アダプターの説明オブジェクト取得

		std::wstring strDesc = adesc.Description;

		//探したいアダプターの名前を確認
		/*↓名前に[NAVIDIA]が含まれるアダプターを見つけて、
		  tmpAdapterに格納する。*/
		if (strDesc.find(L"NAVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

#pragma endregion

#pragma endregion

#pragma region コマンドアロケーターとコマンドリストの生成
	//コマンドアロケーターの宣言
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	//コマンドリストの宣言
	ID3D12GraphicsCommandList* _cmdList = nullptr;

	//コマンドアロケーター生成(デバイスや、Factoryの生成の後)
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	//コマンドリスト生成(デバイスや、Factoryの生成の後)
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	/*commandListはGPUに命令するメソッドを持つ、インターフェース
	  commandListは命令をため込むだけで、実行はできない。*/
	/*commandAllocatorはcommandListの命令を溜めておく箱(List)の役割
	  commandListにまとめられた命令を、別の箱に溜めておく*/
#pragma endregion

#pragma region コマンドキューの生成
	  //コマンドキューの宣言
	ID3D12CommandQueue* _cmdQueue = nullptr;

	//コマンドキュー構造体の生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	//コマンドキュー構造体の設定↓

	//タイムアウト無し
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	//アダプターを１つしか使わない場合は0を入れる
	cmdQueueDesc.NodeMask = 0;

	//プライオリティ(優先順位)は特に指定なし
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	//コマンドリストと合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	//コマンドキュー構造体の設定↑

	//コマンドキューの実態生成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));


	/*コマンドキューはcmdListに溜めていた命令を実行するもの。
	  命令は入れられた順番通りに実行されていく。
	  命令を入れる→実行→命令を入れる と繰り返し、同時にはできない。*/
#pragma endregion

#pragma region スワップチェーンの生成
	  //スワップチェーン構造体の宣言
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	//スワップチェーン構造体の設定↓
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	//バックバッファーは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	//画面フリップした後は素早く削除
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//透明は特に指定なし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//ウィンドウ⇔フルスクリーンを切り替え可能に
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//スワップチェーン構造体の設定↑

	//スワップチェーン実態生成
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);

	/*スワップチェーンはダブルバッファリングするためのもの。
	  Dxlib同様、表面で表示→裏面で描画処理→表面で表示 を行う。
	  表面・裏面のメモリ空間を「レンダーターゲット」と人は呼ぶ。
	  スワップチェーンを行うことで、滑らかに動いて見える。
	  ※スワップチェーンは通常フルスクリーンで行うものだが、
	  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH によって
	  ウィンドウモードで扱えるように切り替えている。*/

	/*CreateSwapChainForHwndの概要
	  引数1.ではコマンドキューオブジェクト
	  引数2.ではウィンドウハンドル
	  引数3.ではスワップチェーンの設定用構造体
	  引数4.ではnullptr
	  引数5.ではnullptr
	  引数6.ではスワップチェーンオブジェクト*/
#pragma endregion

#pragma region ディスクリプタヒープの作成
	  //ディスクリプタヒープの設定構造体の宣言
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	//ディスクリプタヒープ構造体の設定↓

	//レンダーターゲットビューを指定(RTV)
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	//GPUを１つしか使用しない場合は0
	heapDesc.NodeMask = 0;

	//ダブルバッファリングなので表裏の二つ
	heapDesc.NumDescriptors = 2;

	//今はシェーダーを使用しないためNONE
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//ディスクリプタヒープ構造体の設定↑

	//ディスクリプタヒープを宣言(RenderTargetView)
	ID3D12DescriptorHeap* rtvHeaps = nullptr;

	//ディスクリプタヒープの生成
	result = _dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&rtvHeaps));

	/*ディスクリプタヒープはディスクリプタ(メモリの塊の説明をしているデータ)を
	  まとめて書き込まれている[メモリ領域]。
	  ディスクリプタヒープに書き込まれた情報を、GPUに教える。*/
#pragma endregion

	
#pragma region ディスクリプタとスワップチェーンを関連付け+レンダーターゲットビューの作成
	  //スワップチェーンのパラメーターを取得
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	result = _swapchain->GetDesc(&scDesc);

	//バッファーの数の設定
	std::vector<ID3D12Resource*> _backBuffers(scDesc.BufferCount);

	//バッファーの数分回す
	for (int i = 0; i < scDesc.BufferCount; ++i)
	{
		//表裏両方に設定
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));

		//レンダーターゲットビューの生成
		D3D12_CPU_DESCRIPTOR_HANDLE handle =
			rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		//レンダーターゲットビューのサイズを取得して
		//ポインターを１つずらす。(表面から裏面に行くように)
		handle.ptr += i * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);
	}

	/*バックバッファーとディスクリプタはそれぞれ二つずつあるので、
	  for文でバックバッファーの数分回し、それぞれを関連付けるようにする。*/
#pragma endregion


#pragma region メッセージループ
	//メッセージループ
	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

#pragma region スワップチェーンの実行

#pragma endregion

		//毎フレームクリア
		result = _cmdAllocator->Reset();

		//現在のバックバッファーを指すインデックスを取得(例)表:0,裏1 みたいな感じ)
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();//bbIdx(BackBufferIndex)


	}
#pragma endregion

	//もうクラスは使わないので登録を解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
}

