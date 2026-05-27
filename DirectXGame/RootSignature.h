#pragma once
#include <d3d12.h>
#include <wrl.h>

class RootSignature {
public:
	// 生成
	void Create();

	// ゲッター
	ID3D12RootSignature* GetRootSignature() const;

	// コンストラクタ
	RootSignature();

	// デストラクタ
	~RootSignature();

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};