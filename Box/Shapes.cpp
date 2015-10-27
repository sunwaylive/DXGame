#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "Waves.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

class ShapesApp : public D3DApp
{
public:
	ShapesApp(HINSTANCE hInstance);
	~ShapesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertextLayout();

	//for waves
	void BuildWavesGeometryBuffers();

private:
	//vertex buffer and index buffer
	ID3D11Buffer *mVB;
	ID3D11Buffer *mIB;

	//for effect
	ID3DX11Effect *mFX;
	ID3DX11EffectTechnique *mTech;
	ID3DX11EffectMatrixVariable *mfxWorld;
	ID3DX11EffectMatrixVariable *mfxWorldViewProj;
	ID3DX11EffectMatrixVariable *mfxWorldInvTranspose;
	//vertex layout
	ID3D11InputLayout *mInputLayout;

	//Rasterization State
	ID3D11RasterizerState *mWireframeRS;

	//transformations from local space to world space
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	//sphere coordinate to cartesian coordinate
	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	//for waves
	Waves mWaves;
	ID3D11Buffer *mWavesVB;
	ID3D11Buffer *mWavesIB;

	XMFLOAT4X4 mWavesWorld;

	//for light
	ID3DX11EffectVectorVariable *mfxEyePosW;
	ID3DX11EffectVariable *mfxDirLight;
	ID3DX11EffectVariable *mfxPointLight;
	ID3DX11EffectVariable *mfxSpotLight;
	ID3DX11EffectVariable *mfxMaterial;

	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	Material mWavesMat;

	XMFLOAT3 mEyePosW;
};

ShapesApp::ShapesApp(HINSTANCE hInstance)
: D3DApp(hInstance), mVB(NULL), mIB(NULL),
mFX(NULL), mTech(NULL), mfxWorldViewProj(0), mfxWorldInvTranspose(0),
mInputLayout(NULL), mWireframeRS(NULL),
mfxDirLight(NULL), mfxPointLight(NULL), mfxSpotLight(NULL),
mfxEyePosW(NULL), mfxMaterial(NULL),
mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(80.0f)
{
	mMainWndCaption = L"Williamwsun Shapes Demo";
	mLastMousePos = {0, 0};

	//identify the matrix
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
	//place the box
	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	//place the sphere, what's this!! williamwsun
	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[2 * i + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

	//for waves
	XMStoreFloat4x4(&mWavesWorld, I);

	//init lights
	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	// Point light--position is changed every frame to animate in UpdateScene function.
	mPointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPointLight.Range = 25.0f;

	mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot = 96.0f;
	mSpotLight.Range = 10000.0f;

	mWavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
}

ShapesApp::~ShapesApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);

	//for waves
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
}

bool ShapesApp::Init()
{
	if (!D3DApp::Init())
		return false;

	//for wave
	mWaves.Init(160, 160, 0.8f, 0.03f, 3.25f, 0.4f);
	BuildWavesGeometryBuffers();
	BuildGeometryBuffers();
	BuildFX();
	BuildVertextLayout();

	//build RS
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));
		
	return true;
}

void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	//Important: get new proj matrix
	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(),
		1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, p);
}

void ShapesApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius *sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Important: get new view matrix
	XMMATRIX v = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, v);

	//every quarter second, generate a random wave
	static float t_base = 0.0f;
	if (mTimer.TotalTime() - t_base	>= 0.25f)
	{
		t_base += 0.25f;
		DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);
		mWaves.Disturb(i, j, r);
	}
	mWaves.Update(dt);

	//update the dynamic vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex *vb = reinterpret_cast<Vertex*>(mappedData.pData);
	for (int i = 0; i < mWaves.VertexCount(); ++i)
	{
		vb[i].Pos = mWaves[i];
		//vb[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		vb[i].Normal = mWaves.Normal(i);
	}

	md3dImmediateContext->Unmap(mWavesVB, 0);

	//point light
	mPointLight.Position.x = 70.0f * cosf(0.2f * mTimer.TotalTime());
	mPointLight.Position.z = 70.f * sinf(0.2f * mTimer.TotalTime());
	mPointLight.Position.y = 70.f;

	//spot light
	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	mSpotLight.Position = mEyePosW;
	XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));
}

void ShapesApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, (float*)&Colors::Red);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//md3dImmediateContext->RSSetState(mWireframeRS);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	
	//set constants for effects
	XMMATRIX view = XMLoadFloat4x4(&mView); //this will be set using DX11 inside functions
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	//for lights
	mfxDirLight->SetRawValue(&mDirLight, 0, sizeof(mDirLight));
	mfxPointLight->SetRawValue(&mPointLight, 0, sizeof(mPointLight));
	mfxSpotLight->SetRawValue(&mSpotLight, 0, sizeof(mSpotLight));
	mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (int i = 0; i < techDesc.Passes; ++i)
	{
		//try to set the material for all the below objects
		mfxMaterial->SetRawValue(&mWavesMat, 0, sizeof(mWavesMat));

		//Part 1:
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

		//draw the grid
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		mfxWorldViewProj->SetMatrix((float*)&(world * viewProj));
		mTech->GetPassByIndex(i)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);
		//DrawIndexd() https://msdn.microsoft.com/zh-cn/library/windows/desktop/ff476409(v=vs.85).aspx

		//draw the box
		world = XMLoadFloat4x4(&mBoxWorld);
		mfxWorldViewProj->SetMatrix((float*)&(world * viewProj));
		mTech->GetPassByIndex(i)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		//draw center sphere
		world = XMLoadFloat4x4(&mCenterSphere);
		mfxWorldViewProj->SetMatrix((float*)&(world * viewProj));
		mTech->GetPassByIndex(i)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

		//draw the cylinders
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			mfxWorldViewProj->SetMatrix((float*)&(world * viewProj));
			mTech->GetPassByIndex(i)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		//draw the spheres
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			mfxWorldViewProj->SetMatrix((float*)&(world * viewProj));
			mTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}

		//PART2: draw the waves
		//md3dImmediateContext->RSSetState(mWireframeRS);		
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&mWavesWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		mfxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		mfxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		mfxWorldViewProj->SetMatrix((float*)&(world * viewProj));

		mTech->GetPassByIndex(i)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

		//md3dImmediateContext->RSSetState(0);
	}

	//At last
	HR(mSwapChain->Present(0, 0));
}

void ShapesApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	//cache the vertex offsets
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	//cache the index count
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	//cache the start index for each object
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexOffset + mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	//total
	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//pack all the objects into one vertex buffer
	std::vector<Vertex> vertices(totalVertexCount);
	XMFLOAT4 black(0.f, 0.f, 0.f, 1.0f);
	XMFLOAT4 red(1.f, 0.f, 0.f, 1.0f);
	XMFLOAT4 blue(0.f, 0.f, 1.f, 1.0f);


	UINT k = 0;
	for (int i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		//vertices[k].Color = red;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		//vertices[k].Color = black;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		//vertices[k].Color = blue;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		//vertices[k].Color = black;
	}

	//create vertex buffer
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex)* totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];

	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	//pack the index of all the objects
	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	//create index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)* totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void ShapesApp::BuildWavesGeometryBuffers()
{
	//allocate vertex buffer space
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex)* mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));

	//create index buffer
	std::vector<UINT> indices(3 * mWaves.TriangleCount());
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;

	for (int i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + (j + 1);

			k += 6;
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)* indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));
}

void ShapesApp::BuildFX()
{
	std::ifstream fin("fx/Lighting.fxo", std::ios::binary);
	if (!fin.is_open())
	{
		MessageBox(NULL, L"打开文件失败", L"Failed", MB_OK);
		exit(0);
	}

	fin.seekg(0, std::ios_base::end);
	int size = fin.tellg();
	fin.seekg(0, std::ios_base::beg);

	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, md3dDevice, &mFX));

	mTech            = mFX->GetTechniqueByName("LightTech");
	mfxWorld         = mFX->GetVariableByName("gWorld")->AsMatrix();
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mfxWorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	mfxEyePosW       = mFX->GetVariableByName("gEyePosW")->AsVector();
	mfxDirLight      = mFX->GetVariableByName("gDirLight");
	mfxPointLight    = mFX->GetVariableByName("gPointLight");
	mfxSpotLight     = mFX->GetVariableByName("gSpotLight");
	mfxMaterial      = mFX->GetVariableByName("gMaterial");
}

void ShapesApp::BuildVertextLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	//create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	//input layout will be used by the vertex shader in the effect file
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, 
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 200.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ShapesApp theApp(hInstance);
	if (!theApp.Init())
		return 0;

	return theApp.Run();
}