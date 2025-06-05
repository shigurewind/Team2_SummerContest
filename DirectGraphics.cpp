#include <stdio.h>
#include <atlstr.h>
#include <tchar.h>
#include <DirectXMath.h>
#include "DirectGraphics.h"
#include "ShaderBase.h"
#include "Window.h"
#include "GraphicsUtility.h"

#pragma comment(lib,"d3d11.lib")

bool DirectGraphics::Init()
{
	// DeviceとSwapChainの作成
	if (CreateDeviceAndSwapChain() == false)
	{
		return false;
	}

	// RenderTargetViewの作成
	if (CreateRenderTargetView() == false)
	{
		return false;
	}

	// Depth、StencilViewの作成
	if (CreateDepthAndStencilView() == false)
	{
		return false;
	}

	if (CreateTextureSampler() == false)
	{
		return false;
	}

	if (CreateConstantBuffer() == false)
	{
		return false;
	}

	// シェーダ作成
	if (CreateShader() == false)
	{
		return false;
	}

	// ViewPort設定
	SetUpViewPort();

	// 変換行列設定
	SetUpTransform();

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ID3D11RasterizerState* state;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	if (FAILED(m_Device->CreateRasterizerState(&rasterizerDesc, &state)))
	{
		return false;
	}

	m_Context->RSSetState(state);
	return true;
}

void DirectGraphics::Release()
{
	if (m_VertexShader != nullptr)
	{
		delete m_VertexShader;
		m_VertexShader = nullptr;
	}

	if (m_PixelShader != nullptr)
	{
		delete m_PixelShader;
		m_PixelShader = nullptr;
	}

	if (m_Context != nullptr)
	{
		m_Context->ClearState();
		m_Context->Flush();
		m_Context->Release();
		m_Context = nullptr;
	}

	if (m_DepthStencilView != nullptr)
	{
		m_DepthStencilView->Release();
		m_DepthStencilView = nullptr;
	}

	if (m_RenderTargetView != nullptr)
	{
		m_RenderTargetView->Release();
		m_RenderTargetView = nullptr;
	}

	if (m_SwapChain != nullptr)
	{
		m_SwapChain->Release();
		m_SwapChain = nullptr;
	}

	if (m_Device != nullptr)
	{
		m_Device->Release();
		m_Device = nullptr;
	}
}

void DirectGraphics::StartRendering()
{
	float clear_color[ 4 ] = { 1.0f, 0.5f, 0.5f, 1.0f };	// RenderTarget塗りつぶしカラー(RGBA)

	// DirecX9ではRenderTargetとDepth、Stencilバッファのクリアは別々にする
    m_Context->ClearRenderTargetView( 
				m_RenderTargetView,							// クリア対象のView
				clear_color );								// クリアカラー

    m_Context->ClearDepthStencilView( 
				m_DepthStencilView,							// クリア対象のView	
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,	// クリアフラグ(今回はDepth、Stencilともにクリア)
				1.0f,										// 深度クリア値
				0 );										// ステンシルクリア値
}

void DirectGraphics::FinishRendering()
{
	// 描画コマンドを送信する
	m_SwapChain->Present(
			/*
				垂直同期のタイミング
					0なら即時描画
					1以上なら指定した値の垂直同期後に描画
			*/
			0,
			/*
				出力オプション
					フレーム出力を行うためのオプションで
					基本は0で問題ないと思われる
					その他のフラグはDXGI_PRESENTで確認可能
			*/
			0 );	// 出力オプション
}


void DirectGraphics::SetMaterial(ObjMaterial* material)
{
	m_ConstantBufferData.MaterialAmbient = DirectX::XMFLOAT4(material->Ambient[0], material->Ambient[1], material->Ambient[2], 1);
	m_ConstantBufferData.MaterialDiffuse = DirectX::XMFLOAT4(material->Diffuse[0], material->Diffuse[1], material->Diffuse[2], 1);
	m_ConstantBufferData.MaterialSpecular = DirectX::XMFLOAT4(material->Specular[0], material->Specular[1], material->Specular[2], 1);
}


void DirectGraphics::SetTexture(ID3D11ShaderResourceView* texture)
{
	// Samplerの設定
	m_Context->PSSetSamplers(
		0,					// スロット番号
		1,					// サンプラーの数
		&m_SamplerState);	// ID3D11SamplerState

	// PixelShaderで使用するテクスチャの設定
	m_Context->PSSetShaderResources(
		0,								// スロット番号
		1,								// リソースの数
		&texture);						// ID3D11ShaderResourceView

}

void DirectGraphics::SetUpTransform()
{
	HWND window_handle = FindWindow(Window::ClassName, nullptr);
	RECT rect;
	GetClientRect(window_handle, &rect);

	// Viewマトリクス設定
	DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 2.0f, -5.0f, 0.0f);
	DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX view_matrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

	// プロジェクションマトリクス設定
	float    fov = DirectX::XMConvertToRadians(45.0f);
	float    aspect = (float)(rect.right - rect.left) / (rect.bottom  - rect.top);
	float    nearZ = 0.1f;
	float    farZ = 1000.0f;
	DirectX::XMMATRIX proj_matrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

	// ライトの設定
	DirectX::XMVECTOR light = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));

	// コンスタントバッファの設定
	XMStoreFloat4x4(&m_ConstantBufferData.View, XMMatrixTranspose(view_matrix));
	XMStoreFloat4x4(&m_ConstantBufferData.Projection, XMMatrixTranspose(proj_matrix));
	XMStoreFloat4(&m_ConstantBufferData.LightVector, light);
	XMStoreFloat4(&m_ConstantBufferData.CameraPos, eye);

	// ライトのカラー設定
	m_ConstantBufferData.LightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1);
}

void DirectGraphics::SetUpContext()
{
	// プリミティブの形状を指定
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VerteXShader、PixelShaderを設定
	m_Context->VSSetShader(
		m_VertexShader->GetShaderInterface(),	// 使用するVertexShder
		/*
			ClassInstanceのポインタを設定する
				これはShader作成時のID3D11ClassLinkageを使用した場合に
				用いる項目なので今回はnullptrを指定する
		*/
		nullptr,
		0);									// ClassInstanceの数
	m_Context->PSSetShader(m_PixelShader->GetShaderInterface(), nullptr, 0);

	// (OutputManger)RnderTagetの指定
	m_Context->OMSetRenderTargets(
		1,							// 使用するViewの数
		&m_RenderTargetView,		// 使用するRenderTargetView
		m_DepthStencilView);		// 使用するDepthStencilView
}

void DirectGraphics::SetUpDxgiSwapChainDesc(DXGI_SWAP_CHAIN_DESC *dxgi)
{
	HWND window_handle = FindWindow(Window::ClassName, nullptr);
	RECT rect;
	GetClientRect(window_handle, &rect);

	/*
		DirectX11版PresentationParameter
			バッファの数やサイズ、カラーフォーマット等を設定する
	*/
	ZeroMemory(dxgi, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgi->BufferCount = 1;									// バッファの数
	dxgi->BufferDesc.Width = (rect.right - rect.left);		// バッファの横幅
	dxgi->BufferDesc.Height = (rect.bottom - rect.top);		// バッファの縦幅
	dxgi->BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// カラーフォーマット
	dxgi->BufferDesc.RefreshRate.Numerator = 60;			// リフレッシュレートの分母
	dxgi->BufferDesc.RefreshRate.Denominator = 1;			// リフレッシュレートの分子
	dxgi->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// バッファの使い方 Usage => 使用方法
	dxgi->OutputWindow = window_handle;						// 出力対象のウィンドウハンドル
	dxgi->SampleDesc.Count = 1;								// マルチサンプリングのサンプル数(未使用は1)
	dxgi->SampleDesc.Quality = 0;							// マルチサンプリングの品質(未使用は0)
	dxgi->Windowed = true;									// ウィンドウモード指定
}

bool DirectGraphics::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC dxgi;
	SetUpDxgiSwapChainDesc(&dxgi);

	D3D_FEATURE_LEVEL level;
	// デバイス生成とスワップチェーン作成を行う
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,					// ビデオアダプタ指定(nullptrは既定)
		D3D_DRIVER_TYPE_HARDWARE,	// ドライバのタイプ
		nullptr,					// D3D_DRIVER_TYPE_SOFTWARE指定時に使用
		0,							// フラグ指定
		nullptr,					// D3D_FEATURE_LEVEL指定で自分で定義した配列を指定可能
		0,							// 上のD3D_FEATURE_LEVEL配列の要素数
		D3D11_SDK_VERSION,			// SDKバージョン
		&dxgi,						// DXGI_SWAP_CHAIN_DESC
		&m_SwapChain,				// 関数成功時のSwapChainの出力先 
		&m_Device,					// 関数成功時のDeviceの出力先
		&level,						// 成功したD3D_FEATURE_LEVELの出力先
		&m_Context)))				// 関数成功時のContextの出力先
	{
		return false;
	}

	return true;
}

bool DirectGraphics::CreateRenderTargetView()
{
	// RenderTargetViewの対象となるBufferの取得
	ID3D11Texture2D* back_buffer;
	if (FAILED(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer)))
	{
		return false;
	}
	
	// BufferからRenderTargetViewの作成
	if (FAILED(m_Device->CreateRenderTargetView(back_buffer, NULL, &m_RenderTargetView)))
	{
		return false;
	}
	
	// Targetの取得終わったのでBufferを解放
	back_buffer->Release();

	return true;
}

bool DirectGraphics::CreateDepthAndStencilView()
{
	HWND window_handle = FindWindow(Window::ClassName, nullptr);
	RECT rect;
	GetClientRect(window_handle, &rect);

	//深度ステンシルバッファ作成
    D3D11_TEXTURE2D_DESC texture_desc;
    ZeroMemory( &texture_desc, sizeof( D3D11_TEXTURE2D_DESC ) );
    texture_desc.Width              = (rect.right - rect.left);			// 横幅
    texture_desc.Height             = (rect.bottom - rect.top);			// 縦幅
	/*
		ミップマップのレベル指定
			どのレベルまで生成するかという設定
			1はマルチサンプリングされたテクスチャを使用するあるので、
			ミップマップはなしと考えられる
			0は全生成とあるので可能な限りのレベルまでテクスチャが生成されると思われる
	*/
    texture_desc.MipLevels          = 1;
	/*
		テクスチャ配列のサイズ指定
			テクスチャ配列について調べ切れていないので
			他のサンプルと同様に1を設定しておく
	*/
    texture_desc.ArraySize          = 1;
	/*
		テクスチャのフォーマット
			DXGI_FORMAT_D24_UNORM_S8_UINTを使用する
			これはおそらくDepth24bit、Stencil8bitとされると思う

			※．UNORMはUnsigned NORMalizedの略で指定された範囲を0.0～1.0にするみたい
	*/
    texture_desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;	// テクスチャーフォーマット
	// マルチサンプリング設定(使わない)
	texture_desc.SampleDesc.Count   = 1;								
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage              = D3D11_USAGE_DEFAULT;				// テクスチャの使用方法(デフォルト)
    texture_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;			// Bind設定はDepth、Stencilに設定
	/*
		リソースへのCPUのアクセス権限についての設定
			ReadとWriteがあるが、権限について現状は考える必要はないはずなので、
			デフォルト値であると思われる0をしておく

			※．readとwriteはenum(D3D11_CPU_ACCESS_FLAG)で用意されていた

	*/
    texture_desc.CPUAccessFlags     = 0;
	/*
		リソースオプションのフラグ
			Microsoft Docsでフラグを確認する限りは
			通常使用でフラグを設定する必要はないと思われるので
			0としておく
	*/
    texture_desc.MiscFlags          = 0;

	// texture_descの情報でテクスチャを作成
	if (FAILED(m_Device->CreateTexture2D(&texture_desc, NULL, &m_DepthStencilTexture)))
	{
		return false;
	}

	// Depth、StencilViewの設定
	// DepthStencilView 
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
    ZeroMemory( &dsv_desc, sizeof( dsv_desc ) );
    dsv_desc.Format             = texture_desc.Format;				// Viewのフォーマット(Textureのものを使用)
	/*
		DSVが何次元であるかとTextureの種類を設定する値
			D3D11_DSV_DIMENSION_TEXTURE2Dのほかにも
			D3D11_DSV_DIMENSION_TEXTURE1Dや
			D3D11_DSV_DIMENSION_TEXTURE2D_ARRAYなどがあったので
			DSVが何次元であるかとTextureの種類を設定するメンバと思われる
			今回は何も通常のTextureとして用意しているはず、
			なので、D3D11_DSV_DIMENSION_TEXTURE2Dを指定
	*/
    dsv_desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;	
    dsv_desc.Texture2D.MipSlice = 0;								// 最初に使用するミップマップのレベルを指定 

	// CreateTexture2Dとdsv_descからDepthとStencilバッファを作る
	if (FAILED(m_Device->CreateDepthStencilView(
				m_DepthStencilTexture,				// DSVとして使用されるTexture
				&dsv_desc,							// DSVの設定
				&m_DepthStencilView)))				// ID3D11DepthStencilViewの出力先
	{
		return false;
	}

	return true;
}

bool DirectGraphics::CreateShader()
{
	/*
		シェーダの生成はクラスを生成して行う
		※シェーダクラスは自前

		クラス生成した理由
			・ShaderInterface生成のメソッドがShader毎に異なる
				例：
					PixelShader => CreatePixelShader
					VertexShader => CreateVertexShader

			・元データとサイズを使用することがある
				VertexShaderで使用するVertexShaderの元データとサイズを必要とすることがあり、
				他のShaderでも起こりえる可能性があるためクラスを作り、
				そこで保持するようにした

		シェーダはcso(コンパイラ済み)を使用する
	*/
	m_VertexShader = new VertexShader();
	if (m_VertexShader->Create(m_Device, "Res/Shader/VertexShader.cso") == false)
	{
		return false;
	}

	m_PixelShader = new PixelShader();
	if (m_PixelShader->Create(m_Device, "Res/Shader/PixelShader.cso") == false)
	{
		return false;
	}

	return true;
}

void DirectGraphics::SetUpViewPort()
{
 	HWND window_handle = FindWindow(Window::ClassName, nullptr);
	RECT rect;
	GetClientRect(window_handle, &rect);

	//ビューポートの設定
	D3D11_VIEWPORT view_port;
	view_port.TopLeftX = 0;								// 左上X座標
	view_port.TopLeftY = 0;								// 左上Y座標
	view_port.Width = (float)(rect.right - rect.left);	// 横幅
	view_port.Height = (float)(rect.bottom - rect.top);	// 縦幅
	view_port.MinDepth = 0.0f;							// 最小深度
	view_port.MaxDepth = 1.0f;							// 最大深度

	m_Context->RSSetViewports(	
		1,					// 設定するビューポートの数
		&view_port );		// 設定するビューポート情報のポインタ
}

bool DirectGraphics::CreateTextureSampler()
{
	D3D11_SAMPLER_DESC sampler_desc;

	ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// サンプリング時の補間方法
	// UVW値が0.0～1.0の範囲外になった場合の対応設定
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	if (FAILED(m_Device->CreateSamplerState(&sampler_desc, &m_SamplerState)))
	{
		return false;
	}

	return true;
}

bool DirectGraphics::CreateConstantBuffer()
{
	/*
		Constantバッファー作成
			コンスタントバッファーはCPU側のデータを
			まとめてGPU側に送信するためのバッファー

			バッファーには座標変換行列などを設定する
	*/
	D3D11_BUFFER_DESC buffer_desc;
	buffer_desc.ByteWidth = sizeof(ConstantBuffer);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	if (FAILED(m_Device->CreateBuffer(&buffer_desc, nullptr, &m_ConstantBuffer)))
	{
		return false;
	}

	return true;
}

