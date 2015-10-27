//#include "d3dApp.h"
//#include "GeometryGenerator.h"
//
//struct Vertex
//{
//	XMFLOAT3 Pos;
//	XMFLOAT4 Color;
//};
//
//class HillsApp : public D3DApp
//{
//public:
//	HillsApp(HINSTANCE hInstance);
//	~HillsApp();
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
//	float GetHeight(float x, float y) const;
//	void BuildGemoetryBuffers();
//	void BuildFX();
//	void BuildVertexLayout();
//
//private:
//	ID3D11Buffer *mVB;
//	ID3D11Buffer *mIB;
//
//	ID3DX11Effect *mFX;
//	ID3DX11EffectTechnique *mTech;
//	ID3DX11EffectMatrixVariable *mfxWorldViewProj;
//	ID3D11InputLayout *mInputLayout;
//
//	//define transformations from local space to world space
//	XMFLOAT4X4 mGridWorld;
//	XMFLOAT4X4 mView;
//	XMFLOAT4X4 mProj;
//
//	float mTheta;
//	float mPhi;
//	float mRadius;
//
//	UINT mGridIndexCount;
//	POINT mLastMousePos;
//};
//
//HillsApp::HillsApp(HINSTANCE hInstance)
//:D3DApp(hInstance), mVB(NULL), mIB(NULL), mFX(NULL), mTech(NULL),
//mfxWorldViewProj(NULL), mInputLayout(NULL), mGridIndexCount(0),
//mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(200.0f)
//{
//	mMainWndCaption = L"Williamwsun Hills Demo";
//	mLastMousePos = {0, 0};
//
//	//set world, view, proj matrix to identity
//	XMMATRIX I = XMMatrixIdentity();
//	XMStoreFloat4x4(&mGridWorld, I);
//	XMStoreFloat4x4(&mView, I);
//	XMStoreFloat4x4(&mProj, I);
//}
//
//HillsApp::~HillsApp()
//{
//	ReleaseCOM(mVB);
//	ReleaseCOM(mIB);
//	ReleaseCOM(mFX);
//	ReleaseCOM(mInputLayout);
//}
//
//bool HillsApp::Init()
//{
//	if (!D3DApp::Init())
//		return false;
//
//	BuildGemoetryBuffers();
//	BuildFX();
//	BuildVertexLayout();
//	return true;
//}
//
//void HillsApp::OnResize()
//{
//	D3DApp::OnResize();
//
//	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.f);
//	XMStoreFloat4x4(&mProj, p);
//}
//
//void HillsApp::BuildGemoetryBuffers()
//{
//	GeometryGenerator::MeshData grid;
//	GeometryGenerator geoGen;
//	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);
//	mGridIndexCount = grid.Indices.size();
//
//	//
//	// Extract the vertex elements we are interested and apply the height function to
//	// each vertex.  In addition, color the vertices based on their height so we have
//	// sandy looking beaches, grassy low hills, and snow mountain peaks.
//	//
//
//	std::vector<Vertex> vertices(grid.Vertices.size());
//	for (int i = 0; i < grid.Vertices.size(); ++i)
//	{
//		XMFLOAT3 p = grid.Vertices[i].Position;
//		p.y = GetHeight(p.x, p.z);
//		vertices[i].Pos = p;
//
//		// Color the vertex based on its height.
//		if (p.y < -10.0f)
//		{
//			// Sandy beach color.
//			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
//		}
//		else if (p.y < 5.0f)
//		{
//			// Light yellow-green.
//			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
//		}
//		else if (p.y < 12.0f)
//		{
//			// Dark yellow-green.
//			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
//		}
//		else if (p.y < 20.0f)
//		{
//			// Dark brown.
//			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
//		}
//		else
//		{
//			// White snow.
//			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//		}
//	}
//
//	//create vertex buffer
//	D3D11_BUFFER_DESC vbd;
//	vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = sizeof(Vertex)* grid.Vertices.size();
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	vbd.MiscFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA vinitData;
//	vinitData.pSysMem = &vertices[0];
//	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));
//
//	//create index buffer
//	D3D11_BUFFER_DESC ibd;
//	ibd.Usage = D3D11_USAGE_IMMUTABLE;
//	ibd.ByteWidth = sizeof(UINT)* mGridIndexCount;
//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibd.CPUAccessFlags = 0;
//	ibd.MiscFlags = 0;
//	
//	D3D11_SUBRESOURCE_DATA iinitData;
//	iinitData.pSysMem = &grid.Indices[0];
//	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
//}
//
//void HillsApp::BuildFX()
//{
//	std::ifstream fin("fx/color.fxo", std::ios::binary);
//	fin.seekg(0, std::ios_base::end);
//	int size = (int)fin.tellg();
//	fin.seekg(0, std::ios_base::beg);
//	
//	std::vector<char> compiledShader(size);
//	fin.read(&compiledShader[0], size);
//	fin.close();
//
//	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, md3dDevice, &mFX));
//
//	mTech = mFX->GetTechniqueByName("ColorTech");
//	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
//}
//
//void HillsApp::BuildVertexLayout()
//{
//	//create the vertex input layout
//	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
//	{
//		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
//	};
//
//	//create the input layout
//	D3DX11_PASS_DESC passDesc;
//	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
//	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
//}
//
//void HillsApp::UpdateScene(float dt)
//{
//	float x = mRadius*sinf(mPhi)*cosf(mTheta);
//	float z = mRadius*sinf(mPhi)*sinf(mTheta);
//	float y = mRadius*cosf(mPhi);
//
//	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
//	XMVECTOR target = XMVectorZero();
//	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//
//	XMMATRIX v = XMMatrixLookAtLH(pos, target, up);
//	XMStoreFloat4x4(&mView, v);
//}
//
//void HillsApp::DrawScene()
//{
//	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, (float*)&Colors::Cyan);
//	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
//
//	md3dImmediateContext->IASetInputLayout(mInputLayout);
//	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
//	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
//
//	//set constants
//	XMMATRIX view = XMLoadFloat4x4(&mView);
//	XMMATRIX proj = XMLoadFloat4x4(&mProj);
//	XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
//	XMMATRIX worldViewProj = world * view * proj;
//
//	D3DX11_TECHNIQUE_DESC techDesc;
//	mTech->GetDesc(&techDesc);
//	//if there are many passed in one technique, then mix them all, I think
//	for (int i = 0; i < techDesc.Passes; ++i)
//	{
//		//draw the grid
//		mfxWorldViewProj->SetMatrix((float*)&worldViewProj);
//		mTech->GetPassByIndex(i)->Apply(0, md3dImmediateContext);
//		//a draw api submits work to the rendering pipeline
//		md3dImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);
//	}
//
//	HR(mSwapChain->Present(0, 0));
//}
//
//float HillsApp::GetHeight(float x, float z)const
//{
//	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
//}
//
//void HillsApp::OnMouseDown(WPARAM btnState, int x, int y)
//{
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//
//	SetCapture(mhMainWnd);
//}
//
//void HillsApp::OnMouseUp(WPARAM btnState, int x, int y)
//{
//	ReleaseCapture();
//}
//
//void HillsApp::OnMouseMove(WPARAM btnState, int x, int y)
//{
//	if ((btnState & MK_LBUTTON) != 0)
//	{
//		// Make each pixel correspond to a quarter of a degree.
//		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
//		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));
//
//		// Update angles based on input to orbit camera around box.
//		mTheta += dx;
//		mPhi += dy;
//
//		// Restrict the angle mPhi.
//		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
//	}
//	else if ((btnState & MK_RBUTTON) != 0)
//	{
//		// Make each pixel correspond to 0.2 unit in the scene.
//		float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
//		float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);
//
//		// Update the camera radius based on input.
//		mRadius += dx - dy;
//
//		// Restrict the radius.
//		mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
//	}
//
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//}
//
////Main Entry
//int WINAPI	WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//{
//	HillsApp theApp(hInstance);
//
//	if (!theApp.Init())
//		return 0;
//
//	return theApp.Run();
//}
//
