#include "PipelineState.h"
#include "KamataEngine.h"

using namespace Microsoft::WRL;
using namespace KamataEngine;

void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// パイプラインステートの作成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(
		&desc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));

	// 生成されたパイプラインステートをメンバ変数にセットする
	pipelineState_ = pipelineState;
}

ID3D12PipelineState* PipelineState::Get() const { return pipelineState_.Get(); }

PipelineState::PipelineState() {}

PipelineState::~PipelineState() { pipelineState_ = nullptr; }