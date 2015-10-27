//#include "d3dApp.h"
//#include "d3dx11effect.h"
//#include "MathHelper.h"
//#include <iostream>
//using namespace std;
//
//#ifdef _DEBUG
//#pragma comment( linker, "/subsystem:windows /entry:WinMainCRTStartup" )
//#endif
//
//struct Vertex
//{
//	XMFLOAT3 Pos;
//	XMFLOAT4 Color;
//};
//
//class BoxApp : public D3DApp
//{
//public:
//	BoxApp(HINSTANCE hInstance);
//	~BoxApp();
//
//	bool Init();
//	void OnResize();
//	void UpdateScene(float dt);
//	void DrawScene();
//
//	void OnMouseDown(WPARAM btnState, int x, int y);
//	void OnMouseUp(WPARAM btnState, int x, int y);
//	void OnMouseMove(WPARAM btnState, int x, int y);
//
//private:
//	/*void BuildGeometryBuffers();
//	void BuildFX();*/
//	void BuildVertextLayout();
//
//	//for skull mesh
//	void BuildGeometryBuffersSkull();
//	void BuildFXSkull();
//
//private:
//	/*ID3D11Buffer *mBoxVB;
//	ID3D11Buffer *mBoxIB;*/
//
//	ID3DX11Effect *mFX;
//	ID3DX11EffectTechnique *mTech;
//	ID3DX11EffectMatrixVariable *mfxWorldViewProj;
//
//	ID3D11InputLayout *mInputLayout;
//	XMFLOAT4X4 mWorld;
//	XMFLOAT4X4 mView;
//	XMFLOAT4X4 mProj;
//
//	float mTheta;
//	float mPhi;
//	float mRadius;
//
//	POINT mLastMousePos;
//
//	//for skull mesh
//	UINT mSkullIndexCount;
//	ID3D11Buffer *mSkullVB;
//	ID3D11Buffer *mSkullIB;
//
//	ID3DX11Effect *mFXSkull;
//	ID3D11RasterizerState *mWireframeRS;
//	// Define transformations from local spaces to world space.
//	XMFLOAT4X4 mSkullWorld;
//};
//
////*********************Main Entry****************************
//int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
//{
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//	BoxApp theApp(hInstance);
//
//	if (!theApp.Init())
//		return 0;
//
//	return theApp.Run();
//}
//
//BoxApp::BoxApp(HINSTANCE hInstance)
//: D3DApp(hInstance), mTech(NULL),
//mfxWorldViewProj(NULL), mInputLayout(NULL),
//mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(20.0f),
//mSkullIndexCount(0), mWireframeRS(NULL)
//{
//	mMainWndCaption = L"Skull Demo";
//
//	mLastMousePos.x = 0;
//	mLastMousePos.y = 0;
//
//	XMMATRIX I = XMMatrixIdentity();
//	//XMStoreFloat4x4(&mWorld, I);
//	XMStoreFloat4x4(&mView, I);
//	XMStoreFloat4x4(&mProj, I);
//
//	//for skull
//	XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
//	XMStoreFloat4x4(&mSkullWorld, T);
//}
//
//BoxApp::~BoxApp()
//{
//	/*ReleaseCOM(mBoxVB);
//	ReleaseCOM(mBoxIB);*/
//	//ReleaseCOM(mFX);
//	ReleaseCOM(mSkullVB);
//	ReleaseCOM(mSkullIB);
//	ReleaseCOM(mFXSkull);
//	ReleaseCOM(mInputLayout);
//	//for skull
//	ReleaseCOM(mWireframeRS);
//}
//
//bool BoxApp::Init()
//{	
//	if (!D3DApp::Init())
//		return false;
//
//	//draw box
//	/*BuildGeometryBuffers();
//	BuildFX();
//	BuildVertextLayout();*/
//
//	//draw skull
//	BuildGeometryBuffersSkull();
//	BuildFXSkull();
//	BuildVertextLayout();
//
//	//for skull
//	D3D11_RASTERIZER_DESC wireframeDesc;
//	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
//	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
//	wireframeDesc.CullMode = D3D11_CULL_BACK;
//	wireframeDesc.FrontCounterClockwise = false;
//	wireframeDesc.DepthClipEnable = true;
//
//	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));
//
//	return true;
//}
//
//void BoxApp::OnResize()
//{
//	D3DApp::OnResize();
//
//	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
//	XMStoreFloat4x4(&mProj, p);
//}
//
//void BoxApp::UpdateScene(float dt)
//{
//	float x = mRadius * sinf(mPhi) * cosf(mTheta);
//	float z = mRadius * sinf(mPhi) * sinf(mTheta);
//	float y = mRadius * cosf(mPhi);
//
//	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
//	XMVECTOR target = XMVectorZero();
//	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//
//	XMMATRIX v = XMMatrixLookAtLH(pos, target, up);
//	XMStoreFloat4x4(&mView, v);
//}
//
////void BoxApp::DrawScene()
////{
////	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Yellow));
////	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
////
////	md3dImmediateContext->IASetInputLayout(mInputLayout);
////	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
////
////	UINT stride = sizeof(Vertex);
////	UINT offset = 0;
////	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
////	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
////
////	//set constants
////	XMMATRIX world = XMLoadFloat4x4(&mWorld);
////	XMMATRIX view = XMLoadFloat4x4(&mView);
////	XMMATRIX proj = XMLoadFloat4x4(&mProj);
////
////	XMMATRIX worldViewProj = world * view * proj;
////
////	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
////
////	D3DX11_TECHNIQUE_DESC techDesc;
////	mTech->GetDesc(&techDesc);
////	for (UINT p = 0; p < techDesc.Passes; ++p)
////	{
////		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
////
////		md3dImmediateContext->DrawIndexed(36, 0, 0);// 36 vertices needed for 12 triangles in a triangle list
////	}
////	
////	HR(mSwapChain->Present(0, 0));
////}
//
//void BoxApp::DrawScene()
//{
//	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, (float*)&Colors::Blue);
//	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//
//	md3dImmediateContext->IASetInputLayout(mInputLayout);
//	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	md3dImmediateContext->RSSetState(mWireframeRS);
//
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
//	md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
//
//	//set constants
//	XMMATRIX view = XMLoadFloat4x4(&mView);
//	XMMATRIX proj = XMLoadFloat4x4(&mProj);
//	XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
//	XMMATRIX worldViewProj = world * view * proj;
//
//	mfxWorldViewProj->SetMatrix((float*)&worldViewProj);
//
//	//use effect file, get the pass, draw the 
//	D3DX11_TECHNIQUE_DESC techDesc;
//	mTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
//	}
//
//	HR(mSwapChain->Present(0, 0));
//}
//
//void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
//{
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//
//	SetCapture(mhMainWnd);
//}
//
//void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
//{
//	ReleaseCapture();
//}
//
//void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
//{
//	if ((btnState & MK_LBUTTON) != 0)
//	{
//		//make each pixel correspond to a quarter of a degree
//		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
//		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
//		//update angles based on the input to orbit camera around the axis
//		mTheta += dx;
//		mPhi += dy;
//		//restrict the angle mPhi
//		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
//	}
//	else if ((btnState & MK_RBUTTON) != 0)
//	{
//		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
//		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);
//
//		mRadius += dx - dy;
//
//		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
//	}
//
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//}
//
////void BoxApp::BuildGeometryBuffers()
////{
////	Vertex boxVertices[] = 
////	{
////		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::White },
////		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), (const float*)&Colors::Black },
////		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), (const float*)&Colors::Red },
////		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Green },
////		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::Blue },
////		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), (const float*)&Colors::Yellow },
////		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), (const float*)&Colors::Cyan },
////		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Magenta }
////	};
////
////	D3D11_BUFFER_DESC vbd;
////	vbd.Usage = D3D11_USAGE_IMMUTABLE;
////	vbd.ByteWidth = sizeof(Vertex)* 8; //the box has 8 vertexes
////	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
////	vbd.CPUAccessFlags = 0;
////	vbd.MiscFlags = 0;
////	vbd.StructureByteStride = 0;
////
////	D3D11_SUBRESOURCE_DATA vinitData;
////	vinitData.pSysMem = boxVertices;
////
////	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));
////
////	//create the index buffer
////	UINT boxVertexindices[] = {
////		//front face
////		0, 1, 2,
////		0, 2, 3,
////
////		// back face
////		4, 6, 5,
////		4, 7, 6,
////
////		// left face
////		4, 5, 1,
////		4, 1, 0,
////
////		// right face
////		3, 2, 6,
////		3, 6, 7,
////
////		// top face
////		1, 5, 6,
////		1, 6, 2,
////
////		// bottom face
////		4, 0, 3,
////		4, 3, 7
////	};
////
////	D3D11_BUFFER_DESC ibd;
////	ibd.Usage = D3D11_USAGE_IMMUTABLE;
////	ibd.ByteWidth = sizeof(UINT)* 36; //36 elements in the array
////	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
////	ibd.CPUAccessFlags = 0;
////	ibd.MiscFlags = 0;
////	ibd.StructureByteStride = 0;
////
////	D3D11_SUBRESOURCE_DATA iinitData;
////	iinitData.pSysMem = boxVertexindices;
////
////	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
////}
//
////for skull geometry
//void BoxApp::BuildGeometryBuffersSkull()
//{
//	//read from file
//	std::ifstream fin("Models/skull.txt");
//	if (!fin)
//	{
//		MessageBox(0, L"Models/skull.txt not found.", L"File Open Error", 0);
//		return;
//	}
//
//	UINT vcount = 0;
//	UINT tcount = 0;
//	std::string ignore;
//
//	fin >> ignore >> vcount;
//	fin >> ignore >> tcount;
//	fin >> ignore >> ignore >> ignore >> ignore;
//
//	float nx, ny, nz;
//	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
//	XMFLOAT4 red(1.0f, 0.0f, 0.0f, 1.0f);
//
//	std::vector<Vertex> vertices(vcount);
//	for (UINT i = 0; i < vcount; ++i)
//	{
//		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
//		vertices[i].Color = red;
//
//		fin >> nx >> ny >> nz;
//	}
//
//	fin >> ignore;
//	fin >> ignore;
//	fin >> ignore;
//
//	mSkullIndexCount = 3 * tcount;
//	std::vector<UINT> indices(mSkullIndexCount);
//	for (UINT i = 0; i < tcount; ++i)
//	{
//		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
//	}
//
//	fin.close();
//
//	//create vertex buffer
//	D3D11_BUFFER_DESC vbd;
//	vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = sizeof(Vertex)* vcount;
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	vbd.MiscFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA vinitData;
//	vinitData.pSysMem = &vertices[0];
//
//	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));
//
//	//create index buffer
//	D3D11_BUFFER_DESC ibd;
//	ibd.Usage = D3D11_USAGE_IMMUTABLE;
//	ibd.ByteWidth = sizeof(UINT)* mSkullIndexCount;
//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibd.CPUAccessFlags = 0;
//	ibd.MiscFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA iinitData;
//	iinitData.pSysMem = &indices[0];
//	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
//}
//
////void BoxApp::BuildFX()
////{
////	DWORD shaderFlags = 0;
////#if defined(DEBUG) || defined(_DEBUG)
////	shaderFlags |= D3D10_SHADER_DEBUG;
////	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
////#endif
////	
////	ID3D10Blob *compileShader = 0;
////	ID3D10Blob *compilationMsgs = 0;
////	HRESULT hr = D3DX11CompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags, 0, 0, &compileShader, &compilationMsgs, 0);
////
////	//compilationMsgs can store errors or warnings
////	if (compilationMsgs != 0)
////	{
////		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
////		ReleaseCOM(compilationMsgs);
////	}
////
////	if (FAILED(hr))
////	{
////		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
////	}
////
////	HR(D3DX11CreateEffectFromMemory(compileShader->GetBufferPointer(), compileShader->GetBufferSize(), 0, md3dDevice, &mFX));
////
////	//Done with compiled shader
////	ReleaseCOM(compileShader);
////
////	mTech = mFX->GetTechniqueByName("ColorTech");
////	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
////}
//
//void BoxApp::BuildFXSkull()
//{
//	std::ifstream fin("fx/color.fxo", std::ios::binary);
//	fin.seekg(0, std::ios_base::end);
//	int size = (int)fin.tellg();
//	fin.seekg(0, std::ios_base::beg);
//	std::vector<char> compiledShader(size);
//
//	fin.read(&compiledShader[0], size);
//	fin.close();
//
//	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, md3dDevice, &mFXSkull));
//
//	mTech = mFXSkull->GetTechniqueByName("ColorTech");
//	mfxWorldViewProj = mFXSkull->GetVariableByName("gWorldViewProj")->AsMatrix();
//}
//
//void BoxApp::BuildVertextLayout()
//{
//	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
//	{
//		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
//		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
//	};
//
//	//create the input layout
//	D3DX11_PASS_DESC passDesc;
//	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
//	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
//}
