/*==============================================================================

   shader field [shaderfield.cpp]
														 Author : PYAE SONE THANT
														 Date   : 2025/09/10
--------------------------------------------------------------------------------

==============================================================================*/
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "debug_ostream.h"
#include <fstream>
#include "shader.h"
#include "sampler.h"
#include "shader_field.h"

// NEW: Define structs to match HLSL cbuffer registers b3 and b4
struct AmbientLightBuffer
{
	DirectX::XMFLOAT4 ambient_color;
};

struct DirectionalLightBuffer
{
	DirectX::XMFLOAT4 directional_world_vector;
	DirectX::XMFLOAT4 directional_color;
};


static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer3 = nullptr; // NEW: Ambient Light (b3)
static ID3D11Buffer* g_pVSConstantBuffer4 = nullptr; // NEW: Directional Light (b4)
static ID3D11PixelShader* g_pPixelShader = nullptr;

// Note! These are set externally during initialization. No need to release.
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


bool ShaderField_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	HRESULT hr;

	// Check device and device context
	if (!pDevice || !pContext) {
		hal::dout << "Shader3d_Initialize() have failed" << std::endl;
		return false;
	}

	// Save device and device context
	g_pDevice = pDevice;
	g_pContext = pContext;


	// Load precompiled vertex shader
	std::ifstream ifs_vs("shader_vertex_Field.cso", std::ios::binary);

	if (!ifs_vs) {
		MessageBox(nullptr, "Shader vertex failed\n\nshader_vertex_Field.cso", "Error", MB_OK);
		return false;
	}

	// Get file size
	ifs_vs.seekg(0, std::ios::end);
	ifs_vs.seekg(0, std::ios::end); // Move file pointer to the end
	std::streamsize filesize = ifs_vs.tellg(); // Current pointer position = file size
	ifs_vs.seekg(0, std::ios::beg); // Reset pointer to the beginning


	// Allocate buffer for binary data
	unsigned char* vsbinary_pointer = new unsigned char[filesize];

	ifs_vs.read((char*)vsbinary_pointer, filesize); // Read binary data
	ifs_vs.close(); // Close file


	// Create vertex shader
	hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader);

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() Failed to load shader" << std::endl;
		delete[] vsbinary_pointer; // Release buffer to prevent memory leak
		return false;
	}


	// Define vertex layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT num_elements = ARRAYSIZE(layout); // Get array element count
	// Create vertex layout
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout);

	delete[] vsbinary_pointer; // Release binary buffer

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() Failed to load shader" << std::endl;
		return false;
	}


	// Create constant buffers for vertex shader (b0, b1, b2 - Matrices)
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMFLOAT4X4); // Buffer size (Matrix)
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;  // Bind flag

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer0);

	// NEW: Create constant buffer for Ambient Light (b3)
	D3D11_BUFFER_DESC buffer_desc_b3{};
	buffer_desc_b3.ByteWidth = sizeof(AmbientLightBuffer);
	buffer_desc_b3.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	g_pDevice->CreateBuffer(&buffer_desc_b3, nullptr, &g_pVSConstantBuffer3);

	// NEW: Create constant buffer for Directional Light (b4)
	D3D11_BUFFER_DESC buffer_desc_b4{};
	buffer_desc_b4.ByteWidth = sizeof(DirectionalLightBuffer);
	buffer_desc_b4.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	g_pDevice->CreateBuffer(&buffer_desc_b4, nullptr, &g_pVSConstantBuffer4);

	// Load precompiled pixel shader
	// Load precompiled pixel shader
	std::ifstream ifs_ps("shader_pixel_3d_field.cso", std::ios::binary);
	if (!ifs_ps) {
		MessageBox(nullptr, "Failed to load pixel shader\n\nshader_pixel_3d_field.cso", "Error", MB_OK);
		return false;
	}

	ifs_ps.seekg(0, std::ios::end);
	filesize = ifs_ps.tellg();
	ifs_ps.seekg(0, std::ios::beg);

	unsigned char* psbinary_pointer = new unsigned char[filesize];
	ifs_ps.read((char*)psbinary_pointer, filesize);
	ifs_ps.close();

	// Create pixel shader
	hr = g_pDevice->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader);

	delete[] psbinary_pointer;  // Release binary buffer

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() Failed to load shader" << std::endl;
		return false;
	}
	return true;
}

void ShaderField_Finalize()
{
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVSConstantBuffer4); // NEW: Release directional light buffer
	SAFE_RELEASE(g_pVSConstantBuffer3); // NEW: Release ambient light buffer
	SAFE_RELEASE(g_pVSConstantBuffer0);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexShader);
}

// ... existing matrix setters ...

// NEW: Setter for Ambient Light (b3)
void ShaderField_SetAmbientColor(const DirectX::XMFLOAT4& color)
{
	AmbientLightBuffer buffer = { color };
	g_pContext->UpdateSubresource(g_pVSConstantBuffer3, 0, nullptr, &buffer, 0, 0);
}

// NEW: Setter for Directional Light (b4)
void ShaderField_SetDirectionalLight(const DirectX::XMFLOAT4& direction, const DirectX::XMFLOAT4& color)
{
	DirectionalLightBuffer buffer = { direction, color };
	g_pContext->UpdateSubresource(g_pVSConstantBuffer4, 0, nullptr, &buffer, 0, 0);
}

void ShaderField_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
	// Define structure for storing constant buffer matrix
	XMFLOAT4X4 transpose;

	// Transpose matrix and store into buffer structure
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// Set matrix into constant buffer
	g_pContext->UpdateSubresource(g_pVSConstantBuffer0, 0, nullptr, &transpose, 0, 0);
}

void ShaderField_Begin()
{
	// Set vertex shader and pixel shader into the rendering pipeline
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pPixelShader, nullptr, 0);

	// Set vertex layout into the rendering pipeline
	g_pContext->IASetInputLayout(g_pInputLayout);

	// Set constant buffers into the rendering pipeline
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer0);
	g_pContext->VSSetConstantBuffers(3, 1, &g_pVSConstantBuffer3); // NEW: Bind Ambient Light (b3)
	g_pContext->VSSetConstantBuffers(4, 1, &g_pVSConstantBuffer4); // NEW: Bind Directional Light (b4)

	// Set sampler state into the rendering pipeline
	Sampler_SetFilterAnisotropic();
}