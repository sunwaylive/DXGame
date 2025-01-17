//#include "d3dApp.h"
//#include "d3dx11effect.h"
//#include "GeometryGenerator.h"
//#include "MathHelper.h"
//#include "LightHelper.h"
//#include "Effects.h"
//#include "Vertex.h"
//#include "RenderStates.h"
//#include "Camera.h"
//
//enum RenderOptions
//{
//	Lighting = 0,
//	Textures = 1,
//	TexturesAndFog = 2
//};
//
//class MirrorApp : public D3DApp
//{
//public:
//	MirrorApp(HINSTANCE hInstance);
//	~MirrorApp();
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
//	void BuildRoomGeometryBuffer();
//	void BuildSkullGeometryBuffer();
//
//private:
//	ID3D11Buffer *mRoomVB;
//
//	ID3D11Buffer *mSkullVB;
//	ID3D11Buffer *mSkullIB;
//
//	ID3D11ShaderResourceView *mFloorDiffuseMapSRV;
//	ID3D11ShaderResourceView *mWallDiffuseMapSRV;
//	ID3D11ShaderResourceView *mMirrorDiffuseMapSRV;
//	
//	DirectionalLight mDirLights[3];
//	Material mRoomMat;
//	Material mSkullMat;
//	Material mMirrorMat;
//	Material mShadowMat;
//
//	XMFLOAT4X4 mRoomWorld;
//	XMFLOAT4X4 mSkullWorld;
//
//	UINT mSkullIndexCount;
//	XMFLOAT3 mSkullTranslation;
//
//	Camera mCam;
//	XMFLOAT4X4 mView;
//	XMFLOAT4X4 mProj;
//
//	RenderOptions mRenderOptions;
//
//	XMFLOAT3 mEyePosW;
//
//	float mTheta;
//	float mPhi;
//	float mRadius;
//
//	POINT mLastMousePos;
//};
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
//	PSTR cmdLine, int showCmd)
//{
//	// Enable run-time memory check for debug builds.
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//	MirrorApp theApp(hInstance);
//
//	if (!theApp.Init())
//		return 0;
//
//	return theApp.Run();
//}
//
//MirrorApp::MirrorApp(HINSTANCE hInstance)
//: D3DApp(hInstance), mRoomVB(0), mSkullVB(0), mSkullIB(0), mSkullIndexCount(0), mSkullTranslation(0.0f, 1.0f, -5.0f),
//mFloorDiffuseMapSRV(0), mWallDiffuseMapSRV(0), mMirrorDiffuseMapSRV(0),
//mEyePosW(0.0f, 0.0f, 0.0f), mRenderOptions(RenderOptions::Textures),
//mTheta(1.24f*MathHelper::Pi), mPhi(0.42f*MathHelper::Pi), mRadius(12.0f)
//{
//	mMainWndCaption = L"Mirror Demo";
//	mEnable4xMsaa = false;
//
//	mCam.SetPosition(0.0f, 2.0f, -15.0f);
//
//	mLastMousePos.x = 0;
//	mLastMousePos.y = 0;
//
//	XMMATRIX I = XMMatrixIdentity();
//	XMStoreFloat4x4(&mRoomWorld, I);
//	XMStoreFloat4x4(&mView, I);
//	XMStoreFloat4x4(&mProj, I);
//
//	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
//	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
//	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
//	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
//
//	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
//	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
//	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
//
//	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
//	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
//
//	mRoomMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
//	mRoomMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	mRoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
//
//	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
//	mSkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
//
//	// Reflected material is transparent so it blends into mirror.
//	mMirrorMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
//	mMirrorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
//	mMirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
//
//	mShadowMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	mShadowMat.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
//	mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
//}
//
//MirrorApp::~MirrorApp()
//{
//	md3dImmediateContext->ClearState();
//	ReleaseCOM(mRoomVB);
//	ReleaseCOM(mSkullVB);
//	ReleaseCOM(mSkullIB);
//	ReleaseCOM(mFloorDiffuseMapSRV);
//	ReleaseCOM(mWallDiffuseMapSRV);
//	ReleaseCOM(mMirrorDiffuseMapSRV);
//
//	Effects::DestroyAll();
//	InputLayouts::DestroyAll();
//	RenderStates::DestroyAll();
//}
//
//bool MirrorApp::Init()
//{
//	if (!D3DApp::Init())
//		return false;
//
//	Effects::InitAll(md3dDevice);
//	InputLayouts::InitAll(md3dDevice);
//	RenderStates::InitAll(md3dDevice);
//
//	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/checkboard.dds", NULL, NULL, &mFloorDiffuseMapSRV, NULL));
//	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/brick01.dds", NULL, NULL, &mWallDiffuseMapSRV, NULL));
//	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/ice.dds", NULL, NULL, &mMirrorDiffuseMapSRV, NULL));
//
//	//BuildRoomGeometryBuffer();
//	BuildSkullGeometryBuffer();
//
//	return true;
//}
//
//void MirrorApp::OnResize()
//{
//	D3DApp::OnResize();
//	
//	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
//}
//
//void MirrorApp::UpdateScene(float dt)
//{
//	//
//	// Control the camera.
//	//
//	if (GetAsyncKeyState('W') & 0x8000)
//		mCam.Walk(10.0f*dt);
//
//	if (GetAsyncKeyState('S') & 0x8000)
//		mCam.Walk(-10.0f*dt);
//
//	if (GetAsyncKeyState('A') & 0x8000)
//		mCam.Strafe(-10.0f*dt);
//
//	if (GetAsyncKeyState('D') & 0x8000)
//		mCam.Strafe(10.0f*dt);
//
//	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
//		mCam.RiseOrDrop(10.0f*dt);
//
//	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
//		mCam.RiseOrDrop(-10.0f * dt);
//	//
//	// Switch the render mode based in key input.
//	//
//	if (GetAsyncKeyState('1') & 0x8000)
//		mRenderOptions = RenderOptions::Lighting;
//
//	if (GetAsyncKeyState('2') & 0x8000)
//		mRenderOptions = RenderOptions::Textures;
//
//	if (GetAsyncKeyState('3') & 0x8000)
//		mRenderOptions = RenderOptions::TexturesAndFog;
//		
//	//
//	// Allow user to move box.
//	//
//	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
//		mSkullTranslation.x -= 1.0f*dt;
//
//	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
//		mSkullTranslation.x += 1.0f*dt;
//
//	if (GetAsyncKeyState(VK_UP) & 0x8000)
//		mSkullTranslation.y += 1.0f*dt;
//
//	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
//		mSkullTranslation.y -= 1.0f*dt;
//
//	// Don't let user move below ground plane.
//	mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, 0.0f);
//
//	// Update the new world matrix.
//	XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::Pi);
//	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
//	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
//	XMStoreFloat4x4(&mSkullWorld, skullRotate*skullScale*skullOffset);
//}
//
//void MirrorApp::DrawScene()
//{
//	//Do clear every frame
//	/*
//	在每开始绘制一帧时，我们要做的第一件事就是清屏，包括后缓冲区以及深度/模板缓冲区。
//	清屏时我们指定了默认的模板值，比如0，也是我们之前一直做的。
//	如下语句ClearDepthStencilView，最后一个参数就是清屏时设定的模板值
//	*/
//	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, (float*)&Colors::Silver);
//	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
//
//	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
//	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//
//	UINT stride = sizeof(Vertex::Basic32);
//	UINT offset = 0;
//
//	mCam.UpdateViewMatrix();
//
//	XMMATRIX view = mCam.View();
//	XMMATRIX proj = mCam.Proj();
//	XMMATRIX viewProj = mCam.ViewProj();
//
//	//set per frame constants
//	Effects::BasicFX->SetDirLights(mDirLights);
//	Effects::BasicFX->SetEyePosW(mCam.GetPosition());
//	Effects::BasicFX->SetFogColor(Colors::Black);
//	Effects::BasicFX->SetFogStart(2.0f);
//	Effects::BasicFX->SetFogRange(40.f);
//
//	ID3DX11EffectTechnique *activeTech;
//	ID3DX11EffectTechnique *activeSkullTech;
//
//	switch (mRenderOptions)
//	{
//	case RenderOptions::Lighting:
//		activeTech = Effects::BasicFX->Light3Tech;
//		activeSkullTech = Effects::BasicFX->Light3Tech;
//		break;
//	case RenderOptions::Textures:
//		activeTech = Effects::BasicFX->Light3TexTech;
//		activeSkullTech = Effects::BasicFX->Light3Tech;
//		break;
//	case RenderOptions::TexturesAndFog:
//		activeTech = Effects::BasicFX->Light3TexFogTech;
//		activeSkullTech = Effects::BasicFX->Light3FogTech;
//		break;
//	}
//
//	//the core part of using stencil
//	D3DX11_TECHNIQUE_DESC techDesc;
//
//	//draw the floor and walls to the back buffer as usual
//	activeTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		ID3DX11EffectPass *pass = activeTech->GetPassByIndex(p);
//		md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);
//
//		//set per object constants
//		XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
//		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
//		XMMATRIX worldViewProj = world * view * proj;
//
//		Effects::BasicFX->SetWorld(world);
//		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
//		Effects::BasicFX->SetWorldViewProj(worldViewProj);
//
//		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
//		Effects::BasicFX->SetMaterial(mRoomMat);
//
//		//floor
//		Effects::BasicFX->SetDiffuseMap(mFloorDiffuseMapSRV); //SRV: shader resource view
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->Draw(6, 0); //for these vertices, use floor material
//
//		//wall
//		Effects::BasicFX->SetDiffuseMap(mWallDiffuseMapSRV);
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->Draw(18, 6);//for these vertices, use wall material
//	}
//
//	//draw the skull to the back buffer as usual
//	activeSkullTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		ID3DX11EffectPass *pass = activeSkullTech->GetPassByIndex(p);
//		md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
//		md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
//
//		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
//		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
//		XMMATRIX worldViewProj = world * view * proj;
//
//		Effects::BasicFX->SetWorld(world);
//		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
//		Effects::BasicFX->SetWorldViewProj(worldViewProj);
//		Effects::BasicFX->SetMaterial(mSkullMat);
//		
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
//	}
//
//	//draw the mirror to stencil buffer only
//	activeTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		ID3DX11EffectPass *pass = activeTech->GetPassByIndex(p);
//		md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);
//		
//		//set per objects constants
//		XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
//		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
//		XMMATRIX worldViewProj = world * view * proj;
//
//		Effects::BasicFX->SetWorld(world);
//		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
//		Effects::BasicFX->SetWorldViewProj(worldViewProj);
//		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
//
//		//do not write to render target.
//		md3dImmediateContext->OMSetBlendState(RenderStates::NoRenderTargetWritesBS, blendFactor, 0xffffffff);
//
//		//render visible mirror pixels to stencil buffer
//		//do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection
//
//		//!!important, mask out the mirror parts
//		md3dImmediateContext->OMSetDepthStencilState(RenderStates::MarkMirrorDSS, 1);
//
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->Draw(6, 24);
//
//		//reset states.
//		md3dImmediateContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
//		md3dImmediateContext->OMSetDepthStencilState(NULL, 0);
//	}
//
//	//draw the skull reflection.
//	activeSkullTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		ID3DX11EffectPass * pass = activeSkullTech->GetPassByIndex(p);
//		md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
//		md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
//
//		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
//		XMMATRIX R = XMMatrixReflect(mirrorPlane);         //compute the needed matrix
//		
//		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld) * R; //compute the skull world matrix in the mirror
//		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
//		XMMATRIX worldViewProj = world * view * proj;
//
//		Effects::BasicFX->SetWorld(world); //use reflected world matrix
//		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
//		Effects::BasicFX->SetWorldViewProj(worldViewProj);
//		Effects::BasicFX->SetMaterial(mSkullMat);
//
//		//cache the old light dir, and reflect the light dirs;
//		XMFLOAT3 oldLightDirections[3];
//		for (int i = 0; i < 3; ++i)
//		{
//			oldLightDirections[i] = mDirLights[i].Direction;
//
//			XMVECTOR lightDir = XMLoadFloat3(&mDirLights[i].Direction);
//			XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
//			XMStoreFloat3(&mDirLights[i].Direction, reflectedLightDir);
//		}
//
//		Effects::BasicFX->SetDirLights(mDirLights);
//
//		//cull clockwise triangles for reflection. reversed winding order
//		md3dImmediateContext->RSSetState(RenderStates::CullClockwiseRS);
//
//		//only draw reflection into visible mirror pixels as marked by the stencil buffer.
//		md3dImmediateContext->OMSetDepthStencilState(RenderStates::DrawReflectionDSS, 1);
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
//
//		//restore default states.
//		md3dImmediateContext->RSSetState(NULL);
//		md3dImmediateContext->OMSetDepthStencilState(NULL, 0);
//
//		//restore light directions.
//		for (int i = 0; i < 3; ++i)
//		{
//			mDirLights[i].Direction = oldLightDirections[i];
//		}
//
//		Effects::BasicFX->SetDirLights(mDirLights);
//	}
//
//	//draw the mirror to the back buffer as usual but with transparency
//	//blending so the reflection shows through.
//	activeTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		ID3DX11EffectPass * pass = activeTech->GetPassByIndex(p);
//		md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);
//
//		//set per object constants.
//		XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
//		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
//		XMMATRIX worldViewProj = world * view * proj;
//
//		Effects::BasicFX->SetWorld(world);
//		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
//		Effects::BasicFX->SetWorldViewProj(worldViewProj);
//
//		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
//		Effects::BasicFX->SetMaterial(mMirrorMat);
//		Effects::BasicFX->SetDiffuseMap(mMirrorDiffuseMapSRV);
//
//		//Mirror
//		md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->Draw(6, 24);
//	}
//
//	//draw the skull shadow
//	activeSkullTech->GetDesc(&techDesc);
//	for (UINT p = 0; p < techDesc.Passes; ++p)
//	{
//		ID3DX11EffectPass *pass = activeSkullTech->GetPassByIndex(p);
//
//		md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
//		md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
//
//		XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//		XMVECTOR toMainLight = -XMLoadFloat3(&mDirLights[0].Direction);
//		XMMATRIX s = XMMatrixShadow(shadowPlane, toMainLight);
//		XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);
//
//		//set per object constants.
//		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld) * s * shadowOffsetY;
//		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
//		XMMATRIX worldViewProj = world * view * proj;
//
//		Effects::BasicFX->SetWorld(world);
//		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
//		Effects::BasicFX->SetWorldViewProj(worldViewProj);
//
//		Effects::BasicFX->SetMaterial(mShadowMat);
//
//		//use stencil buffer to avoid double blending problem
//		md3dImmediateContext->OMSetDepthStencilState(RenderStates::NoDoubleBlendDSS, 0); 
//		pass->Apply(0, md3dImmediateContext);
//		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
//
//		//reset default state
//		md3dImmediateContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
//		md3dImmediateContext->OMSetDepthStencilState(NULL, 0);
//	}
//
//	HR(mSwapChain->Present(0, 0));
//}
//
//void MirrorApp::OnMouseDown(WPARAM btnState, int x, int y)
//{
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//
//	SetCapture(mhMainWnd);
//}
//
//void MirrorApp::OnMouseUp(WPARAM btnState, int x, int y)
//{
//	ReleaseCapture();
//}
//
//void MirrorApp::OnMouseMove(WPARAM btnState, int x, int y)
//{
//	if ((btnState & MK_RBUTTON) != 0)
//	{
//		// Make each pixel correspond to a quarter of a degree.
//		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
//		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));
//
//		// Update angles based on input to orbit camera around box.
//		mCam.Pitch(dy);
//		mCam.RotateY(dx);
//
//		// Restrict the angle mPhi.
//		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
//	}
//	else if ((btnState & MK_LBUTTON) != 0)
//	{
//		// Make each pixel correspond to 0.01 unit in the scene.
//		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
//		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);
//
//		// Update the camera radius based on input.
//		mRadius += dx - dy;
//
//		// Restrict the radius.
//		mRadius = MathHelper::Clamp(mRadius, 3.0f, 50.0f);
//	}
//
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//}
//
//void MirrorApp::BuildRoomGeometryBuffer()
//{
//	// Create and specify geometry.  For this sample we draw a floor
//	// and a wall with a mirror on it.  We put the floor, wall, and
//	// mirror geometry in one vertex buffer.
//	//
//	//   |--------------|
//	//   |              |
//	//   |----|----|----|
//	//   |Wall|Mirr|Wall|
//	//   |    | or |    |
//	//   /--------------/
//	//  /   Floor      /
//	// /--------------/
//
//	Vertex::Basic32 v[30];
//
//	//floor: observe we tile texture coordinates.
//	v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
//	v[1] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
//	v[2] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
//
//	v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
//	v[4] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
//	v[5] = Vertex::Basic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);
//
//	//wall: observe we tile texture coordinates, and that we leave a gap in the middle for the mirror
//	v[6] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
//	v[7] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
//	v[8] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
//
//	v[9] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
//	v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
//	v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);
//
//	v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
//	v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
//	v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
//
//	v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
//	v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
//	v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);
//
//	v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
//	v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
//	v[20] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
//
//	v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
//	v[22] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
//	v[23] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);
//
//	// Mirror
//	v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
//	v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
//	v[26] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
//
//	v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
//	v[28] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
//	v[29] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
//
//	D3D11_BUFFER_DESC vbd;
//	vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = sizeof(Vertex::Basic32) * 30;
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	vbd.MiscFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA vinitData;
//	vinitData.pSysMem = v;
//
//	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mRoomVB));
//}
//
//void MirrorApp::BuildSkullGeometryBuffer()
//{
//	std::ifstream fin("Models/skull.txt");
//	if (!fin)
//	{
//		MessageBox(NULL, L"Models/skull.txt not found", NULL, 0);
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
//	std::vector<Vertex::Basic32> vertices(vcount);
//	for (UINT i = 0; i < vcount; ++i)
//	{
//		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
//		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
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
//	fin.close();
//
//	D3D11_BUFFER_DESC vbd;
//	vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	vbd.MiscFlags = 0;
//	
//	D3D11_SUBRESOURCE_DATA vinitData;
//	vinitData.pSysMem = &vertices[0];
//	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));
//
//	//pack the indices of all the meshes into one index buffer
//	D3D11_BUFFER_DESC ibd;
//	ibd.Usage = D3D11_USAGE_IMMUTABLE;
//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibd.ByteWidth = sizeof(UINT)* mSkullIndexCount;
//	ibd.CPUAccessFlags = 0;
//	ibd.MiscFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA iinitData;
//	iinitData.pSysMem = &indices[0];
//	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
//}