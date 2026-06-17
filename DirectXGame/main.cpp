#include "IndexBuffer.h"
#include "KamataEngine.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;
using namespace Microsoft::WRL;

//
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	// inputElementDescsの作成
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";                        // 頂点シェーダー側のセマンティクス
	inputElementDescs[0].SemanticIndex = 0;                                // セマンティクスインデックス
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;          // データの形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値
	inputElementDescs[1].SemanticName = "TEXCOORD";                        // 頂点シェーダー側のセマンティクス
	inputElementDescs[1].SemanticIndex = 0;                                // セマンティクスインデックス
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;                // データの形式
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};                 // 入力レイアウトの説明構造体
	inputLayoutDesc.pInputElementDescs = inputElementDescs;    // 入力要素の配列
	inputLayoutDesc.NumElements = _countof(inputElementDescs); // 入力要素の数

	// BlendStateの作成
	D3D12_BLEND_DESC blendDesc = {};

	// 全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerStateの作成
	D3D12_RASTERIZER_DESC rasterizerDesc = {};

	//  裏面をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

	// 塗りつぶしモードをソリッドにする
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rs.GetRootSignature();                                       // ルートシグネチャ
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // 入力レイアウト
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // 頂点シェーダー
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // ピクセルシェーダー
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // ブレンドステート
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                       // 書き込むRTVの数
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // RTVのフォーマット

	// 利用するトロポジーの情報
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // トポロジーのタイプ

	// どのように画面に色を表示するかの情報
	graphicsPipelineStateDesc.SampleDesc.Count = 1;                   // サンプリング数
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準のサンプルマスク

	// PSOの作成
	pipelineState.Create(graphicsPipelineStateDesc);
}

/// 関数プロトタイプ宣言
// レンダーテクスチャーリソースの作成
ComPtr<ID3D12Resource> CreateRenderTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT clearformat, const FLOAT* clearColor) {

	// 描画先のテクスチャーリソースを作成する
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Width = UINT(width);                             // テクスチャーの幅
	resourceDesc.Height = UINT(height);                           // テクスチャーの高さ
	resourceDesc.DepthOrArraySize = 1;                            // 配列サイズ
	resourceDesc.MipLevels = 1;                                   // ミップレベル数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             // フォーマット
	resourceDesc.SampleDesc.Count = 1;                            // サンプリング数
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2Dテクスチャー
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして使用可能

	// 利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // デフォルトヒープ

	// クリアヴァリューの設定
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = clearformat;     // クリアする値のフォーマット
	clearValue.Color[0] = clearColor[0]; // クリアする値の赤成分
	clearValue.Color[1] = clearColor[1]; // クリアする値の緑成分
	clearValue.Color[2] = clearColor[2]; // クリアする値の青成分
	clearValue.Color[3] = clearColor[3]; // クリアする値のアルファ成分

	// 描画先のテクスチャーリソースを作成する
	ComPtr<ID3D12Resource> renderTextureResource;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                       // ヒーププロパティ
	    D3D12_HEAP_FLAG_NONE,                  // ヒープフラグ
	    &resourceDesc,                         // リソース記述子
	    D3D12_RESOURCE_STATE_COMMON,           // 初期リソース状態
	    &clearValue,                           // 最適化されたクリア値
	    IID_PPV_ARGS(&renderTextureResource)); // 作成されたリソースへのポインタ
	assert(SUCCEEDED(hr));
	return renderTextureResource;
}

/// 関数プロトタイプ宣言
// depthStencilリソースの作成
ComPtr<ID3D12Resource> CreateDepthStencilResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height) {

	// 深度ステンシルバッファーのリソースを作成する
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Width = UINT(width);                             // テクスチャーの幅
	resourceDesc.Height = UINT(height);                           // テクスチャーの高さ
	resourceDesc.DepthOrArraySize = 1;                            // 配列サイズ
	resourceDesc.MipLevels = 1;                                   // ミップレベル数
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;                  // フォーマット
	resourceDesc.SampleDesc.Count = 1;                            // サンプリング数
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2Dテクスチャー
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // 深度ステンシルとして使用可能

	// 利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // デフォルトヒープ

	// クリアヴァリューの設定
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT; // クリアする値のフォーマット
	clearValue.DepthStencil.Depth = 1.0f;      // 深度バッファーをクリアする値

	// 深度ステンシルバッファーのリソースを作成する
	ComPtr<ID3D12Resource> depthStencilResource;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                      // ヒーププロパティ
	    D3D12_HEAP_FLAG_NONE,                 // ヒープフラグ
	    &resourceDesc,                        // リソース記述子
	    D3D12_RESOURCE_STATE_DEPTH_WRITE,     // 初期リソース状態
	    &clearValue,                          // 最適化されたクリア値
	    IID_PPV_ARGS(&depthStencilResource)); // 作成されたリソースへのポインタ
	assert(SUCCEEDED(hr));
	return depthStencilResource;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// KamataEngineの初期化
	KamataEngine::Initialize(L"CG5_LE3D_16_タナベ");

	// DirectXCommonのインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXのウィンドウの幅と高さを取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf("ウィンドウの幅: %d, 高さ: %d\n", w, h);

	// DirectXCommonのコマンドリストを取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	/// RootSignatureの作成
	RootSignature rs;
	rs.Create();

	Shader vsShader;
	vsShader.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vsShader.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader psShader;
	psShader.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(psShader.GetDxcBlob() != nullptr);

	// パイプラインステートの作成
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vsShader, psShader);

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	// 頂点データの準備
	VertexData vertices[] = {
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 左上
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, // 右下
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}, // 右上
	};

	/// VertexResourceの作成
	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	/// 頂点リソースにデータを書き込む
	VertexData* pGpuVertices = nullptr;
	vb.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	// 頂点リソースをマップして、CPUから書き込めるようにする
	for (int i = 0; i < _countof(vertices); i++) {
		pGpuVertices[i] = vertices[i];
	}

	// 頂点データの準備
	uint16_t indices[] = {0, 1, 2, 2, 1, 3}; // 頂点のインデックス

	// indexBuffer
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点データを書き込む
	uint16_t* pGpuIndices = nullptr;
	ib.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	// 頂点リソースをマップして、CPUから書き込めるようにする
	for (int i = 0; i < _countof(indices); i++) {
		pGpuIndices[i] = indices[i];
	}

	/// リソースの生成
	ComPtr<ID3D12Device> device = dxCommon->GetDevice();
	HRESULT hr;

	// レンダーテクスチャーリソースの作成
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f}; // レンダーテクスチャーのクリアカラー

	ComPtr<ID3D12Resource> renderTextureResource = CreateRenderTextureResource(device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, kRenderTargetClearColor);

	// RTV用のデスクリプタヒープの作成
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 1;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	assert(SUCCEEDED(hr));

	// CPU側からハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

	/// RTV用のviewの作成
	device->CreateRenderTargetView(renderTextureResource.Get(), nullptr, rtvHandle);

	/// 深度ステンシルバッファーのリソースの作成
	ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilResource(device, w, h);

	// DSV用のデスクリプタヒープの作成
	ComPtr<ID3D12DescriptorHeap> dsvHeap;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	assert(SUCCEEDED(hr));

	// CPU側からハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// DSV用のviewの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// DSVHeapの先頭にDSVを作成する
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvHandle);

	// SRV用のデスクリプタヒープの作成
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NumDescriptors = 1;
	hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(hr));

	// CPU側からハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart());

	// GPU側からハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(srvHeap->GetGPUDescriptorHandleForHeapStart());

	// SRV用のviewの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// SRVHeapの先頭にSRVを作成する
	device->CreateShaderResourceView(renderTextureResource.Get(), &srvDesc, srvHandle);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画前処理
		dxCommon->PreDraw();

		// TransitionBarrierをSRVからRTVに変更する
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrier);

		// レンダーターゲットと深度ステンシルをセット
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Viewportの設定
		D3D12_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(w);
		viewport.Height = static_cast<float>(h);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		commandList->RSSetViewports(1, &viewport);

		// ScissorRectの設定
		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = w;
		scissorRect.bottom = h;
		commandList->RSSetScissorRects(1, &scissorRect);

		// 全画面をクリア
		commandList->ClearRenderTargetView(rtvHandle, kRenderTargetClearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		/// ここに描画処理を記述


		// TransitionBarrierをRTVからSRVに変更する
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// PSOの設定
		commandList->SetPipelineState(pipelineState.Get());

		// ルートシグネチャの設定
		commandList->SetGraphicsRootSignature(rs.GetRootSignature());

		// 頂点バッファビューの設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView());

		// IBVを設定
		commandList->IASetIndexBuffer(ib.GetView());

		// プリミティブトポロジーの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画コマンド
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画後処理
		dxCommon->PostDraw();
	}

	// 解放処理

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}