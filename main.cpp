#include <d3d11.h>
#include <d3dcompiler.h>

#include <stdio.h>
#include <iostream>

#include "util.hpp"

int main() {
	HRESULT hr;

	D3D_FEATURE_LEVEL supportedFeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	ID3D11Device *device;
	ID3D11DeviceContext *deviceCtx;
	D3D_FEATURE_LEVEL usedFeatureLevel;

	/*******************************************
	 *    Creating device and device context   *
	 *******************************************/
	hr = D3D11CreateDevice(
			NULL,  // default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,  // no software renderer needed
			0,     // no flags needed
			supportedFeatureLevels, _countof(supportedFeatureLevels),
			D3D11_SDK_VERSION,
			&device, &usedFeatureLevel, &deviceCtx
			);

	/*******************************************
	 *      Creating compute shader object     *
	 *******************************************/
	ID3DBlob *shaderCode = 0, *shaderCompileErrors = 0;
	hr = D3DCompileFromFile(
			L"shader.hlsl", 
			NULL,  // no other defines or macros
			NULL,  // not including files
			"main", "cs_5_0", 
			0,     // in the future, use D3DCOMPILE_OPTIMIZATION_LEVEL3
			0,     // only used for effects; set to 0 for shaders
			&shaderCode, &shaderCompileErrors
			);
	if (hr) {
		util::printWinErr(hr, "Compiling code failed");
		if (shaderCompileErrors) {
			char *msg = (char*)shaderCompileErrors->GetBufferPointer();
			size_t msgSize = shaderCompileErrors->GetBufferSize();
			fwrite(msg, 1, msgSize, stdout);
		}
		return 2;
	}
 
	ID3D11ComputeShader *shader;
	hr = device->CreateComputeShader(
			shaderCode->GetBufferPointer(),
			shaderCode->GetBufferSize(),
			NULL,   // idk what this does
			&shader
			);
	if (hr) {
		util::printWinErr(hr, "Failed to create compute shader object");
		return 2;
	}

	/*******************************************
	 *           Making the buffers            *
	 *******************************************/
    int inputData[4] = {
        5, 10, 15, 20,
    };
    ID3D11Buffer *inputBuffer = 0, *outputBuffer = 0, *stagingBuffer = 0;
    D3D11_BUFFER_DESC inputBufferDesc = {}, outputBufferDesc = {}, stagingBufferDesc = {};

    inputBufferDesc.ByteWidth = 16;
    inputBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    inputBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    inputBufferDesc.CPUAccessFlags = 0;  // No CPU access necessary
    inputBufferDesc.MiscFlags = 0;  // Not a structured buffer
    inputBufferDesc.StructureByteStride = 0;  // Not a structured buffer
                                              //
    outputBufferDesc.ByteWidth = 16;
    outputBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    outputBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    outputBufferDesc.CPUAccessFlags = 0;  // No CPU access necessary
    outputBufferDesc.MiscFlags = 0;  // Not a structured buffer
    outputBufferDesc.StructureByteStride = 0;  // Not a structured buffer

    stagingBufferDesc.ByteWidth = 16;
    stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
    stagingBufferDesc.BindFlags = 0;  // Not binding to the pipeline, only for copy
    stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingBufferDesc.MiscFlags = 0;
    stagingBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initialInputData = {};
    initialInputData.SysMemPitch = 0;       // Not a texture
    initialInputData.SysMemSlicePitch = 0;  // Not a texture
    initialInputData.pSysMem = inputData;

    hr = device->CreateBuffer(&inputBufferDesc, &initialInputData, &inputBuffer);
    if (hr) {
        util::printWinErr(hr, "Failed to make input buffer");
        return 2;
    }

    hr = device->CreateBuffer(&outputBufferDesc, 0, &outputBuffer);
    if (hr) {
        util::printWinErr(hr, "Failed to make output buffer");
        return 2;
    }

    hr = device->CreateBuffer(&stagingBufferDesc, 0, &stagingBuffer);
    if (hr) {
        util::printWinErr(hr, "Failed to make staging buffer");
        return 2;
    }

	/*******************************************
	 *        Making the resource views        *
	 *******************************************/
    ID3D11ShaderResourceView *inputSRV;
    ID3D11UnorderedAccessView *outputUAV;

    D3D11_SHADER_RESOURCE_VIEW_DESC inputSRVDesc = {};
    D3D11_UNORDERED_ACCESS_VIEW_DESC outputUAVDesc = {};

    inputSRVDesc.Format = DXGI_FORMAT_R32_SINT;
    inputSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    inputSRVDesc.Buffer.FirstElement = 0;
    inputSRVDesc.Buffer.NumElements = 4;

    outputUAVDesc.Format = DXGI_FORMAT_R32_SINT;
    outputUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    outputUAVDesc.Buffer.FirstElement = 0;
    outputUAVDesc.Buffer.NumElements = 4;

    hr = device->CreateShaderResourceView(inputBuffer, &inputSRVDesc, &inputSRV);
    if (hr) {
        util::printWinErr(hr, "Failed to make input SRV");
        return 2;
    }

    hr = device->CreateUnorderedAccessView(outputBuffer, &outputUAVDesc, &outputUAV);
    if (hr) {
        util::printWinErr(hr, "Failed to make output UAV");
        return 2;
    }

    deviceCtx->CSSetShaderResources(0, 1, &inputSRV);
    deviceCtx->CSSetUnorderedAccessViews(0, 1, &outputUAV, 0);

    deviceCtx->CSSetShader(shader, NULL, 0);
    deviceCtx->Dispatch(1, 1, 1);
    deviceCtx->CSSetShader(NULL, NULL, 0);


    D3D11_MAPPED_SUBRESOURCE outputMappedData = {};
    deviceCtx->CopyResource(stagingBuffer, outputBuffer);
    hr = deviceCtx->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &outputMappedData);
    if (hr) {
        util::printWinErr(hr, "Failed to map to staging buffer");
        return 2;
    }
    memcpy(&inputData, outputMappedData.pData, 16);
    printf("%d %d %d %d\n", inputData[0], inputData[1], inputData[2], inputData[3]);
    deviceCtx->Unmap(stagingBuffer, 0);

    void *m_nptr = nullptr;
    deviceCtx->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)&m_nptr, 0);
    outputUAV->Release();

    deviceCtx->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&m_nptr);
    inputSRV->Release();

    inputBuffer->Release();
    outputBuffer->Release();

    shader->Release();

	device->Release();
	deviceCtx->Release();
}
