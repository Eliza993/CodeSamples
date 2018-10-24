#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace App1;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),

	m_indexCountSkybox(0),
	m_indexCountPyramid(0),
	m_indexCountModel(0),
	m_indexCountCherry(0),
	m_indexCountFloor(0),
	m_indexCountTerrain(0),
	m_indexCountCastle(0),
	m_indexCountKnight(0),
	m_indexCountRabbit(0),

	m_tracking(false),
	m_deviceResources(deviceResources)
{
	srand((int)time(0));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
	CreateRenderToTexture();

	
	static const XMVECTORF32 eye = { 0.0f, 0.0f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMStoreFloat4x4(&camera, XMMatrixLookAtRH(eye, at, up));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(0, XMMatrixLookAtRH(eye, at, up))));
}

void Sample3DSceneRenderer::CreateRenderToTexture()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Width = 512;
	texDesc.Height = 512;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &texRTT)
	);

	D3D11_TEXTURE2D_DESC texDesc2;
	texDesc2.Usage = D3D11_USAGE_DEFAULT;
	texDesc2.SampleDesc.Count = 1;
	texDesc2.SampleDesc.Quality = 0;
	texDesc2.Width = 512;	//may change later
	texDesc2.Height = 512;
	texDesc2.MipLevels = 1;
	texDesc2.ArraySize = 1;
	texDesc2.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc2.CPUAccessFlags = 0;
	texDesc2.Format = DXGI_FORMAT_D32_FLOAT;	
	texDesc2.MiscFlags = 0;
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateTexture2D(&texDesc2, NULL, &texDepthRTT)
	);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;	//will use same format
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;	//use mipmap level 0
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateRenderTargetView(texRTT, &rtvDesc, &rtvRTT)
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;	//same format
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateShaderResourceView(texRTT, &srvDesc, &srvRTT)
	);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = 0;
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateDepthStencilView(texDepthRTT, &dsvDesc, &dsvRTT)
	);


	//create viewport
	vpRTT.TopLeftX = 0;
	vpRTT.TopLeftY = 0;
	vpRTT.Width = 512;
	vpRTT.Height = 512;
	vpRTT.MinDepth = 0;
	vpRTT.MaxDepth = 1;

	//m_deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &vpRTT);

	//Bind render targets, depth-stencil buffer to the output-merger stage (OM)
	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(1, &rtvRTT, dsvRTT);

	targetPosRTT = XMMatrixTranslation(0.7f, 0, -1.0f);
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));


#pragma region Create Light
	//directional light, point light & spot light
	XMFLOAT4 lightPos[3] = { XMFLOAT4(0, 0, 0, 1.0), XMFLOAT4(3.0f, 0, 4.0, 1.0),XMFLOAT4(2, 0, 0, 1.0) };
	XMFLOAT4 lightDir[3] = { XMFLOAT4(-0.7, -0.6, 0.4, 1.0), XMFLOAT4(3.0f, -0.8, 2.0, 1.0),XMFLOAT4(2, -1, 1, 1.0) };
	XMFLOAT4 lightColor[3] = { XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 1, 0, 1),XMFLOAT4(1, 0, 0, 1) };

	cBuffer.lightPos[0] = lightPos[0];
	cBuffer.lightPos[1] = lightPos[1];
	cBuffer.lightPos[2] = lightPos[2];
	cBuffer.lightDir[0] = lightDir[0];
	cBuffer.lightDir[1] = lightDir[1];
	cBuffer.lightDir[2] = lightDir[2];
	cBuffer.lightColor[0] = lightColor[0];
	cBuffer.lightColor[1] = lightColor[1];
	cBuffer.lightColor[2] = lightColor[2];
#pragma endregion

}

using namespace Windows::UI::Core;
extern CoreWindow^ gwindow;
#include <atomic>
extern bool mouse_move;
extern float diffx;
extern float diffy;
extern bool w_down;
extern bool a_down;
extern bool s_down;
extern bool d_down;
extern bool left_click;

extern char buttons[256];

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		//float radians = static_cast<float>(fmod(totalRotation, XM_2PI));
		radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);

#pragma region Moving Lights
		XMVECTOR vDirectDir = XMLoadFloat4(&cBuffer.lightDir[0]);	//have to load float4 even though dir is float3 because vector is float4, w will be 0
		vDirectDir = XMVector3Transform(vDirectDir, XMMatrixRotationY(0.02f)); //this mul vector & matrix
		XMStoreFloat4(&cBuffer.lightDir[0], vDirectDir);			//put value back in
		
		XMVECTOR vPointPos = XMLoadFloat4(&cBuffer.lightPos[1]);
		vPointPos = XMVector3Transform(vPointPos, XMMatrixRotationY(0.02f));
		XMStoreFloat4(&cBuffer.lightPos[1], vPointPos);

		XMVECTOR vSpotDir = XMLoadFloat4(&cBuffer.lightDir[2]);
		XMVECTOR vSpotPos = XMLoadFloat4(&cBuffer.lightPos[2]);
		vSpotDir = XMVector3Transform(vSpotDir, XMMatrixRotationY(0.02f));
		vSpotPos = XMVector3Transform(vSpotPos, XMMatrixRotationY(0.02f));
		XMStoreFloat4(&cBuffer.lightDir[2], vSpotDir);
		XMStoreFloat4(&cBuffer.lightPos[2], vSpotPos);
#pragma endregion
	}

#pragma region Check Button for Camera	
	XMMATRIX newcamera = XMLoadFloat4x4(&camera);

	if (buttons['W'])
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[2] * -timer.GetElapsedSeconds() * 5.0;
	}

	if (a_down)
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[0] * -timer.GetElapsedSeconds() *5.0;
	}

	if (s_down)
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[2] * timer.GetElapsedSeconds() * 5.0;
	}

	if (d_down)
	{
		newcamera.r[3] = newcamera.r[3] + newcamera.r[0] * timer.GetElapsedSeconds() * 5.0;
	}

	Windows::UI::Input::PointerPoint^ point = nullptr;

	//if(mouse_move)/*This crashes unless a mouse event actually happened*/
	//point = Windows::UI::Input::PointerPoint::GetCurrentPoint(pointerID);

	if (mouse_move)
	{
		// Updates the application state once per frame.
		if (left_click)
		{
			XMVECTOR pos = newcamera.r[3];
			newcamera.r[3] = XMLoadFloat4(&XMFLOAT4(0, 0, 0, 1));
			newcamera = XMMatrixRotationX(-diffy*0.01f) * newcamera * XMMatrixRotationY(-diffx*0.01f);
			newcamera.r[3] = pos;
		}
	}

	XMStoreFloat4x4(&camera, newcamera);

	/*Be sure to inverse the camera & Transpose because they don't use pragma pack row major in shaders*/
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(0, newcamera)));

	mouse_move = false;/*Reset*/
#pragma endregion
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)*XMMatrixScaling(0.3, 0.3, 0.3)*XMMatrixTranslation(0.7, 0, 0)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	//draw solid
	context->RSSetState(rasterStateSolid);
	context->OMSetRenderTargets(1, &rtvRTT, dsvRTT);
	ClearColorAndDepthRTT(rtvRTT, dsvRTT);
	
	context->RSSetViewports(1, &vpRTT);

	context->UpdateSubresource1(
		m_constantBufferTexture.Get(),
		0,
		NULL,
		&cBuffer,
		0,
		0,
		0
	);

	context->UpdateSubresource1(
		m_constantBufferGeometric.Get(),
		0,
		NULL,
		&gCBuffer,
		0,
		0,
		0
	);

	// Each vertex is one instance of the Vertex truct.
	UINT stride = sizeof(VertexUvNormal);
	UINT offset = 0;

#pragma region Draw Skybox
	//move Skybox to camera's pos
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(camera._41, camera._42, camera._43)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferSkybox.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBufferSkybox.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	context->VSSetShader(
		m_vertexShaderSkybox.Get(),
		nullptr,
		0
	);

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShader(
		m_pixelShaderSkybox.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvEnvi);
	context->PSSetSamplers(0, 1, &samplerState);

	context->DrawIndexed(
		m_indexCountSkybox,
		0,
		0
	);

	//clear Z buffer after draw skybox so other object can be seen
	context->ClearDepthStencilView(dsvRTT, D3D11_CLEAR_DEPTH, 1.0f, 0);
#pragma endregion

	stride = sizeof(VertexPositionColor);
#pragma region Draw Pyramid
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)*XMMatrixScaling(0.3, 0.3, 0.3)*XMMatrixTranslation(0.7, 0, 0)));
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferPyramid.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBufferPyramid.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutPyramid.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderPyramid.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderPyramid.Get(),
		nullptr,
		0
	);

	// Draw the objects.

	context->DrawIndexed(
		m_indexCountPyramid,
		0,
		0
	);

#pragma endregion

//#pragma region Draw Stars
//	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixIdentity()));
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_constantBufferData,
//		0,
//		0,
//		0
//	);
//
//	//this is just a point
//	context->IASetVertexBuffers(
//		0,
//		1,
//		m_vertexBufferStar.GetAddressOf(),
//		&stride,
//		&offset
//	);
//
//	//context->IASetIndexBuffer(
//	//	m_indexBufferPyramid.Get(),
//	//	DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
//	//	0
//	//);
//
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
//
//	context->IASetInputLayout(m_inputLayoutPyramid.Get());
//
//	// Attach our vertex shader.
//	context->VSSetShader(
//		m_vertexShaderGeometry.Get(),
//		nullptr,
//		0
//	);
//
//	// Send the constant buffer to the graphics device.
//	context->VSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->GSSetShader(
//		m_geometryShaderStar.Get(),
//		nullptr,
//		0
//	);
//
//	ID3D11Buffer *gBuffer[2] = { m_constantBuffer.Get() , m_constantBufferGeometric.Get() };
//
//	context->GSSetConstantBuffers1(
//		0,
//		2,
//		gBuffer,
//		nullptr,
//		nullptr
//	);
//
//
//	//context->GSSetConstantBuffers1(
//	//	0,
//	//	2,
//	//	m_constantBuffer.GetAddressOf(),
//	//	nullptr,
//	//	nullptr
//	//);
//
//	// Attach our pixel shader.
//	context->PSSetShader(
//		m_pixelShaderPyramid.Get(),
//		nullptr,
//		0
//	);
//
//	// Draw the objects.
//	context->DrawInstanced(1, 40, 0, 0);
//
//	//remove Geometric Shader after use
//	ID3D11GeometryShader *gS = nullptr;
//	context->GSSetShader(
//		gS,
//		nullptr,
//		0
//	);
//#pragma endregion

	stride = sizeof(VertexUvNormal);
//#pragma region Draw Wolf
//	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
//	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 0, -2.0f)*XMMatrixRotationY(XMConvertToRadians(180.0f))));
//
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_constantBufferData,
//		0,
//		0,
//		0
//	);
//
//	context->IASetVertexBuffers(
//		0,
//		1,
//		m_vertexBufferModel.GetAddressOf(),
//		&stride,
//		&offset
//	);
//	context->IASetIndexBuffer(
//		m_indexBufferModel.Get(),
//		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
//		0
//	);
//
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	context->IASetInputLayout(m_inputLayoutModel.Get());
//
//	// Attach our vertex shader.
//	context->VSSetShader(
//		m_vertexShaderTexture.Get(),
//		nullptr,
//		0
//	);
//
//	// Send the constant buffer to the graphics device.
//	context->VSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	// Attach our pixel shader.
//	context->PSSetShader(
//		m_pixelShaderTexture.Get(),
//		nullptr,
//		0
//	);
//
//	context->PSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBufferTexture.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->PSSetShaderResources(0, 1, &srvWolf);
//	context->PSSetSamplers(0, 1, &samplerState);
//
//	// Draw the objects.
//	context->DrawIndexed(
//		m_indexCountModel,
//		0,
//		0
//	);
//#pragma endregion

#pragma region Draw Castle
	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 3.0f, -20.0f)*XMMatrixRotationY(XMConvertToRadians(180.0f))));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferCastle.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferCastle.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvBrick);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountCastle,
		0,
		0
	);
#pragma endregion

#pragma region Draw Knight
	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(20.0f, -2.0f, 0)*XMMatrixRotationY(XMConvertToRadians(180.0f))*XMMatrixRotationX(XMConvertToRadians(90.0f))));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferKnight.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferKnight.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvKnight);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountKnight,
		0,
		0
	);
#pragma endregion

#pragma region Draw Floor
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(2, 1, 5)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferFloor.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferFloor.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);



	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		1,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	//for texture
	context->PSSetShaderResources(0, 1, &srv);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountFloor,
		0,
		0
	);
#pragma endregion

	UINT insStride[2] = { sizeof(VertexUvNormal), sizeof(InstanceData) };
	UINT insOffset[2] = { 0, 0 };	
#pragma region Draw Cherry

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(0.1f, 0.1f, 0.1f)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);
	
	ID3D11Buffer *bufferCherry[] = { {m_vertexBufferCherry}, {m_instanceBufferCherry} };
	context->IASetVertexBuffers(
		0,
		2,
		bufferCherry,
		insStride,
		insOffset
	);

	context->IASetIndexBuffer(
		m_indexBufferCherry.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutInstance.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderIns.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderIns.Get(),
		nullptr,
		0
	);

	//context->PSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBufferTexture.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

	context->PSSetShaderResources(0, 1, &srvCherry);
	context->PSSetSamplers(0, 1, &samplerState);

	context->DrawIndexedInstanced(
		m_indexCountCherry,
		numCherry,
		0,
		0,
		0);
#pragma endregion

	ID3D11RenderTargetView* views[] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, views, m_deviceResources->GetDepthStencilView());
	ClearColorAndDepthRTT(views[0], m_deviceResources->GetDepthStencilView());

	context->RSSetViewports(1, &m_deviceResources->GetScreenViewport());

	//context->UpdateSubresource1(
	//	m_constantBufferTexture.Get(),
	//	0,
	//	NULL,
	//	&cBuffer,
	//	0,
	//	0,
	//	0
	//);
	//context->UpdateSubresource1(
	//	m_constantBufferGeometric.Get(),
	//	0,
	//	NULL,
	//	&gCBuffer,
	//	0,
	//	0,
	//	0
	//);

	stride = sizeof(VertexUvNormal);
	offset = 0;
#pragma region Draw Skybox
	//move Skybox to camera's pos
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(camera._41, camera._42, camera._43)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferSkybox.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBufferSkybox.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	context->VSSetShader(
		m_vertexShaderSkybox.Get(),
		nullptr,
		0
	);

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShader(
		m_pixelShaderSkybox.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvEnvi);
	context->PSSetSamplers(0, 1, &samplerState);

	context->DrawIndexed(
		m_indexCountSkybox,
		0,
		0
	);

	//clear Z buffer after draw skybox so other object can be seen
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
#pragma endregion

	stride = sizeof(VertexPositionColor);
#pragma region Draw Pyramid
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)*XMMatrixScaling(0.3, 0.3, 0.3)*XMMatrixTranslation(0.7, 0, 0)));
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferPyramid.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBufferPyramid.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutPyramid.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderPyramid.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderPyramid.Get(),
		nullptr,
		0
	);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountPyramid,
		0,
		0
	);

#pragma endregion

//#pragma region Draw Stars
//	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixIdentity()));
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_constantBufferData,
//		0,
//		0,
//		0
//	);
//
//	//this is just a point
//	context->IASetVertexBuffers(
//		0,
//		1,
//		m_vertexBufferStar.GetAddressOf(),
//		&stride,
//		&offset
//	);
//
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
//
//	context->IASetInputLayout(m_inputLayoutPyramid.Get());
//
//	// Attach our vertex shader.
//	context->VSSetShader(
//		m_vertexShaderGeometry.Get(),
//		nullptr,
//		0
//	);
//
//	// Send the constant buffer to the graphics device.
//	context->VSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->GSSetShader(
//		m_geometryShaderStar.Get(),
//		nullptr,
//		0
//	);
//
//	//NOTE: have to build clockwise if do Look at
//	XMVECTORF32 objectP = { 0, 2.0f, 0 };
//	XMVECTOR objectPos = objectP;
//	XMVECTORF32 cPos = { camera._41, camera._42, camera._43 };
//	XMVECTOR cameraPos = cPos;
//	XMVECTORF32 upV = {0, 1, 1};
//	XMVECTOR upVector = upV;
//	XMMATRIX objectLookAtCamera = XMMatrixLookAtLH(objectPos, cameraPos, upVector);
//	XMMatrixInverse(nullptr, objectLookAtCamera);
//
//	m_cBufferDataLookAt = m_constantBufferData;
//	//XMStoreFloat4x4(&m_cBufferDataLookAt.view, XMMatrixTranspose(objectLookAtCamera));
//	XMStoreFloat4x4(&m_cBufferDataLookAt.model, XMMatrixTranspose(objectLookAtCamera));
//
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_cBufferDataLookAt,
//		0,
//		0,
//		0
//	);
//
//	context->GSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	ID3D11Buffer *gBuffer[2] = { m_constantBuffer.Get() , m_constantBufferGeometric.Get() };
//
//	context->GSSetConstantBuffers1(
//		0,
//		2,
//		gBuffer,
//		nullptr,
//		nullptr
//	);
//
//
//	//context->GSSetConstantBuffers1(
//	//	0,
//	//	2,
//	//	m_constantBuffer.GetAddressOf(),
//	//	nullptr,
//	//	nullptr
//	//);
//
//	// Attach our pixel shader.
//	context->PSSetShader(
//		m_pixelShaderIns.Get(),
//		nullptr,
//		0
//	);
//	context->PSSetShaderResources(0, 1, &srvCloud[0]);
//	//context->PSSetShaderResources(0, 1, &srv);
//	context->PSSetSamplers(0, 1, &samplerState);
//
//	// Draw the objects.
//	context->DrawInstanced(1, 40, 0, 0);
//
//	//remove Geometric Shader after use
//	ID3D11GeometryShader *gS = nullptr;
//	context->GSSetShader(
//		gS,
//		nullptr,
//		0
//	);
//#pragma endregion

	stride = sizeof(VertexUvNormal);
//#pragma region Draw Wolf
//	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
//	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 0, -2.0f)*XMMatrixRotationY(XMConvertToRadians(180.0f))));
//
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_constantBufferData,
//		0,
//		0,
//		0
//	);
//
//	context->IASetVertexBuffers(
//		0,
//		1,
//		m_vertexBufferModel.GetAddressOf(),
//		&stride,
//		&offset
//	);
//	context->IASetIndexBuffer(
//		m_indexBufferModel.Get(),
//		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
//		0
//	);
//
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	context->IASetInputLayout(m_inputLayoutModel.Get());
//
//	// Attach our vertex shader.
//	context->VSSetShader(
//		m_vertexShaderTexture.Get(),
//		nullptr,
//		0
//	);
//
//	// Send the constant buffer to the graphics device.
//	context->VSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	// Attach our pixel shader.
//	context->PSSetShader(
//		m_pixelShaderTexture.Get(),
//		nullptr,
//		0
//	);
//
//	context->PSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBufferTexture.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->PSSetShaderResources(0, 1, &srvWolf);
//	context->PSSetSamplers(0, 1, &samplerState);
//
//	// Draw the objects.
//	context->DrawIndexed(
//		m_indexCountModel,
//		0,
//		0
//	);
//#pragma endregion


#pragma region Draw Castle
	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 3.0f, -20.0f)*XMMatrixRotationY(XMConvertToRadians(180.0f))));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferCastle.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferCastle.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvBrick);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountCastle,
		0,
		0
	);
#pragma endregion

#pragma region Draw Knight
	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(20.0f, -2.0f, 0)*XMMatrixRotationY(XMConvertToRadians(180.0f))*XMMatrixRotationX(XMConvertToRadians(90.0f))));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferKnight.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferKnight.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvKnight);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountKnight,
		0,
		0
	);
#pragma endregion

#pragma region Draw Rabbit
	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(20.0f, -2.0f, 0)*XMMatrixRotationY(XMConvertToRadians(180.0f))*XMMatrixScaling(0.02f, 0.02f, 0.02f)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferRabbit.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferRabbit.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvRabbit);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountRabbit,
		0,
		0
	);
#pragma endregion

#pragma region Draw Floor
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(2, 1, 5)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferFloor.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferFloor.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);



	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		1,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	//for texture
	context->PSSetShaderResources(0, 1, &srv);
	//context->PSSetShaderResources(0, 1, &srvRTT);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountFloor,
		0,
		0
	);
#pragma endregion

#pragma region Draw Cherry

	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(0.1f, 0.1f, 0.1f)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	//ID3D11Buffer *bufferCherry[] = { { m_vertexBufferCherry },{ m_instanceBufferCherry } };
	context->IASetVertexBuffers(
		0,
		2,
		bufferCherry,
		insStride,
		insOffset
	);

	context->IASetIndexBuffer(
		m_indexBufferCherry.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutInstance.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderIns.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderIns.Get(),
		nullptr,
		0
	);

	//context->PSSetConstantBuffers1(
	//	0,
	//	1,
	//	m_constantBufferTexture.GetAddressOf(),
	//	nullptr,
	//	nullptr
	//);

	context->PSSetShaderResources(0, 1, &srvCherry);
	context->PSSetSamplers(0, 1, &samplerState);

	context->DrawIndexedInstanced(
		m_indexCountCherry,
		numCherry,
		0,
		0,
		0);
#pragma endregion

#pragma region Draw Cube RTT
	//move Skybox to camera's pos
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(0.2f, 0.2f, 0.2f)));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferCube.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBufferCube.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShaderTexture.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderSolid.Get(),
		nullptr,
		0
	);

	context->PSSetShaderResources(0, 1, &srvRTT);
	context->PSSetSamplers(0, 1, &samplerState);

	//context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	//ClearColorAndDepthRTT(rtvRTT, dsvRTT);

	context->DrawIndexed(
		m_indexCountSkybox,
		0,
		0
	);

	ID3D11ShaderResourceView* pNullSRV = nullptr;
	context->PSSetShaderResources(0, 1, &pNullSRV);
	m_deviceResources->GetD3DDeviceContext()->GenerateMips(srvRTT);

#pragma endregion

	//switch to show effect of Tessellation
	context->RSSetState(rasterStateFrame);
//#pragma region Draw FrameWolf
//	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(2.0f, 0, -2.0f)*XMMatrixRotationY(XMConvertToRadians(180.0f))));
//
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_constantBufferData,
//		0,
//		0,
//		0
//	);
//
//	context->IASetVertexBuffers(
//		0,
//		1,
//		m_vertexBufferModel.GetAddressOf(),
//		&stride,
//		&offset
//	);
//	context->IASetIndexBuffer(
//		m_indexBufferModel.Get(),
//		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
//		0
//	);
//
//	//need to change this to Control Points
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
//
//	context->IASetInputLayout(m_inputLayoutModel.Get());
//
//	context->VSSetShader(
//		m_vertexShaderTess.Get(),
//		nullptr,
//		0
//	);
//
//	context->VSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->HSSetShader(m_hullShader.Get(), nullptr, 0);
//	context->DSSetShader(m_domainShader.Get(), nullptr, 0);
//
//	context->DSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	// Attach our pixel shader.
//	context->PSSetShader(
//		m_pixelShaderTexture.Get(),
//		nullptr,
//		0
//	);
//
//	context->PSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBufferTexture.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->PSSetShaderResources(0, 1, &srvWolf);
//	context->PSSetSamplers(0, 1, &samplerState);
//
//	// Draw the objects.
//	context->DrawIndexed(
//		m_indexCountModel,
//		0,
//		0
//	);
//
//	//set back after use to avoid error
//	ID3D11HullShader *hS = nullptr;
//	ID3D11DomainShader *dS = nullptr;
//	context->HSSetShader(hS, nullptr, 0);
//	context->DSSetShader(dS, nullptr, 0);
//#pragma endregion

#pragma region Draw Knight
	//NOTE: to make each model act differently, put new matrix in Model matrix and UpdateSubresource
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(60.0f, -2.0f, 0)*XMMatrixRotationY(XMConvertToRadians(180.0f))*XMMatrixRotationX(XMConvertToRadians(90.0f))));

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBufferKnight.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBufferKnight.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	//need to change this to Control Points
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	context->IASetInputLayout(m_inputLayoutModel.Get());

	context->VSSetShader(
		m_vertexShaderTess.Get(),
		nullptr,
		0
	);

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->HSSetShader(m_hullShader.Get(), nullptr, 0);
	context->DSSetShader(m_domainShader.Get(), nullptr, 0);

	context->DSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShaderTexture.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBufferTexture.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &srvKnight);
	context->PSSetSamplers(0, 1, &samplerState);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCountKnight,
		0,
		0
	);

	//set back after use to avoid error
	ID3D11HullShader *hS = nullptr;
	ID3D11DomainShader *dS = nullptr;
	context->HSSetShader(hS, nullptr, 0);
	context->DSSetShader(dS, nullptr, 0);

#pragma endregion

//#pragma region Draw Terrain
//	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixScaling(100.0f, 0, 100.0f)*XMMatrixTranslation(0, -1.0f, 0)));
//
//	context->UpdateSubresource1(
//		m_constantBuffer.Get(),
//		0,
//		NULL,
//		&m_constantBufferData,
//		0,
//		0,
//		0
//	);
//
//	context->IASetVertexBuffers(
//		0,
//		1,
//		m_vertexBufferTerrain.GetAddressOf(),
//		&stride,
//		&offset
//	);
//
//	
//	context->IASetIndexBuffer(
//		m_indexBufferTerrain.Get(),
//		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
//		0
//	);
//
//	//need to change this to Control Points
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
//
//	context->IASetInputLayout(m_inputLayoutModel.Get());
//
//	//context->VSSetShader(
//	//	m_vertexShaderTess.Get(),
//	//	nullptr,
//	//	0
//	//);
//
//	context->VSSetShader(
//		m_vertexShaderTerrain.Get(),
//		nullptr,
//		0
//	);
//
//	context->VSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	context->VSSetShaderResources(0, 1, &srvHM);
//	context->VSSetSamplers(0, 1, &samplerState);
//
//	//context->HSSetShader(m_hullShader.Get(), nullptr, 0);
//	context->HSSetShader(m_hullShaderTerrain.Get(), nullptr, 0);
//
//
//
//	context->DSSetShader(m_domainShader.Get(), nullptr, 0);
//
//	context->DSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBuffer.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	// Attach our pixel shader.
//	context->PSSetShader(
//		m_pixelShaderTexture.Get(),
//		nullptr,
//		0
//	);
//
//	context->PSSetConstantBuffers1(
//		0,
//		1,
//		m_constantBufferTexture.GetAddressOf(),
//		nullptr,
//		nullptr
//	);
//
//	
//	context->PSSetShaderResources(0, 1, &srvWolf);
//	context->PSSetSamplers(0, 1, &samplerState);
//
//	// Draw the objects.
//	context->DrawIndexed(
//		m_indexCountTerrain,
//		0,
//		0
//	);
//
//	//set back after use to avoid error
//	//ID3D11HullShader *hS = nullptr;
//	//ID3D11DomainShader *dS = nullptr;
//	context->HSSetShader(hS, nullptr, 0);
//	context->DSSetShader(dS, nullptr, 0);
//#pragma endregion
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	rasterDescFrame = CD3D11_RASTERIZER_DESC(D3D11_FILL_WIREFRAME, D3D11_CULL_BACK, FALSE, 0, 0.f, 0.f, TRUE, FALSE, TRUE, FALSE);
	rasterDescSolid = CD3D11_RASTERIZER_DESC(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, 0, 0.f, 0.f, TRUE, FALSE, TRUE, FALSE);
	result = m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterDescFrame, &rasterStateFrame);
	result = m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterDescSolid, &rasterStateSolid);

#pragma region Create Skybox
	auto loadVSTaskSkybox = DX::ReadDataAsync(L"SkyboxVertexShader.cso");
	auto loadPSTaskSkybox = DX::ReadDataAsync(L"SkyboxPixelShader.cso");

	// Load shaders asynchronously.
	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTaskSkybox = loadVSTaskSkybox.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderSkybox
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayoutModel
			)
		);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTaskSkybox = loadPSTaskSkybox.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderSkybox
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
	});

	// Once both shaders are loaded, create the mesh.
	auto createSkyboxTask = (createPSTaskSkybox && createVSTaskSkybox).then([this]() {

		static const VertexUvNormal cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, -1.0f, 0.0f) }, //top
			{ XMFLOAT3(-0.5f, 0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 1.0f, 0.0f) }, //bottom
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, 0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  -0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) }, //front
			{ XMFLOAT3(-0.5f, 0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f),  XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) }, //back
			{ XMFLOAT3(-0.5f, 0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, 0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) }, //left
			{ XMFLOAT3(-0.5f, 0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, 0.5f,  0.5f),  XMFLOAT2(0, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(-1.0f, 0.0f, 0.0f) }, //left
			{ XMFLOAT3(0.5f, 0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, 0.5f,  0.5f),  XMFLOAT2(0, 0), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferSkybox
			)
		);

		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\SkyBox.DDS", nullptr, &srvEnvi);

		D3D11_SAMPLER_DESC sampDes;
		ZeroMemory(&sampDes, sizeof(sampDes));
		sampDes.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDes.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDes.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDes.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		//sampDes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		//sampDes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		//sampDes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDes.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDes.MinLOD = 0;
		sampDes.MaxLOD = D3D11_FLOAT32_MAX; //no max

		result = m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDes, &samplerState);

		//anti-clockwise
		static const unsigned short cubeIndices[] =
		{		
			0,1,2,
			0,2,3,
			4,6,5,
			4,7,6,
			8,9, 10,
			8,10,11,
			12,14,13,
			12,15,14,
			16,18,17,
			16,19,18,
			20,21,22,
			20,22,23,
		};
		m_indexCountSkybox = ARRAYSIZE(cubeIndices);
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferSkybox
			)
		);

		static const unsigned short cubeIndices2[] =
		{
			0, 2, 1,
			0, 3, 2,
			4, 5, 6,
			4, 6, 7,
			8, 10,9,
			8, 11,10,
			12,13,14,
			12,14,15,
			16,17,18,
			16,18,19,
			20,22,21,
			20,23,22,
		};
		m_indexCountSkybox = ARRAYSIZE(cubeIndices2);
		D3D11_SUBRESOURCE_DATA indexBufferData2 = { 0 };
		indexBufferData2.pSysMem = cubeIndices2;
		indexBufferData2.SysMemPitch = 0;
		indexBufferData2.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc2(sizeof(cubeIndices2), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc2,
				&indexBufferData2,
				&m_indexBufferCube
			)
		);
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createSkyboxTask.then([this]() {
		m_loadingComplete = true;
	});

#pragma endregion

	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	
	auto loadVSTaskGeometry = DX::ReadDataAsync(L"VertexShaderForGeometric.cso");
	auto loadGSTask = DX::ReadDataAsync(L"GeometryShader.cso");
#pragma region Create Pyramid, Stars

	//TODO: 1 - Draw Pyramid - V
	// Load shaders asynchronously.

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderPyramid
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayoutPyramid
				)
			);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderPyramid
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});

	// Once both shaders are loaded, create the mesh.
	auto createPyramidTask = (createPSTask && createVSTask).then([this] () {

		static const VertexPositionColor triVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3( 0.5f, -0.5f,  -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3( 0.0f,  -0.5f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3( 0.0f,  0.5f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		};


		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = triVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(triVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferPyramid
				)
			);

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.

		//m_indexCount = ARRAYSIZE(cubeIndices);
		//D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		//indexBufferData.pSysMem = cubeIndices;
		//indexBufferData.SysMemPitch = 0;
		//indexBufferData.SysMemSlicePitch = 0;
		//CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		//DX::ThrowIfFailed(
		//	m_deviceResources->GetD3DDevice()->CreateBuffer(
		//		&indexBufferDesc,
		//		&indexBufferData,
		//		&m_indexBuffer
		//		)
		//	);

		//make sure clockwise or anticlockwise is consistent, depend on the set up
		//need to put anti-clockwise in this case
		static const unsigned short triIndices[] =
		{
			0,1,2,
			0,1,3,
			1,2,3,
			2,0,3,
		};

		m_indexCountPyramid = ARRAYSIZE(triIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = triIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(triIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferPyramid
			)
		);

	});

	// Once the cube is loaded, the object is ready to be rendered.
	createPyramidTask.then([this]() {
		m_loadingComplete = true;
	});

	auto createVSTaskGeometry = loadVSTaskGeometry.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderGeometry
			)
		);
	});

	auto createGeometryTask = loadGSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_geometryShaderStar
			)
		);

		//CD3D11_BUFFER_DESC constantBufferGeoDesc(sizeof(GeometryConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		CD3D11_BUFFER_DESC constantBufferGeoDesc(sizeof(XMFLOAT4)*numGeoIns, D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferGeoDesc,
				nullptr,
				&m_constantBufferGeometric
			)
		);
	});

	auto createStarTask = (createPSTask && createVSTaskGeometry && createGeometryTask).then([this]() {

		static const VertexPositionColor starsVertices[] =
		{
			{ XMFLOAT3(0, 2.0f, 0), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};


		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = starsVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(starsVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferStar
			)
		);

		//pos for Stars
		const int numIns = 40;
		//XMFLOAT4 *arrPos = new XMFLOAT4[numIns];
		float x, y, z, w;
		w = 1.0f;
		int maxDis = 100;
		for (int i = 0; i < numIns; i++)
		{
			x = rand() % maxDis - maxDis/2;
			y = rand() % (maxDis/5);
			z = rand() % maxDis - maxDis / 2;
			gCBuffer.pos[i] = {x, y, z, w};
		}

#pragma region Clouds texture
		ID3D11ShaderResourceView *texture1, *texture2, *texture3, *texture4, *texture5, *texture6;
		//result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\c1.DDS", nullptr, &texture1);
		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\cloud2.DDS", nullptr, &texture1);
		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\c2.DDS", nullptr, &texture2);
		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\c3.DDS", nullptr, &texture3);
		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\c4.DDS", nullptr, &texture4);
		texture5 = texture3;
		texture6 = texture2;
		srvCloud.push_back(texture1);
		srvCloud.push_back(texture2);
		srvCloud.push_back(texture3);
		srvCloud.push_back(texture4);
		srvCloud.push_back(texture5);
		srvCloud.push_back(texture6);
#pragma endregion
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createStarTask.then([this]() {
		m_loadingComplete = true;
	});

#pragma endregion

	auto loadVSTask2 = DX::ReadDataAsync(L"PosUvNVertexShader.cso");
	auto loadPSTask2 = DX::ReadDataAsync(L"PosUvNPixelShader.cso");

#pragma region Create Model

	//TODO: 2 - Draw Model - V

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask2 = loadVSTask2.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderTexture
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
			//m_deviceResourcesModel->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayoutModel
			)
		);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask2 = loadPSTask2.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
			//m_deviceResourcesModel->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderTexture
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
			//m_deviceResourcesModel->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);

		//for texture
		CD3D11_BUFFER_DESC constantBufferTextureDesc(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				//m_deviceResourcesModel->GetD3DDevice()->CreateBuffer(
				&constantBufferTextureDesc,
				nullptr,
				&m_constantBufferTexture
			)
		);
	});

	// Once both shaders are loaded, create the mesh.
	auto createModelTask = (createPSTask2 && createVSTask2).then([this]() {

		// Load mesh vertices. Each vertex has a position and a color.
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT2> uvs;
		vector<XMFLOAT3> normals;

		char *path = "Assets\\wolf2.obj";
		bool resultLoad = loadOBJ(path, vertices, uvs, normals);

		vector<VertexUvNormal> modelVertices;
		vector<unsigned short> modelIndices;

		GetUniqueVertexAndIndices(vertices, uvs, normals, modelVertices, modelIndices);

		//TODO: check if need this or not
		IndexSwitchClockWiseAntiClockWise(modelIndices);

		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\wolfB.DDS", nullptr, &srvWolf);
		//result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\SkyBox.DDS", nullptr, (ID3D11Resource**)&enviTexture, &srvEnvi);		
		//result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\SkyBox.DDS", nullptr, &srvEnvi);
		//D3D11_SAMPLER_DESC sampDes;
		//ZeroMemory(&sampDes, sizeof(sampDes));
		//sampDes.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//sampDes.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		//sampDes.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		//sampDes.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		////sampDes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		////sampDes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		////sampDes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		//sampDes.ComparisonFunc = D3D11_COMPARISON_NEVER;
		//sampDes.MinLOD = 0;
		//sampDes.MaxLOD = D3D11_FLOAT32_MAX; //no max
		//result = m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDes, &samplerState);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = modelVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexUvNormal)*modelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferModel
			)
		);

		m_indexCountModel = modelIndices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = modelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * modelIndices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferModel
			)
		);

	});

	createModelTask.then([this]() {
		m_loadingComplete = true;
	});


	auto createCastleTask = (createPSTask2 && createVSTask2).then([this]() {

		// Load mesh vertices. Each vertex has a position and a color.
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT2> uvs;
		vector<XMFLOAT3> normals;

		char *path = "Assets\\Castle.obj";
		bool resultLoad = loadOBJ(path, vertices, uvs, normals);

		vector<VertexUvNormal> modelVertices;
		vector<unsigned short> modelIndices;

		GetUniqueVertexAndIndices(vertices, uvs, normals, modelVertices, modelIndices);

		IndexSwitchClockWiseAntiClockWise(modelIndices);

		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\Brick.DDS", nullptr, &srvBrick);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = modelVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexUvNormal)*modelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferCastle
			)
		);

		m_indexCountCastle = modelIndices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = modelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * modelIndices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferCastle
			)
		);

	});

	createCastleTask.then([this]() {
		m_loadingComplete = true;
	});

	auto createKnightTask = (createPSTask2 && createVSTask2).then([this]() {

		// Load mesh vertices. Each vertex has a position and a color.
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT2> uvs;
		vector<XMFLOAT3> normals;

		char *path = "Assets\\Knight.obj";
		bool resultLoad = loadOBJ(path, vertices, uvs, normals);

		vector<VertexUvNormal> modelVertices;
		vector<unsigned short> modelIndices;

		GetUniqueVertexAndIndices(vertices, uvs, normals, modelVertices, modelIndices);

		IndexSwitchClockWiseAntiClockWise(modelIndices);

		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\Knight.DDS", nullptr, &srvKnight);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = modelVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexUvNormal)*modelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferKnight
			)
		);

		m_indexCountKnight = modelIndices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = modelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * modelIndices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferKnight
			)
		);

	});

	createKnightTask.then([this]() {
		m_loadingComplete = true;
	});

	auto createRabbitTask = (createPSTask2 && createVSTask2).then([this]() {

		// Load mesh vertices. Each vertex has a position and a color.
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT2> uvs;
		vector<XMFLOAT3> normals;

		char *path = "Assets\\Rabbit.obj";
		bool resultLoad = loadOBJ(path, vertices, uvs, normals);

		vector<VertexUvNormal> modelVertices;
		vector<unsigned short> modelIndices;

		GetUniqueVertexAndIndices(vertices, uvs, normals, modelVertices, modelIndices);

		IndexSwitchClockWiseAntiClockWise(modelIndices);

		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\Rabbit.DDS", nullptr, &srvRabbit);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = modelVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexUvNormal)*modelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferRabbit
			)
		);

		m_indexCountRabbit = modelIndices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = modelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * modelIndices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferRabbit
			)
		);

	});

	createRabbitTask.then([this]() {
		m_loadingComplete = true;
	});

	// Create Floor
	auto createFloorTask = (createPSTask2 && createVSTask2).then([this]() {

		static const VertexUvNormal floorVertices[] =
		{

			{ XMFLOAT3(-2.0f, -0.8f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3 (0, 1, 0) },
			{ XMFLOAT3(2.0f, -0.8f,  -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3 (0, 1, 0) },
			{ XMFLOAT3(-2.0f, -0.8f,  1.0f),  XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0, 1, 0)},
			{ XMFLOAT3(2.0f,  -0.8f,  1.0f),  XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0, 1, 0)},
		};

#pragma region Load Texture
		//TODO: 3 - Load texure data, should do this after load mesh - V
		
		//Way 1: use D3D11 DDSTextureLoader.h
		//CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"rose.DDS", nullptr, &srv);
		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\floor.DDS", nullptr, &srv);
		
		//D3D11_SAMPLER_DESC sampDes;
		//ZeroMemory(&sampDes, sizeof(sampDes));
		//sampDes.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//sampDes.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		//sampDes.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		//sampDes.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		////sampDes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		////sampDes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		////sampDes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		//sampDes.ComparisonFunc = D3D11_COMPARISON_NEVER;
		//sampDes.MinLOD = 0;
		//sampDes.MaxLOD = D3D11_FLOAT32_MAX; //no max

		//result = m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDes, &samplerState);

		////Way 2: 
		//D3D11_TEXTURE2D_DESC texDesc;
		//D3D11_SUBRESOURCE_DATA texSource[Celestial_numlevels]; //for each mipmap
		//ZeroMemory(&texDesc, sizeof(texDesc));
		//for (unsigned int i = 0; i < Celestial_numlevels; i++)
		//{
		//	ZeroMemory(&texSource[i], sizeof(texSource[i]));
		//	texSource[i].pSysMem = &Celestial_pixels[Celestial_leveloffsets[i]];
		//	texSource[i].SysMemPitch = (Celestial_width >> i) * sizeof(unsigned int);
		//}
		//texDesc.ArraySize = 1;	//only load 1 texture
		//texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //will read this value into a shader
		////the format is in BIG ENDIAN
		//texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; //this is same format as .h texture file
		//texDesc.Height = Celestial_height;
		//texDesc.Width = Celestial_width;
		//texDesc.MipLevels = Celestial_numlevels;
		//texDesc.Usage = D3D11_USAGE_IMMUTABLE; //we wont change the texture
		//texDesc.SampleDesc.Count = 1;
		//result = m_deviceResources->GetD3DDevice()->CreateTexture2D(&texDesc, texSource, &diffuseTexture);

#pragma endregion


		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = floorVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(floorVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferFloor
			)
		);

		//need to put anti-clockwise
		static const unsigned short floorIndices[] =
		{
			0,1,3,
			0,3,2,
		};

		m_indexCountFloor = ARRAYSIZE(floorIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = floorIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(floorIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferFloor
			)
		);

	});

	createFloorTask.then([this]() {
		m_loadingComplete = true;
	});


#pragma endregion

	auto loadPSSolidTask = DX::ReadDataAsync(L"SolidPixelShader.cso");
	auto createPSSolidTask = loadPSSolidTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderSolid
			)
		);
	});

	auto createCubeTask = (createVSTask2 && createPSSolidTask).then([this]() {

		static const VertexUvNormal cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, 0.5f, -0.5f),  XMFLOAT2(0, 1), XMFLOAT3(0.0f,1.0f, 0.0f) }, //top
			{ XMFLOAT3(-0.5f, 0.5f,  0.5f),  XMFLOAT2(0, 0), XMFLOAT3(0.0f,1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f, 0.5f),   XMFLOAT2(1, 0), XMFLOAT3(0.0f,1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  -0.5f), XMFLOAT2(1, 1), XMFLOAT3(0.0f,1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 1), XMFLOAT3(0.0f, -1.0f, 0.0f) }, //bottom
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, 0.5f),  XMFLOAT2(1, 0), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, -0.5f), XMFLOAT2(1, 1), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 1), XMFLOAT3(0.0f, 0.0f, -1.0f) }, //front
			{ XMFLOAT3(-0.5f, 0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f),  XMFLOAT2(1, 0), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, -0.5f), XMFLOAT2(1, 1), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, 0.5f),  XMFLOAT2(0, 1), XMFLOAT3(0.0f, 0.0f, 1.0f) }, //back
			{ XMFLOAT3(-0.5f, 0.5f,  0.5f),  XMFLOAT2(0, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f),  XMFLOAT2(1, 0), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  -0.5f, 0.5f),  XMFLOAT2(1, 1), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 1), XMFLOAT3(-1.0f, 0.0f, 0.0f) }, //left
			{ XMFLOAT3(-0.5f, 0.5f,  -0.5f), XMFLOAT2(0, 0), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, 0.5f,  0.5f),  XMFLOAT2(1, 0), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT2(1, 1), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f),  XMFLOAT2(0, 1), XMFLOAT3(1.0f, 0.0f, 0.0f) }, //left
			{ XMFLOAT3(0.5f, 0.5f,  -0.5f),  XMFLOAT2(0, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, 0.5f,  0.5f),   XMFLOAT2(1, 0), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f),  XMFLOAT2(1, 1), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferCube
			)
		);

		//anti-clockwise
		static const unsigned short cubeIndices2[] =
		{
			0, 2, 1,
			0, 3, 2,
			4, 5, 6,
			4, 6, 7,
			8, 10,9,
			8, 11,10,
			12,13,14,
			12,14,15,
			16,17,18,
			16,18,19,
			20,22,21,
			20,23,22,
		};
		m_indexCountSkybox = ARRAYSIZE(cubeIndices2);
		D3D11_SUBRESOURCE_DATA indexBufferData2 = { 0 };
		indexBufferData2.pSysMem = cubeIndices2;
		indexBufferData2.SysMemPitch = 0;
		indexBufferData2.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc2(sizeof(cubeIndices2), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc2,
				&indexBufferData2,
				&m_indexBufferCube
			)
		);
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]() {
		m_loadingComplete = true;
	});

#pragma region Create Instancing
	auto loadVSInsTask = DX::ReadDataAsync(L"InstanceVertexShader.cso");
	auto loadPSInsTask = DX::ReadDataAsync(L"InstancePixelShader.cso");

	auto createVSInsTask = loadVSInsTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderIns
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "POSITION_INSTANCE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	//this is for instance
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayoutInstance
			)
		);
	});

	auto createPSInsTask = loadPSInsTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderIns
			)
		);
	});


	auto createCherryTask = (createPSInsTask && createVSInsTask).then([this]() {

		vector<XMFLOAT3> vertices;
		vector<XMFLOAT2> uvs;
		vector<XMFLOAT3> normals;

		//char *path = "Assets\\Cherry2.obj";
		char *path = "Assets\\MapleTree.obj";
		bool resultLoad = loadOBJ(path, vertices, uvs, normals);

		vector<VertexUvNormal> modelVertices;
		vector<unsigned short> modelIndices;

		GetUniqueVertexAndIndices(vertices, uvs, normals, modelVertices, modelIndices);

		IndexSwitchClockWiseAntiClockWise(modelIndices);

		//result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\CherryTree.DDS", nullptr, &srvCherry);
		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\MapleTree.DDS", nullptr, &srvCherry);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = modelVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexUvNormal)*modelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferCherry
			)
		);

		m_indexCountCherry = modelIndices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = modelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * modelIndices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferCherry
			)
		);

		//Create Instance Buffer
		InstanceData *insData = new InstanceData[numCherry];

		float x = 0.0f;
		float y = -2.0f;
		float z = 0.0f;
		int maxDistance = 200;
		for (int i = 0; i < numCherry; i++)
		{
			x = rand() % maxDistance;
			z = rand() % maxDistance;
			insData[i].pos = { x, y, z };
		}

		D3D11_SUBRESOURCE_DATA insBufferData = { 0 };
		insBufferData.pSysMem = insData;
		insBufferData.SysMemPitch = 0;
		insBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC insBufferDesc(sizeof(InstanceData)*numCherry, D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&insBufferDesc,
				&insBufferData,
				&m_instanceBufferCherry
			)
		);
	});

	createCherryTask.then([this]() {
		m_loadingComplete = true;
	});
#pragma endregion

	auto loadVSTessTask = DX::ReadDataAsync(L"TessellationVertexShader.cso");
	auto loadVSTerrainTask = DX::ReadDataAsync(L"TerrainVertexShader.cso");
	auto loadHSTask = DX::ReadDataAsync(L"HullShader.cso");
	auto loadHSTerrainTask = DX::ReadDataAsync(L"TerrainHullShader.cso");
	auto loadDSTask = DX::ReadDataAsync(L"DomainShader.cso");

	auto createVSTerrainTask = loadVSTerrainTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderTerrain
			)
		);
	});
	auto createVSTessTask = loadVSTessTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderTess
			)
		);
	});
	auto createHSTask = loadHSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateHullShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_hullShader
			)
		);
	});
	auto createHSTerrainTask = loadHSTerrainTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateHullShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_hullShaderTerrain
			)
		);
	});
	auto createDSTask = loadDSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDomainShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_domainShader
			)
		);
	});

//#pragma region Create Flat Terrain
//	auto createTerrainTask = (createPSTask2 && createVSTerrainTask && createHSTerrainTask && createDSTask).then([this]() {
//
//		//CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC();
//
//		D3D11_TEXTURE2D_DESC texDesc;
//		texDesc.Usage = D3D11_USAGE_DEFAULT;
//		texDesc.SampleDesc.Count = 1;
//		texDesc.SampleDesc.Quality = 0;
//		texDesc.Width = 512;
//		texDesc.Height = 512;
//		texDesc.MipLevels = 1;
//		texDesc.ArraySize = 1;
//		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//		texDesc.MiscFlags = 0;
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &texHM)
//		);
//
//		//result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\heightMap.DDS", nullptr, &srvHM);
//		result = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\heightMap.DDS", (ID3D11Resource**)texHM, &srvHM);
//
//		vector<XMFLOAT3> vertices;
//		vector<XMFLOAT2> uvs;
//		vector<XMFLOAT3> normals;
//
//		char *path = "Assets\\FlatTerrain.obj";
//		bool resultLoad = loadOBJ(path, vertices, uvs, normals);
//
//		vector<VertexUvNormal> modelVertices;
//		vector<unsigned short> modelIndices;
//
//		GetUniqueVertexAndIndices(vertices, uvs, normals, modelVertices, modelIndices);
//
//		IndexSwitchClockWiseAntiClockWise(modelIndices);
//
//		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
//		vertexBufferData.pSysMem = modelVertices.data();
//		vertexBufferData.SysMemPitch = 0;
//		vertexBufferData.SysMemSlicePitch = 0;
//		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexUvNormal)*modelVertices.size(), D3D11_BIND_VERTEX_BUFFER);
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateBuffer(
//				&vertexBufferDesc,
//				&vertexBufferData,
//				&m_vertexBufferTerrain
//			)
//		);
//
//		m_indexCountTerrain = modelIndices.size();
//
//		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
//		indexBufferData.pSysMem = modelIndices.data();
//		indexBufferData.SysMemPitch = 0;
//		indexBufferData.SysMemSlicePitch = 0;
//		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * modelIndices.size(), D3D11_BIND_INDEX_BUFFER);
//		DX::ThrowIfFailed(
//			m_deviceResources->GetD3DDevice()->CreateBuffer(
//				&indexBufferDesc,
//				&indexBufferData,
//				&m_indexBufferTerrain
//			)
//		);
//
//		//int width = 512;
//		//const int numVertices = width * width;
//		//const int numIndices = numVertices * 3;
//		//VertexUvNormal *floorVertices = new VertexUvNormal[numVertices];
//
//		//int count = 0;
//		//for (int i = 0; i < width; i++)
//		//{
//		//	for (int j = 0; j < width; j++)
//		//	{
//		//		floorVertices[count] = VertexUvNormal{ XMFLOAT3(i, 0, j), XMFLOAT2(0, 0), XMFLOAT3(0, 1, 0) };
//		//		count++;
//		//	}
//		//}
//
//		////static const VertexUvNormal floorVertices[] = vertices;
//
//		//D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
//		//vertexBufferData.pSysMem = floorVertices;
//		//vertexBufferData.SysMemPitch = 0;
//		//vertexBufferData.SysMemSlicePitch = 0;
//		//CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(floorVertices), D3D11_BIND_VERTEX_BUFFER);
//		//DX::ThrowIfFailed(
//		//	m_deviceResources->GetD3DDevice()->CreateBuffer(
//		//		&vertexBufferDesc,
//		//		&vertexBufferData,
//		//		&m_vertexBufferTerrain
//		//	)
//		//);
//
//		//unsigned short *floorIndices = new unsigned short[numIndices];
//
//		//count = 0;
//		//for (int i = 0; i < width/2; i++)	//row
//		//{
//		//	for (int j = 0; j < width/2; j++) //column
//		//	{
//		//		floorIndices[count] = 
//		//		count++;
//		//	}
//		//}
//
//		//static const unsigned short floorIndices[] =
//		//{
//		//	0,1,3,
//		//	0,3,2,
//		//};
//
//		////m_indexCountFloor = ARRAYSIZE(floorIndices);
//		//m_indexCountFloor = numIndices;
//
//		//D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
//		//indexBufferData.pSysMem = floorIndices;
//		//indexBufferData.SysMemPitch = 0;
//		//indexBufferData.SysMemSlicePitch = 0;
//		//CD3D11_BUFFER_DESC indexBufferDesc(sizeof(floorIndices), D3D11_BIND_INDEX_BUFFER);
//		//DX::ThrowIfFailed(
//		//	m_deviceResources->GetD3DDevice()->CreateBuffer(
//		//		&indexBufferDesc,
//		//		&indexBufferData,
//		//		&m_indexBufferTerrain
//		//	)
//		//);
//	});
//	createTerrainTask.then([this]() {
//		m_loadingComplete = true;
//	});
//#pragma endregion

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_constantBuffer.Reset();
	m_constantBufferTexture.Reset();
	m_constantBufferGeometric.Reset();

	m_inputLayoutPyramid.Reset();
	m_inputLayoutModel.Reset();
	m_inputLayoutInstance.Reset();

	m_vertexShaderSkybox.Reset();
	m_pixelShaderSkybox.Reset();
	m_vertexShaderPyramid.Reset();
	m_pixelShaderPyramid.Reset();
	m_vertexShaderTexture.Reset();
	m_pixelShaderTexture.Reset();

	m_vertexShaderGeometry.Reset();
	m_geometryShaderStar.Reset();
	m_vertexShaderIns.Reset();
	m_pixelShaderIns.Reset();

	m_pixelShaderSolid.Reset();

	m_vertexShaderTess.Reset();
	m_vertexShaderTerrain.Reset();
	m_hullShader.Reset();
	m_hullShaderTerrain.Reset();
	m_domainShader.Reset();

	m_vertexBufferSkybox.Reset();
	m_indexBufferSkybox.Reset();
	m_vertexBufferPyramid.Reset();
	m_indexBufferPyramid.Reset();
	m_vertexBufferModel.Reset();
	m_indexBufferModel.Reset();
	m_vertexBufferFloor.Reset();
	m_indexBufferFloor.Reset();
	m_vertexBufferCastle.Reset();
	m_indexBufferCastle.Reset();
	m_vertexBufferKnight.Reset();
	m_indexBufferKnight.Reset();
	m_vertexBufferRabbit.Reset();
	m_indexBufferRabbit.Reset();

	m_indexBufferCherry.Reset();
	m_vertexBufferStar.Reset();
	m_indexBufferTerrain.Reset();
	m_vertexBufferTerrain.Reset();

	m_vertexBufferCherry->Release();
	m_instanceBufferCherry->Release();

	texRTT->Release();
	texDepthRTT->Release();
	texHM->Release();

	srvEnvi->Release();
	srv->Release();
	srvBrick->Release();
	srvKnight->Release();
	srvRabbit->Release();
	srvWolf->Release();
	srvCherry->Release();
	srvRTT->Release();
	srvHM->Release();

	for (int i = 0; i < srvCloud.size(); i++)
	{
		srvCloud[i]->Release();
	}
	samplerState->Release();

	rtv->Release();
	dsv->Release();
	rtvRTT->Release();
	dsvRTT->Release();
}

void App1::Sample3DSceneRenderer::ClearColorAndDepthRTT(ID3D11RenderTargetView * rtv, ID3D11DepthStencilView * dsv)
{
	FLOAT color = (0, 0, 0, 1);
	m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(rtv, &color);
	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
}
