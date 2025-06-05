#ifndef DIRECT_GRAPHICS_H_
#define DIRECT_GRAPHICS_H_

#include <map>
#include <string>

#include <d3d11.h>
#include "WICTextureLoader.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "GraphicsUtility.h"

//=====================================================================//
//! DirectX11の機能を実装するクラス
//=====================================================================//
class DirectGraphics
{
public:
	static DirectGraphics* GetInstance()
	{
		static DirectGraphics instance;
		return &instance;
	}
public:

	/** Destructor */
	~DirectGraphics() {}

	/**
	* @brief 初期化関数@n
	* DirectX11の初期化を行い、成功したらtrue、失敗したらfalseを返す
	* @return 初期化の成否 成功(true)
	*/
	bool Init();

	/**
	* @brief DirectX11の解放関数@n
	* 保持しているデバイスなどを解放する
	*/
	void Release();

	/**
	* @brief 描画開始関数@n
	* 描画処理を行う場合、必ずこの関数の後に実行する@n
	* 実行しないと描画されない
	*/
	void StartRendering();

	/**
	* @brief 描画終了関数@n
	* 描画処理が完了した場合、必ずこの関数を実行する
	* 実行しないと描画内容が反映されない
	*/
	void FinishRendering();

	void SetUpTransform();

	void SetUpContext();

	ID3D11Device* GetDevice()
	{
		return m_Device;
	}

	VertexShader* GetVertexShader()
	{
		return m_VertexShader;
	}

	ID3D11Buffer* GetConstantBuffer()
	{
		return m_ConstantBuffer;
	}

	ConstantBuffer* GetConstantBufferData()
	{
		return &m_ConstantBufferData;
	}

	ID3D11DeviceContext* GetContext()
	{
		return m_Context;
	}

	void SetMaterial(ObjMaterial* material);

	void SetTexture(ID3D11ShaderResourceView* texture);

private:
	/** Constructor */
	DirectGraphics() :
		m_Device(nullptr),
		m_Context(nullptr),
		m_SwapChain(nullptr),
		m_RenderTargetView(nullptr),
		m_DepthStencilTexture(nullptr),
		m_DepthStencilView(nullptr),
		m_VertexShader(nullptr),
		m_PixelShader(nullptr)
	{
	}

	/**
	* @brief DXGI_SWAP_CHAIN_DESCの設定関数@n
	* SwapChainを作成するうえで必要な設定をDXGI_SWAP_CHAIN_DESCに行う
	* @param[out] dxgi 設定を行うDXGI_SWAP_CHAIN_DESCのポインタ
	*/
	void SetUpDxgiSwapChainDesc(DXGI_SWAP_CHAIN_DESC* dxgi);

	/**
	* @brief DeviceとSwapChainの作成関数@n
	* DirectX11のDeviceとSwapChainを作成する@n
	* まとめた理由は生成関数をD3D11CreateDeviceAndSwapChainにしたため
	* @return 作成の成否 成功(true)
	*/
	bool CreateDeviceAndSwapChain();

	/**
	* @brief RenderTargetViewの作成関数@n
	* DirectX11のRenderTargetViewを作成する
	* @return 作成の成否 成功(true)
	*/
	bool CreateRenderTargetView();

	/**
	* @brief DepthバッファStencilバッファ作成関数
	* @return 作成の成否 成功(true)
	*/
	bool CreateDepthAndStencilView();

	/**
	* @brief シェーダ作成関数@n
	* 今回のプロジェクトで使用するシェーダを作成する@n
	* @return 作成の成否 成功(true)
	*/
	bool CreateShader();

	/**
	* @brief ViewPort設定関数@n
	* ContextにViewPortの設定を行う関数@n
	* ゲーム中に変更がなければ最初に１度行えば問題ない
	*/
	void SetUpViewPort();

	/**
	* @brief TextureSampler作成関数@n
	* TextureSamplerの設定を行い、ID3D11SamplerStateを作成する
	* @return 作成の成否 成功(true)
	*/
	bool CreateTextureSampler();

	/**
	* @brief ConstantBuffer作成関数@n
	* TextureSamplerの設定を行い、ID3D11SamplerStateを作成する
	* @return 作成の成否 成功(true)
	*/
	bool CreateConstantBuffer();

private:
	ID3D11Device* m_Device;												//!< @brief DirectX11のDeviceのInterface
	ID3D11DeviceContext* m_Context;										//!< @brief Context
	IDXGISwapChain* m_SwapChain;										//!< @brief SwapChainのInterface
	ID3D11RenderTargetView* m_RenderTargetView;							//!< @brief RenderTargetViewのInterface
	ID3D11Texture2D* m_DepthStencilTexture;								//!< @brief ID3D11DepthStencilViewを生成するためのテクスチャ
	ID3D11DepthStencilView* m_DepthStencilView;							//!< @brief DepthStencilViewのInterface
	ID3D11SamplerState* m_SamplerState;									//!< @brief Textureサンプラー
	ID3D11Buffer* m_ConstantBuffer;										//!< @brief 定数バッファ
	VertexShader* m_VertexShader;										//!< @brief VertexShader保持用
	PixelShader* m_PixelShader;											//!< @brief PixelShader保持用
	ConstantBuffer m_ConstantBufferData;								//!< @brief ConstantBufferデータ
};

#endif
