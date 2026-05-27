#include "KamataEngine.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;
using namespace Microsoft::WRL;

//
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	// inputElementDescsの作成
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";                        // 頂点シェーダー側のセマンティクス
	inputElementDescs[0].SemanticIndex = 0;                                // セマンティクスインデックス
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;          // データの形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値

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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// KamataEngineの初期化
	KamataEngine::Initialize(L"タイトルバー");

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

	/// VertexResourceの作成
	// 頂点リソース用のヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // アップロード可能なヒープ

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc = {};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // リソースの次元
	vertexResourceDesc.Width = sizeof(Vector4) * 3;                 // 頂点データ全体のサイズ

	// バッファの場合は1にする
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作成する
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr =
	    dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	/// VertexBufferViewの作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	// リソースの先頭アドレスを指定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用する頂点リソースのサイズを指定
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;

	// 頂点1つ分のサイズを指定
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// 頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f}; // 頂点1の位置
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};   // 頂点2の位置
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};  // 頂点3の位置
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画前処理
		dxCommon->PreDraw();

		/// ここに描画処理を記述

		// PSOの設定
		commandList->SetPipelineState(pipelineState.Get());

		// ルートシグネチャの設定
		commandList->SetGraphicsRootSignature(rs.GetRootSignature());

		// 頂点バッファビューの設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

		// プリミティブトポロジーの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画コマンド
		commandList->DrawInstanced(3, 1, 0, 0);

		// 描画後処理
		dxCommon->PostDraw();
	}

	// 解放処理

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}