#pragma once
#include <d3d12.h>
#include <wrl.h>

class PipelineState {
public:
	// 生成
	void Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
	
	// ゲッター
	ID3D12PipelineState* Get() const;
	
	// コンストラクタ
	PipelineState();
	
	// デストラクタ
	~PipelineState();
private:

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};