#include "depthShader.h"
#include <fstream>

DepthShaderClass::DepthShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
}

DepthShaderClass::~DepthShaderClass() {}

bool DepthShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	HRESULT hr;
	std::ifstream ifs_vs("depth_vertex.cso", std::ios::binary);
	if (!ifs_vs) {
		MessageBox(nullptr, "Shader vertex failed\n\ndepth_vertex.cso", "Error", MB_OK);
		return false;
	}

	ifs_vs.seekg(0, std::ios::end);
	std::streamsize vs_filesize = ifs_vs.tellg();
	ifs_vs.seekg(0, std::ios::beg);

	unsigned char* vs_data = new unsigned char[vs_filesize];
	ifs_vs.read((char*)vs_data, vs_filesize);
	ifs_vs.close();

	hr = device->CreateVertexShader(vs_data, vs_filesize, nullptr, &m_vertexShader);
	if (FAILED(hr)) {
		delete[] vs_data;
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(layout, 1, vs_data, vs_filesize, &m_layout);
	delete[] vs_data; // Clean up buffer after layout creation
	if (FAILED(hr)) return false;


	// --- Load Pixel Shader Binary (.cso) ---
	std::ifstream ifs_ps("depth_pixel.cso", std::ios::binary);
	if (!ifs_ps) {
		MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\ndepth_pixel.cso", "Error", MB_OK);
		return false;
	}

	ifs_ps.seekg(0, std::ios::end);
	std::streamsize ps_filesize = ifs_ps.tellg();
	ifs_ps.seekg(0, std::ios::beg);

	unsigned char* ps_data = new unsigned char[ps_filesize];
	ifs_ps.read((char*)ps_data, ps_filesize);
	ifs_ps.close();

	hr = device->CreatePixelShader(ps_data, ps_filesize, nullptr, &m_pixelShader);
	delete[] ps_data;
	if (FAILED(hr)) return false;


	// --- Create Matrix Constant Buffer ---
	D3D11_BUFFER_DESC matrixBufferDesc{};
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = device->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);
	if (FAILED(hr)) return false;

	return true;
}
void DepthShaderClass::Shutdown()
{
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
	if (m_layout) { m_layout->Release(); m_layout = 0; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

bool DepthShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* texture)
{
	// 1. Set the matrices (World, View, Projection)
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix)) return false;

	// 2. Bind the player texture so the Pixel Shader can discard transparent pixels
	if (texture)
	{
		deviceContext->PSSetShaderResources(0, 1, &texture);
	}

	// 3. Draw the geometry
	RenderShader(deviceContext, indexCount);

	return true;
}
bool DepthShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose matrices for HLSL
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) return false;

	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	return true;
}

void DepthShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}