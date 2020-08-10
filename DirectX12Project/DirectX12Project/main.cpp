#include <Windows.h>
#include <d3d12.h>  //ファイルリンク(.lib)が必要
#include <dxgi1_6.h>//ファイルリンク(.lib)が必要
#include <vector>
#include <DirectXMath.h>
#include <d3dcompiler.h>//シェーダーのコンパイル用

//リンクの設定
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef _DEBUG
#include < iostream >
#endif 

using namespace std;
using namespace DirectX;//DirectXMathの名前空間

const int window_width = 1280;//画面横幅
const int window_height = 720;//画面縦幅

//受け皿となる変数の用意
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

//頂点数(頂点の順序は時計回りにする)
XMFLOAT3 vertices[] = 
{
	{-0.5f, -0.7f, 0.0f},//左下
    {+0.0f, +0.7f, 0.0f},//左上
    {+0.5f, -0.7f, 0.0f},//右下
};

//シェーダーオブジェクトを入れるための変数を用意
ID3DBlob* _vsBlob = nullptr;
ID3DBlob* _psBlob = nullptr;
ID3DBlob* errorBlob = nullptr;

//デバッグレイヤー(「見えなかった」エラーを浮き彫りにしてくれる)
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));

	//デバッグレイヤーを有効にする
	debugLayer->EnableDebugLayer();

	//有効化したら、インターフェースを解放
	debugLayer->Release();
}

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

#pragma region デバッグレイヤーの有効化&DXGIのエラーメッセージ取得

#ifdef _DEBUG
	//デバッグレイヤーをオンにする
	EnableDebugLayer();
	////DXGIのエラーメッセージを取得できるようにする
	//CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,
	//	IID_PPV_ARGS(&_dxgiFactory));
#else
	CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));

#endif // _DEBUG

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

#pragma region フェンスの作成
	  //フェンスの作成
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;

	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence));

	//フェンスの実行はループの中で行います。

	/*フェンスは、コマンドリストの命令が
　    すべて完了したかどうかを調べる
	  命令がすべて終わっていたら実行に移動し、
	  実行が終わったらまた命令に移動する。*/
#pragma endregion

#pragma region ディスクリプタとスワップチェーンを関連付け+レンダーターゲットビューの作成
	 //スワップチェーンのパラメーターを取得
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	result = _swapchain->GetDesc(&scDesc);

	//バッファーの数の設定
	std::vector<ID3D12Resource*> _backBuffers(scDesc.BufferCount);

	//レンダーターゲットビューの生成
	D3D12_CPU_DESCRIPTOR_HANDLE handle =
		rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//バッファーの数分回す
	for (int i = 0; i < scDesc.BufferCount; ++i)
	{
		//表裏両方に設定
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i]));


		//レンダーターゲットビューのサイズを取得して
		//ポインターを１つずらす。(表面から裏面に行くように)
		handle.ptr += i * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);
	}

	/*バックバッファーとディスクリプタはそれぞれ二つずつあるので、
	  for文でバックバッファーの数分回し、それぞれを関連付けるようにする。
	  レンダーターゲットビューはダブルバッファリングを行った時の
	  表とか裏の奴*/
#pragma endregion

#pragma endregion




#pragma region 頂点バッファーの作成

	//頂点ヒープ設定
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty =
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference =
		D3D12_MEMORY_POOL_UNKNOWN;

	/*ヒープの設定はくそムズイらしいよ*/

	//リソース設定構造体
	D3D12_RESOURCE_DESC resdesc = {};

	//バックバッファーに使うのでBufferを指定
	resdesc.Dimension =
		D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	//画像ではないのでunknownにする
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	//アンチエイジングを行うパラメーターらしい
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	//テクスチャレイアウトではないため↓を使うらしい
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	//頂点バッファーの生成
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	/*頂点バッファーは、GPUに頂点情報を受け取るためのリソース(領域)*/

#pragma endregion

#pragma region 頂点情報のマップ(コピー)

	XMFLOAT3* vertMap = nullptr;

	//バッファーに頂点情報コピーする
	/*頂点バッファーの番号
	　マップしたい範囲の指定
	 ポインターのポインター*/
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	//頂点データをコピーする
	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	//コピーを解除する命令
	vertBuff->Unmap(0, nullptr);

	/*頂点バッファーだけでは、GPUにデータを渡すことはできないため、
	　頂点情報をコピーする必要がある。
	 コピーするためには、ポインターのアドレスが必要になる。
	 (メモリを指定してるやつを指定する)
	 (Aを参照してるBをCで持ってくる)*/
#pragma endregion

#pragma region 頂点バッファービューの作成

	D3D12_VERTEX_BUFFER_VIEW vbView = {};//VertexBufferView

	//バッファーの仮想アドレス取得(ポインターのポインター)
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//全バイト数を調べる
	vbView.SizeInBytes = sizeof(vertices);
	//1頂点当たりのバイト数を調べる
	vbView.StrideInBytes = sizeof(vertices[0]);

	/*頂点バッファービューは、データの大きさを知らせるもの*/
#pragma endregion

#pragma region 頂点レイアウト

	//頂点レイアウト構造体
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		"POSITION",                                //座標のデータを指定
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,               //Float3型を使用するといっている
		0,                                         //GPUが頂点データを見てる
		D3D12_APPEND_ALIGNED_ELEMENT,//データの場所
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,//1頂点ごとのレイアウトが入っている
		0                                          //一度に描画するインスタンシングの数
	};

	/*頂点レイアウトは、GPUに頂点データがどんなものかを教えるもの*/

#pragma endregion




#pragma region シェーダーの読み込みと生成

	//頂点シェーダーの読み込み
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",        //シェーダー名
		nullptr,                          //defineはしない
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//インクルードはデフォルト
		"BasicVS",                        //呼び出す関数はBasicVS
		"vs_5_0",                         //対象シェーダーはvs_5_0
		D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION,     //デバッグ用、最適化はしない
		0,
		&_vsBlob,
		&errorBlob);                      //エラー時はerrorBlobにメッセージが入る

	//ピクセルシェーダーの読み込み      
	result = D3DCompileFromFile(          //シェーダー名
		L"BasicPixelShader.hlsl",		  //defineはしない
		nullptr,						  //インクルードはデフォルト
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//呼び出す関数はBasicVS
		"BasicPS",						  //対象シェーダーはvs_5_0
		"ps_5_0",
		D3DCOMPILE_DEBUG |				  //デバッグ用、最適化はしない
		D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob,						  //エラー時はerrorBlobにメッセージが入る
		&errorBlob);

#pragma region 読み込み時のエラーメッセージの表示

	if (FAILED(result))
	{
		//ファイルが見つかりませんの時
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("ファイルが見つかりません");

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



#pragma region グラフィクスパイプラインステートの作成

	//グラフィクスパイプラインステート構造体の作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	//ルートシグネチャの生成↓

	//ルートシグネチャの設定構造体
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	//空ではあるけども頂点情報だけは存在することを伝える
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//不定形のオブジェクト確保変数を作成
	ID3DBlob* rootSigBlob = nullptr;

	//バイナリコード(ソースコードを機械語に変換するもの)の作成
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,            //ルートシグネチャの設定
		D3D_ROOT_SIGNATURE_VERSION_1_0,//ルートシグネチャのバージョン
		&rootSigBlob,                  //シェーダーの時と同じ
		&errorBlob);                   //エラーの時はここにメッセージが入る


	ID3D12RootSignature* rootsignature = nullptr;
	//ルートシグネチャオブジェクトの作成
	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootsignature)
	);

	rootSigBlob->Release();

	gpipeline.pRootSignature = rootsignature;

	//ルートシグネチャの生成↑

	//シェーダーのセット↓

	//頂点シェーダーのセット
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength  = _vsBlob->GetBufferSize();

	//ピクセルシェーダーのセット
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength  = _psBlob->GetBufferSize();

	//シェーダーのセット↑
	//------------------------------------------------------------------------
	//サンプルマスクとラスタライザーステートの設定↓

	//デフォルトのサンプルマスクを表す定数
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//アンチエイリアスを使わないためfalseにする
	gpipeline.RasterizerState.MultisampleEnable = false;

	//カリング(必要のないポリゴンを描画しないようにする)をしない
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//図形の中身を塗りつぶす
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	//深度方向のクリッピングを有効にする
	gpipeline.RasterizerState.DepthClipEnable = true;

	//サンプルマスクとラスタライザーステートの設定↑
	//-----------------------------------------------------------------------
	//ブレンドステートの設定↓

	//αテスト(透明の設定を反映する)を行うかどうか
	gpipeline.BlendState.AlphaToCoverageEnable = false;

	//レンダーターゲットをそれぞれ個別で設定するか
	gpipeline.BlendState.IndependentBlendEnable = false;


	//レンダーターゲットの設定構造体
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	//α値などのブレンドをするか
	renderTargetBlendDesc.BlendEnable = false;

	//論理演算(1と0で1にするとか)を行うかどうか
	renderTargetBlendDesc.LogicOpEnable = false;
	/*BlendEnableとLogicOpEnableは同時にtrueにすることはできない*/

	//全ての色でブレンドを可能にする
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//ブレンドの反映
	gpipeline.BlendState.RenderTarget[0] =
		renderTargetBlendDesc;

	/*ブレンドステートは場合によって、
	　使用するレンダーターゲットの分だけ個別に設定が必要になる。*/

	//ブレンドステートの設定↑
	//---------------------------------------------------------------------------
	//入力レイアウトの設定↓

	//レイアウトの先頭アドレスを取得
	gpipeline.InputLayout.pInputElementDescs = inputLayout;

	//レイアウト配列の要素数を取得
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//頂点のカットを行わない(6頂点で四角形を作るタイプ)
	gpipeline.IBStripCutValue =
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	//三角形を構成するようにする
	gpipeline.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//入力レイアウトの設定↑
	//---------------------------------------------------------------------------------
	//レンダーターゲットの設定↓

	//レンダーターゲットは今は一つしかないよ
	gpipeline.NumRenderTargets = 1;

	//0～1に正規化されたRGBA
	gpipeline.RTVFormats[0] =
		DXGI_FORMAT_R8G8B8A8_UNORM;

	//レンダーターゲットの設定↑
	//-------------------------------------------------------------------------
	//アンチエイリアシングのためのサンプル数設定↓

	//サンプリングは1ピクセルにつき１回
	gpipeline.SampleDesc.Count = 1;

	//クオリティは最低
	gpipeline.SampleDesc.Quality = 0;

	//アンチエイリアシングのためのサンプル数設定↑

	//グラフィックスパイプラインステートオブジェクトの生成↓

	//オブジェクトの宣言
	ID3D12PipelineState* _pipelinestate = nullptr;

	//オブジェクトの作成
	result = _dev->CreateGraphicsPipelineState(
		&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	//グラフィックスパイプラインステートオブジェクトの生成↑

	/*グラフィックパイプラインステートとは
	  グラフィックパイプラインの設定を定義する構造体。
	  グラフィックパイプラインにかかわる設定をひとまとめにして、
	  効率よくするのがグラフィクスパイプラインステート*/

#pragma endregion

#pragma region ビューポートとシザー矩形の設定

	//ビューポート設定↓

	//ビューポート設定構造体
	D3D12_VIEWPORT viewport = {};

	//出力先の横幅(ピクセル)
	viewport.Width = window_width;

	//出力先の縦幅(ピクセル)
	viewport.Height = window_height;

	viewport.TopLeftX = 0;   //出力先の左上座標X
	viewport.TopLeftY = 0;   //出力先の左上座標Y
	viewport.MaxDepth = 1.0f;//深度最大値
	viewport.MinDepth = 0.0f;//深度最小値

	/*ビューポートは、画面に対する描画をどうするかというもの。
	　出力した画像がビューポートに収まるように表示される*/

	//ビューポート設定↑

	//シザー矩形設定↓

	//シザー矩形設定構造体
	D3D12_RECT scissorrect = {};

	scissorrect.top = 0; //切り抜き上座標
	scissorrect.left = 0;//切り抜き左座標
	scissorrect.bottom = scissorrect.top + window_height;//切り抜き下座標
	scissorrect.right = scissorrect.left + window_width; //切り抜き右座標

	/*シザー矩形は、ビューポートに出力された画像の、
	　どこからどこまでを表示するかを設定するもの。
	 一部分だけを表示したい場合はビューポートの値より小さくする。*/

	//シザー矩形設定↑

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

#pragma region 画面初期化&実行

		//次のフレームで表示されるバッファーのインデックス(例)表:0,裏1 みたいな感じ)
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();//bbIdx(BackBufferIndex)
		
		//----------------------------------------------------------------------------
		//リソースバリアの設定↓

		D3D12_RESOURCE_BARRIER BarrierDesc = {};

		//バリアの種別(遷移はtransition)
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		//指定なし
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//バックバッファーリソース
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = 0;

		//直前はpresent(存在)状態
		BarrierDesc.Transition.StateBefore =
			D3D12_RESOURCE_STATE_PRESENT;
		//この先からレンダーターゲット状態
		BarrierDesc.Transition.StateAfter =
			D3D12_RESOURCE_STATE_RENDER_TARGET;

		//バリアの実行
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		//リソースバリアの設定↑
		//----------------------------------------------------------------------------
		//レンダーターゲットの設定↓

		//レンダーターゲットビューの描画
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		/*レンダーターゲットビューの数
		　rtvHeapsのアドレス
		  マルチレンダーターゲット
		  深度*/
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//レンダーターゲットの設定↑
		//--------------------------------------------------------------------------------
		//画面のクリア↓

		//画面を特定の色を指定してクリア
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };

		//色を反映させる
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		//パイプラインステートの読み込み
		_cmdList->SetPipelineState(_pipelinestate);

		//ビューポートの読み込み
		_cmdList->RSSetViewports(1, &viewport);

		//シザー矩形の読み込み
		_cmdList->RSSetScissorRects(1, &scissorrect);

		//ルートシグネチャの読み込み
		_cmdList->SetGraphicsRootSignature(rootsignature);

		//頂点をどう組み合わせるか→三角形
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//頂点バッファーをセットする
		/*スロット番号
		　頂点バッファービューの数
		  頂点バッファービューの配列*/
		_cmdList->IASetVertexBuffers(0, 1, &vbView);

		//描画コマンドの呼び出し
		/*頂点数
		  インスタンス(表示するポリゴンの)数
		  頂点データのオフセット
		  インスタンスのオフセット*/
		_cmdList->DrawInstanced(3, 1, 0, 0);


		//画面のクリア↑
		//---------------------------------------------------------------------
		//現在の状態を、レンダーターゲットからPresentに移行する↓

		//前後だけを入れ替える
		BarrierDesc.Transition.StateBefore =
			D3D12_RESOURCE_STATE_RENDER_TARGET;//これが現在の状態
		BarrierDesc.Transition.StateAfter =
			D3D12_RESOURCE_STATE_PRESENT;//こっちがこれからなる状態

		_cmdList->ResourceBarrier(1, &BarrierDesc);//実行

		//現在の状態を、レンダーターゲットからPresentに移行する↑
		//-------------------------------------------------------------------------
		//ため込んだ命令の実行↓

		//命令のクローズ
		/*命令を終了して、実行フェーズに移行する*/
		_cmdList->Close();

		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//フェンスの呼び出し
		_cmdQueue->Signal(_fence, ++_fenceVal);//待ちます
		if (_fence->GetCompletedValue() != _fenceVal)
		{
			//イベントハンドルの取得
			auto event = CreateEvent(nullptr, false, false, nullptr);

			/*この値になったらイベントを発生させる
			　発生させるイベントをきめる*/
			_fence->SetEventOnCompletion(_fenceVal, event);

			//イベントが発生するまで待ち続ける(INFINITE)
			WaitForSingleObject(event, INFINITE);

			//イベントハンドルを閉じる
			CloseHandle(event);
		}

		/*実行が終わったらコマンドリストは不要になるため、
		  中身をクリアする*/
		  //キューをクリア
		_cmdAllocator->Reset();
		//実行を終了し、命令フェーズに移行する
		_cmdList->Reset(_cmdAllocator, nullptr);

		//ため込んだ命令の実行↑
		//-------------------------------------------------------------------
		//画面のスワップ
		/*命令の実行が完了したら、フリップを行う*/
		_swapchain->Present(1, 0);

#pragma endregion
	}
#pragma endregion

	//もうクラスは使わないので登録を解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}

