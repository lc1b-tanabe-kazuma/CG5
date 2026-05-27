#pragma once
#include <d3d12.h>
#include <wrl.h>

class RootSignatuer {
public:
	// 生成
	void Create();

	// ゲッター
	ID3D12RootSignature* GetRootSignature() const;

	// コンストラクタ
	RootSignatuer();

	// デストラクタ
	~RootSignatuer();

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};