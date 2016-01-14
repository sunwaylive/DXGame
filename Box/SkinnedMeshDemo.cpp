//***************************************************************************************
// SkinnedMeshDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates skinned mesh character animation.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//      Press '1' for wire frame
//
//***************************************************************************************

#include "d3dApp.h"
#include "MathHelper.h"
#include "TextureMgr.h"
#include "Sky.h"
#include "SkinnedModel.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include "Effects.h"
#include "RenderStates.h"
#include "GeometryGenerator.h"
#include "MeshGeometry.h"
#include "xnacollision.h"

#include <iostream>

//used for sky sphere
struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

class SkinnedMeshApp : public D3DApp
{
public:
	SkinnedMeshApp(HINSTANCE hInstance); 
	~SkinnedMeshApp();					 
										 
	bool Init(); 						 
	void OnResize();					 
	void UpdateScene(float dt);			 
	void DrawScene();				

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void DrawSceneToSsaoNormalDepthMap();
	void DrawSceneToShadowMap();
	void DrawScreenQuad(ID3D11ShaderResourceView *srv);
	void BuildShadowTransform();
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();
	void BuildScreenQuadGeometryBuffers();

	//Pick things in the world
	void Pick(int px, int py);

private:
	TextureMgr mTexMgr;
	Sky *mSky;

	SkinnedModel *mCharacterModel;
	SkinnedModelInstance mCharacterInstance1;
	SkinnedModelInstance mCharacterInstance2;

	ID3D11Buffer *mShapesVB;
	ID3D11Buffer *mShapesIB;

	ID3D11Buffer *mSkullVB;
	ID3D11Buffer *mSkullIB;

	ID3D11Buffer* mSkySphereVB;
	ID3D11Buffer* mSkySphereIB;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	ID3D11ShaderResourceView *mStoneTexSRV;
	ID3D11ShaderResourceView *mBrickTexSRV;

	ID3D11ShaderResourceView *mStoneNormalTexSRV;
	ID3D11ShaderResourceView *mBrickNormalTexSRV;

	BoundingSphere mSceneBounds;

	static const int SMapSize = 2048;
	ShadowMap *mSmap;
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	Ssao *mSsao;

	float mLightRotationAngle;
	XMFLOAT3 mOriginalLightDir[3];
	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	//from local to world space
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;

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

	UINT mSkullIndexCount;

	Camera mCam;
	POINT mLastMousePos;

	//For picking ray, defined in xnacollision.h
	XNA::AxisAlignedBox mSkullAABB;
	std::vector<UINT> mSkullIndices;
	std::vector<Vertex::Basic32> mSkullVertices;
	Material mPickedTriangleMat;
	UINT mPickedTriangle;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	SkinnedMeshApp theApp(hInstance);
	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

SkinnedMeshApp::SkinnedMeshApp(HINSTANCE hInstance)
: D3DApp(hInstance), mSky(0), mCharacterModel(0),
  mShapesVB(0), mShapesIB(0), mSkullVB(0), mSkullIB(0), mScreenQuadVB(0), mScreenQuadIB(0),
  mStoneTexSRV(0), mBrickTexSRV(0),
  mStoneNormalTexSRV(0), mBrickNormalTexSRV(0),
  mSkullIndexCount(0), mSmap(0), mSsao(0),
  mLightRotationAngle(0.0f),
  mPickedTriangle(-1)
{
	mMainWndCaption = L"Skinned Mesh";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	// Estimate the scene bounding sphere manually since we know how the scene was constructed.
	// The grid is the "widest object" with a width of 20 and depth of 30.0f, and centered at
	// the world space origin.  In general, you need to loop over every world space vertex
	// position and compute the bounding sphere.

	// the sky sphere 
	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	//move objects from world origin to its right place
	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset)); //SRT

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

	//place cylinders and spheres
	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

	//set lights and material of the objects
	mDirLights[0].Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(1.0f, 0.9f, 0.9f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	// Shadow acne gets worse as we increase the slope of the polygon (from the
	// perspective of the light).
	//mDirLights[0].Direction = XMFLOAT3(5.0f/sqrtf(50.0f), -5.0f/sqrtf(50.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(125.0f), -5.0f/sqrtf(125.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(116.0f), -4.0f/sqrtf(116.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(109.0f), -3.0f/sqrtf(109.0f), 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	mOriginalLightDir[0] = mDirLights[0].Direction;
	mOriginalLightDir[1] = mDirLights[1].Direction;
	mOriginalLightDir[2] = mDirLights[2].Direction;

	mGridMat.Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.3f, 0.4f, 0.5f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	mBoxMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);
	mSkullMat.Reflect = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	mPickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

SkinnedMeshApp::~SkinnedMeshApp()
{
	SafeDelete(mCharacterModel);

	SafeDelete(mSky);
	SafeDelete(mSmap);
	SafeDelete(mSsao);

	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool SkinnedMeshApp::Init()
{
	if (!D3DApp::Init())
		return false;

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mTexMgr.Init(md3dDevice);

	mSky = new Sky(md3dDevice, L"Textures/desertCube1024.dds", 5000.0f);
	mSmap = new ShadowMap(md3dDevice, SMapSize, SMapSize);

	mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mSsao = new Ssao(md3dDevice, md3dImmediateContext, mClientWidth, mClientHeight, mCam.GetFovY(), mCam.GetFarZ());

	mStoneTexSRV = mTexMgr.CreateTexture(L"Textures/floor.dds");
	mBrickTexSRV = mTexMgr.CreateTexture(L"Textures/bricks.dds");
	mStoneNormalTexSRV = mTexMgr.CreateTexture(L"Textures/floor_nmap.dds");
	mBrickNormalTexSRV = mTexMgr.CreateTexture(L"Textures/bricks_nmap.dds");

	BuildShapeGeometryBuffers();
	BuildSkullGeometryBuffers();
	BuildScreenQuadGeometryBuffers();

	//this is the key contents of this demo
	mCharacterModel = new SkinnedModel(md3dDevice, mTexMgr, "Models\\soldier.m3d", L"Textures\\");
	mCharacterInstance1.Model = mCharacterModel;
	mCharacterInstance2.Model = mCharacterModel;
	mCharacterInstance1.TimePos = 0.0f;
	mCharacterInstance2.TimePos = 0.0f;
	mCharacterInstance1.ClipName = "Take1";
	mCharacterInstance2.ClipName = "Take1";
	mCharacterInstance1.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());
	mCharacterInstance2.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());

	//Reflect to change coordinate system from the RHS the data was exported out
	XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
	XMMATRIX modelRot = XMMatrixRotationY(MathHelper::Pi);
	XMMATRIX modelOffset = XMMatrixTranslation(-2.0f, 0.0f, -7.0f);
	XMStoreFloat4x4(&mCharacterInstance1.World, modelScale * modelRot * modelOffset);

	modelOffset = XMMatrixTranslation(2.0f, 0.0f, -7.0f);
	XMStoreFloat4x4(&mCharacterInstance2.World, modelScale * modelRot * modelOffset);

	return true;
}

void SkinnedMeshApp::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 50, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

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

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			box.Vertices[i].TangentU.x,
			box.Vertices[i].TangentU.y,
			box.Vertices[i].TangentU.z,
			1.0f);
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			grid.Vertices[i].TangentU.x,
			grid.Vertices[i].TangentU.y,
			grid.Vertices[i].TangentU.z,
			1.0f);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			sphere.Vertices[i].TangentU.x,
			sphere.Vertices[i].TangentU.y,
			sphere.Vertices[i].TangentU.z,
			1.0f);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			cylinder.Vertices[i].TangentU.x,
			cylinder.Vertices[i].TangentU.y,
			cylinder.Vertices[i].TangentU.z,
			1.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)* totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}

void SkinnedMeshApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	
	mSkullVertices.resize(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> mSkullVertices[i].Pos.x >> mSkullVertices[i].Pos.y >> mSkullVertices[i].Pos.z;
		fin >> mSkullVertices[i].Normal.x >> mSkullVertices[i].Normal.y >> mSkullVertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&mSkullVertices[i].Pos);
		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mSkullAABB.Center, 0.5 * (vMin + vMax));
	XMStoreFloat3(&mSkullAABB.Extents, 0.5 * (vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount;
	mSkullIndices.resize(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> mSkullIndices[i * 3 + 0] >> mSkullIndices[i * 3 + 1] >> mSkullIndices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mSkullVertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)* mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mSkullIndices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

void SkinnedMeshApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	if (mSsao)
	{
		mSsao->OnSize(mClientWidth, mClientHeight, mCam.GetFovY(), mCam.GetFarZ());
	}
}

void SkinnedMeshApp::UpdateScene(float dt)
{
	// Control the camera.
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f*dt);

	//animate the character
	mCharacterInstance1.Update(dt);
	mCharacterInstance2.Update(dt);

	//animate the lights
	BuildShadowTransform();

	mCam.UpdateViewMatrix();
}

void SkinnedMeshApp::DrawScene()
{
	//render the scene to the shadow map.
	mSmap->BindDsvAndSetNullRenderTarget(md3dImmediateContext);
	DrawSceneToShadowMap();
	md3dImmediateContext->RSSetState(NULL);

	//Render the view space normals and depths. This render target has the same 
	//dimensions as the back buffer, so we can use the screen view port.
	//This render pass is needed to compute the ambient occlusion(ssao).
	//Notice: we use the main depth/stencil buffer in this pass

	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	mSsao->SetNormalDepthRenderTarget(mDepthStencilView);

	DrawSceneToSsaoNormalDepthMap();

	//After we draw the depth map, we now can compute the ambient occlusion.
	mSsao->ComputeSsao(mCam);
	mSsao->BlurAmbientMap(2);

	//Restore the back and depth buffer and view port to the OM stage.
	ID3D11RenderTargetView *renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));

	// We already laid down scene depth to the depth buffer in the Normal/Depth map pass,
	// so we can set the depth comparison test to EQUALS.This prevents any overdraw
	// in this rendering pass, as only the nearest visible pixels will pass this depth
	// comparison test.

	md3dImmediateContext->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);

	XMMATRIX view     = mCam.View();
	XMMATRIX proj     = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	//Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());
	Effects::BasicFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::BasicFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::BasicFX->SetSsaoMap(mSsao->AmbientSRV());     //ambient occlusion

	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
	Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::NormalMapFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::NormalMapFX->SetSsaoMap(mSsao->AmbientSRV()); //ambient occlusion

	//Figure out which technique to use for different geometry
	ID3DX11EffectTechnique *activeTech        = Effects::NormalMapFX->Light3Tech;
	ID3DX11EffectTechnique *activeSphereTech  = Effects::BasicFX->Light3ReflectTech;
	ID3DX11EffectTechnique *activeSkullTech   = Effects::BasicFX->Light3ReflectTech;
	ID3DX11EffectTechnique *activeSkinnedTech = Effects::NormalMapFX->Light3TexSkinnedTech;

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	//Explanation: 
	//first: move 1 to position axis along x, y, [-1, 1] -> [0, 2]
	//second: scale to normalize, [0, 2] * 0.5 -> [0, 1]
	XMMATRIX toTexSpace(0.5f, 0.0f, 0.0f, 0.0f,
						0.0f, -0.5f, 0.0f, 0.0f, 
						0.0f, 0.0f, 1.0f, 0.0f,  //above 3 lines do scale
						0.5f, 0.5f, 0.0f, 1.0f); //transform (1, 1, 0) * 1/2

	XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

	// Draw the grid, cylinders, and box without any cubemap reflection.
	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	if (GetAsyncKeyState('1') && 0x8000)
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (int p = 0; p < techDesc.Passes; ++p)
	{
		//Draw the grid.
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj * toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world * shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.f, 1.0f));
		Effects::NormalMapFX->SetMaterial(mGridMat);
		Effects::NormalMapFX->SetDiffuseMap(mStoneTexSRV);
		Effects::NormalMapFX->SetNormalMap(mStoneNormalTexSRV);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		//Draw the box
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj * toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world * shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
		Effects::NormalMapFX->SetMaterial(mBoxMat);
		Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
		Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		//Draw the cylinders
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view * proj;

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj * toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world * shadowTransform);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));

			Effects::NormalMapFX->SetMaterial(mCylinderMat);
			Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
			Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);

			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}
	}//end for techDesc

	//Draw the sphere with cube map reflection.
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	activeSphereTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::BasicFX->SetShadowTransform(world*shadowTransform);
			Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
			Effects::BasicFX->SetMaterial(mSphereMat);

			activeSphereTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	//Draw the skull.
	stride = sizeof(Vertex::Basic32);
	offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the skull.
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::BasicFX->SetMaterial(mSkullMat);
		Effects::BasicFX->SetShadowTransform(world*shadowTransform);

		activeSkullTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	
		// Draw just the picked triangle again with a different material to highlight it.
		if (mPickedTriangle != -1)
		{
			// Change depth test from < to <= so that if we draw the same triangle twice, it will still pass
			// the depth test.  This is because we redraw the picked triangle with a different material
			// to highlight it.  
			md3dImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

			Effects::BasicFX->SetMaterial(mPickedTriangleMat);
			activeSkullTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

			// restore default, because this is used before we changed it for drawing pick
			md3dImmediateContext->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);
		}
	}
		
	// Draw the animated characters.
	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);
	activeSkinnedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Instance 1

		world = XMLoadFloat4x4(&mCharacterInstance1.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		Effects::NormalMapFX->SetBoneTransforms(
			&mCharacterInstance1.FinalTransforms[0],
			mCharacterInstance1.FinalTransforms.size());

		for (UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			Effects::NormalMapFX->SetMaterial(mCharacterInstance1.Model->Mat[subset]);
			Effects::NormalMapFX->SetDiffuseMap(mCharacterInstance1.Model->DiffuseMapSRV[subset]);
			Effects::NormalMapFX->SetNormalMap(mCharacterInstance1.Model->NormalMapSRV[subset]);

			activeSkinnedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			mCharacterInstance1.Model->ModelMesh.Draw(md3dImmediateContext, subset);
		}

		// Instance 2

		world = XMLoadFloat4x4(&mCharacterInstance2.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

		Effects::NormalMapFX->SetBoneTransforms(
			&mCharacterInstance2.FinalTransforms[0],
			mCharacterInstance2.FinalTransforms.size());

		for (UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			Effects::NormalMapFX->SetMaterial(mCharacterInstance2.Model->Mat[subset]);
			Effects::NormalMapFX->SetDiffuseMap(mCharacterInstance2.Model->DiffuseMapSRV[subset]);
			Effects::NormalMapFX->SetNormalMap(mCharacterInstance2.Model->NormalMapSRV[subset]);

			activeSkinnedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			mCharacterInstance2.Model->ModelMesh.Draw(md3dImmediateContext, subset);
		}
	}//end for techDesc

	//Turn off wireframe
	md3dImmediateContext->RSSetState(NULL);

	//Restore from RenderStates:EqualsDSS
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	//Debug view SSAO map
	//DrawScreenQuad(mSsap->AmbientSRV());

	mSky->Draw(md3dImmediateContext, mCam);

	//Restore default states, as the SkyFx changed them in the effect file.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	//Unbind shadow map and AmbientMap as a shader input because we are going to render
	//it next frame. These textures can be at any slot, so clear all slots.
	ID3D11ShaderResourceView *nullSRV[16] = { NULL };
	md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	HR(mSwapChain->Present(0, 0));
}

//generate shadow map
void SkinnedMeshApp::DrawSceneToShadowMap()
{
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	Effects::BuildShadowMapFX->SetEyePosW(mCam.GetPosition());
	Effects::BuildShadowMapFX->SetViewProj(viewProj);

	//these properties could be set per object if needed
	Effects::BuildShadowMapFX->SetHeightScale(0.07);
	Effects::BuildShadowMapFX->SetMaxTessDistance(1.0f);
	Effects::BuildShadowMapFX->SetMinTessDistance(25.0f);
	Effects::BuildShadowMapFX->SetMinTessFactor(1.0f);
	Effects::BuildShadowMapFX->SetMaxTessFactor(5.0f);

	ID3DX11EffectTechnique *smapTech = Effects::BuildShadowMapFX->BuildShadowMapTech;
	ID3DX11EffectTechnique *animatedSmapTech = Effects::BuildShadowMapFX->BuildShadowMapSkinnedTech;

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//draw the grid, cylinders, spheres, and boxes
	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;
	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	if (GetAsyncKeyState('1') & 0x8000)
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;
	smapTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//draw the grid
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));

		smapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		//draw the box
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));

		smapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mGridVertexOffset);

		//draw the cylinders
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view * proj;

			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));

			smapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		//draw the sphere
		for (int i = 0; i < 10; i++)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());

			smapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	//Draw the skull
	//Because the input vertex buffer changes, we can not use the same loop
	stride = sizeof(Vertex::Basic32);
	offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	for (int p = 0; p < techDesc.Passes; ++p)
	{
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());

		smapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	//Draw the animated characters.
	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);
	animatedSmapTech->GetDesc(&techDesc);
	for (int p = 0; p < techDesc.Passes; ++p)
	{
		//Instance 1
		world = XMLoadFloat4x4(&mCharacterInstance1.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());
		Effects::BuildShadowMapFX->SetBoneTransforms(&mCharacterInstance1.FinalTransforms[0],
													  mCharacterInstance1.FinalTransforms.size());

		animatedSmapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		for (int subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
			mCharacterInstance1.Model->ModelMesh.Draw(md3dImmediateContext, subset);

		//Instance 2
		world = XMLoadFloat4x4(&mCharacterInstance2.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world * view * proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());
		Effects::BuildShadowMapFX->SetBoneTransforms(&mCharacterInstance2.FinalTransforms[0],
													 mCharacterInstance2.FinalTransforms.size());

		animatedSmapTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		for (int subset = 0; subset < mCharacterInstance2.Model->SubsetCount; ++subset)
			mCharacterInstance2.Model->ModelMesh.Draw(md3dImmediateContext, subset);
	}

	md3dImmediateContext->RSSetState(NULL);
}

void SkinnedMeshApp::DrawScreenQuad(ID3D11ShaderResourceView* srv)
{
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Effects::DebugTexFX->SetWorldViewProj(world);
		Effects::DebugTexFX->SetTexture(srv);

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}
}


void SkinnedMeshApp::DrawSceneToSsaoNormalDepthMap()
{
	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	ID3DX11EffectTechnique *tech = Effects::SsaoNormalDepthFX->NormalDepthTech;
	ID3DX11EffectTechnique *animatedTech = Effects::SsaoNormalDepthFX->NormalDepthSkinnedTech;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;

	//Draw the grid, cylinders, sphere and box
	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;
	//Bind information about the primitive type, and data order that describes input data for the input assembler stage.
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Input-layout objects describe how vertex buffer data is streamed into the IA pipeline stage.
	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	//Set index buffer
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	if (GetAsyncKeyState('1') & 0x8000)
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (int p = 0; p < techDesc.Passes; ++p)
	{
		//Draw the grid
		world                 = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose     = MathHelper::InverseTranspose(world);
		worldView             = world * view;
		worldInvTransposeView = worldInvTranspose * view;
		worldViewProj         = world * view * proj;

		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		//Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world * view;
		worldInvTransposeView = worldInvTranspose * view;
		worldViewProj = world * view * proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView = world*view;
			worldInvTransposeView = worldInvTranspose*view;
			worldViewProj = world*view*proj;

			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView = world*view;
			worldInvTransposeView = worldInvTranspose*view;
			worldViewProj = world*view*proj;

			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());

			tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}//end for techDesc

	//Draw the skull
	stride = sizeof(Vertex::Basic32);
	offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	for (int p = 0; p < techDesc.Passes; ++p)
	{
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	//Draw the animated characters.
	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);
	animatedTech->GetDesc(&techDesc);
	for (int p = 0; p < techDesc.Passes; ++p)
	{
		//Instance 1
		world = XMLoadFloat4x4(&mCharacterInstance1.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());
		Effects::SsaoNormalDepthFX->SetBoneTransforms(&mCharacterInstance1.FinalTransforms[0],
													   mCharacterInstance1.FinalTransforms.size());

		animatedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		for (int subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
			mCharacterInstance1.Model->ModelMesh.Draw(md3dImmediateContext, subset);

		// Instance 2
		world = XMLoadFloat4x4(&mCharacterInstance2.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());
		Effects::SsaoNormalDepthFX->SetBoneTransforms(
			&mCharacterInstance2.FinalTransforms[0],
			mCharacterInstance2.FinalTransforms.size());

		animatedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		for (UINT subset = 0; subset < mCharacterInstance2.Model->SubsetCount; ++subset)
		{
			mCharacterInstance2.Model->ModelMesh.Draw(md3dImmediateContext, subset);
		}
	}

	md3dImmediateContext->RSSetState(NULL); //reset the render state
}

void SkinnedMeshApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		Pick(x, y);
	}
}

void SkinnedMeshApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SkinnedMeshApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SkinnedMeshApp::BuildShadowTransform()
{
	//only the first "main" light casts a shadow
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f * mSceneBounds.Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Builds a view matrix for a left - handed coordinate system using a camera position, an up direction, and a focal point.
	//Returns a view matrix that transforms a point from world space into view space.
	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	//transform bounding sphere to light space
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	//orthogonal frustum in light space encloses scene
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	//Builds a custom orthogonal projection matrix for a left-handed coordinate system
	//Returns the custom orthogonal projection matrix.
	//All the parameters of XMMatrixOrthographicOffCenterLH are distances in camera space.
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	//Explanation: 
	//first: move 1 to position axis along x, y, [-1, 1] -> [0, 2]
	//second: scale to normalize, [0, 2] * 0.5 -> [0, 1]
	XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f,
			   0.0f, -0.5f, 0.0f, 0.0f,
			   0.0f, 0.0f, 1.0f, 0.0f,  //above 3 lines do scale
			   0.5f, 0.5f, 0.0f, 1.0f); //transform (1, 1, 0) * 1/2

	XMMATRIX S = V * P * T; //lightview * othogonal proj * NDC2texture

	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S); 
}

void SkinnedMeshApp::BuildScreenQuadGeometryBuffers()
{
	GeometryGenerator::MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

	for (UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].Tex = quad.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)* quad.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &quad.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void SkinnedMeshApp::Pick(int px, int py)
{
	//BUG: We can only pick points in the character
	XMFLOAT4X4 mMeshWorld = mSkullWorld;

	XMMATRIX P = mCam.Proj();
	 
	//Compute picking ray in view space.
	float vx = (+2.0f * px / mClientWidth - 1.0f) / P(0, 0);
	float vy = (-2.0f * py / mClientHeight + 1.0f) / P(1, 1);

	//Ray definition in view space.
	//every time you pick an object, the picking ray always start from origin point
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	//Always use depth = 1.0 point
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	//Transform ray to local
	XMMATRIX V = mCam.View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	XMMATRIX W = XMLoadFloat4x4(&mMeshWorld);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	//Transform to another coordinate system.
	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	//Normalize ray dir
	rayDir = XMVector3Normalize(rayDir);

	//If we hit the bounding box of a mesh, then we might have picked a Mesh
	//triangle, to continue to do finer ray/triangle test.

	//Assume we have not picked anything, so init to -1
	mPickedTriangle = -1;
	float tmin = 0.0f;

	if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &mSkullAABB, &tmin))
	{
		//Find the nearest ray/triangle intersection
		tmin = MathHelper::Infinity;
		for (int i = 0; i < mSkullIndices.size() / 3; ++i)
		{
			//Indices for this triangle
			UINT i0 = mSkullIndices[i * 3 + 0];
			UINT i1 = mSkullIndices[i * 3 + 1];
			UINT i2 = mSkullIndices[i * 3 + 2];

			//Vertices for this triangle.
			XMVECTOR v0 = XMLoadFloat3(&mSkullVertices[i0].Pos);
			XMVECTOR v1 = XMLoadFloat3(&mSkullVertices[i1].Pos);
			XMVECTOR v2 = XMLoadFloat3(&mSkullVertices[i2].Pos);

			//We have to iterate over all the triangles in order to find 
			//the nearest intersection.
			float t = 0.0f;
			if (XNA::IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &t))
			{
				if (t < tmin)
				{
					//This is the nearest picked triangle.
					tmin = t;
					mPickedTriangle = i;
				}
			}
		}//end for mSkullIndices
	}//end for if IntersectRayAxisAlignedBox

	std::cout <<"mPickedTriangle: " <<mPickedTriangle << std::endl;
} 