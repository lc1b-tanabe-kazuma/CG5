#pragma once
#include <d3d12.h>
#include <wrl.h>
class IndexBuffer {
public:

	// 生成
	void Create(const UINT size, const UINT stride);

	// ゲッター
	ID3D12Resource* GetResource();
	D3D12_INDEX_BUFFER_VIEW* GetView();

	// デストラクタ
	~IndexBuffer();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
};