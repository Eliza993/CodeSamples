#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

#include <DirectXMath.h>
#include <vector>
#include "LoadObj.h"
#include "DDSTextureLoader.h" 

using namespace std;
using namespace Microsoft::WRL;	//ComPtr in here
using namespace DirectX;

namespace App1
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateRenderToTexture();
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		//void ClearColorAndDepthRTT(ID3D11RenderTargetView* rtv);//, ID3D11DepthStencilView* dsv);
		void ClearColorAndDepthRTT(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


	private:
		void Rotate(float radians);

	private:
		HRESULT result; //use to check error 

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		//std::shared_ptr<DX::DeviceResources> m_deviceResourcesModel;

		// Direct3D resources for cube geometry.
		//need 2 because there are 2 diff layout
		ComPtr<ID3D11InputLayout>	m_inputLayoutPyramid;
		ComPtr<ID3D11InputLayout>	m_inputLayoutModel;
		ComPtr<ID3D11InputLayout>	m_inputLayoutInstance;

		ComPtr<ID3D11Buffer>		m_constantBuffer;
		ComPtr<ID3D11Buffer>		m_constantBufferTexture;
		ComPtr<ID3D11Buffer>		m_constantBufferGeometric;

		//need 1 for each model
		ComPtr<ID3D11Buffer>		m_indexBufferSkybox;
		ComPtr<ID3D11Buffer>		m_indexBufferCube;
		ComPtr<ID3D11Buffer>		m_indexBufferPyramid;
		ComPtr<ID3D11Buffer>		m_indexBufferModel;
		ComPtr<ID3D11Buffer>		m_indexBufferCherry;
		ComPtr<ID3D11Buffer>		m_indexBufferFloor;
		ComPtr<ID3D11Buffer>		m_indexBufferTerrain;
		ComPtr<ID3D11Buffer>		m_indexBufferCastle;
		ComPtr<ID3D11Buffer>		m_indexBufferKnight;
		ComPtr<ID3D11Buffer>		m_indexBufferRabbit;

		ComPtr<ID3D11Buffer>		m_vertexBufferSkybox;
		ComPtr<ID3D11Buffer>		m_vertexBufferCube;
		ComPtr<ID3D11Buffer>		m_vertexBufferPyramid;
		ComPtr<ID3D11Buffer>		m_vertexBufferModel;
		ComPtr<ID3D11Buffer>		m_vertexBufferFloor;
		ComPtr<ID3D11Buffer>		m_vertexBufferCastle;
		ComPtr<ID3D11Buffer>		m_vertexBufferKnight;
		ComPtr<ID3D11Buffer>		m_vertexBufferRabbit;
		
		ComPtr<ID3D11Buffer>		m_vertexBufferStar;

		ComPtr<ID3D11Buffer>		m_vertexBufferTerrain;
		
		//ComPtr<ID3D11Buffer>		m_vertexBufferCherry;
		//ComPtr<ID3D11Buffer>		m_instanceBufferCherry;
		ID3D11Buffer*		m_vertexBufferCherry;
		ID3D11Buffer*		m_instanceBufferCherry;
		const uint32 numCherry = 10;
		const uint32 numCloud = 40;

		//there are 2 shaders, 1 for color, 1 for texture
		ComPtr<ID3D11VertexShader>	m_vertexShaderSkybox;
		ComPtr<ID3D11PixelShader>	m_pixelShaderSkybox;

		ComPtr<ID3D11VertexShader>	m_vertexShaderPyramid;
		ComPtr<ID3D11PixelShader>	m_pixelShaderPyramid;

		ComPtr<ID3D11PixelShader>	m_pixelShaderTexture;
		ComPtr<ID3D11VertexShader>	m_vertexShaderTexture;

		ComPtr<ID3D11VertexShader>	m_vertexShaderGeometry;
		ComPtr<ID3D11GeometryShader> m_geometryShaderStar;

		ComPtr<ID3D11VertexShader>	m_vertexShaderIns;
		ComPtr<ID3D11PixelShader>	m_pixelShaderIns;

		ComPtr<ID3D11PixelShader>	m_pixelShaderSolid;

		ComPtr<ID3D11VertexShader>	m_vertexShaderTess;
		ComPtr<ID3D11VertexShader>	m_vertexShaderTerrain;
		ComPtr<ID3D11HullShader> m_hullShader;
		ComPtr<ID3D11HullShader> m_hullShaderTerrain;
		ComPtr<ID3D11DomainShader> m_domainShader;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		ModelViewProjectionConstantBuffer	m_cBufferDataLookAt;	//this will always face he camera

		//InstanceConstantBuffer insConstantBuffer;
		ConstantBuffer cBuffer;
		GeometryConstantBuffer gCBuffer;

		uint32	m_indexCountSkybox;
		uint32	m_indexCountPyramid;
		uint32	m_indexCountModel;
		uint32	m_indexCountCherry;
		uint32	m_indexCountFloor;
		uint32	m_indexCountTerrain;
		uint32	m_indexCountCastle;
		uint32	m_indexCountKnight;
		uint32	m_indexCountRabbit;

		//texture variables
		//ComPtr<ID3D11Texture2D> diffuseTexture; //what we load our pixel data into
		//for skybox
		//ComPtr<ID3D11Texture2D> enviTexture;
		//ID3D11Texture2D* enviTexture;
		//ComPtr<ID3D11Texture2D> texRTT;
		ID3D11Texture2D* texRTT;
		ID3D11Texture2D* texDepthRTT;
		ID3D11Texture2D* texHM;

		//cant use comptr in function because it keep become null, auto clean?

		//ComPtr<ID3D11ShaderResourceView> srv;
		//ComPtr<ID3D11SamplerState> samplerState; 
		ID3D11ShaderResourceView *srv;
		ID3D11ShaderResourceView *srvBrick;
		ID3D11ShaderResourceView *srvKnight;
		ID3D11ShaderResourceView *srvRabbit;
		ID3D11ShaderResourceView *srvWolf;
		ID3D11ShaderResourceView *srvCherry;
		ID3D11ShaderResourceView *srvEnvi;
		ID3D11ShaderResourceView *srvRTT;
		ID3D11ShaderResourceView *srvHM;
		vector<ID3D11ShaderResourceView*> srvCloud;	//for animated cloud
		ID3D11SamplerState *samplerState;

		CD3D11_RASTERIZER_DESC rasterDescFrame;
		CD3D11_RASTERIZER_DESC rasterDescSolid;
		//ComPtr<ID3D11RasterizerState> rasterState;
		ID3D11RasterizerState *rasterStateFrame;
		ID3D11RasterizerState *rasterStateSolid;


		ID3D11RenderTargetView* rtv;
		ID3D11DepthStencilView* dsv;
		//will use this for Render to texture
		ID3D11RenderTargetView* rtvRTT;
		ID3D11DepthStencilView* dsvRTT;
		D3D11_VIEWPORT vpRTT;
		XMMATRIX targetPosRTT;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		//put outside to access from Render() to rotate after draw Skybox
		float radians;

		XMFLOAT4X4 world, camera, proj;

	};
}

