#pragma once
#include <d3d12.h>
#include <wrl.h>
class VertexBuffer {
public:
	// 生成
	void Create(const UINT size, const UINT stride);
   
	// ゲッター
	ID3D12Resource* GetResource();
	D3D12_VERTEX_BUFFER_VIEW* GetView();

	// デストラクタ
	~VertexBuffer();

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
};
