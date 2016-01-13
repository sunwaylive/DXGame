#include "BasicModel.h"
#include "LoadM3d.h"

BasicModel::BasicModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath)
{
	std::vector<M3dMaterial> mats;
	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(modelFilename, mVertices, mIndices, mSubsets, mats);

	mModelMesh.SetVertices(device, &mVertices[0], mVertices.size());
	mModelMesh.SetIndices(device, &mIndices[0], mIndices.size());
	mModelMesh.SetSubsetTable(mSubsets);

	mSubsetCount = mats.size();

	for(UINT i = 0; i < mSubsetCount; ++i)
	{
		mMatVec.push_back(mats[i].Mat);

		ID3D11ShaderResourceView* diffuseMapSRV = texMgr.CreateTexture(texturePath + mats[i].DiffuseMapName);
		mDiffuseMapSRVVec.push_back(diffuseMapSRV);

		ID3D11ShaderResourceView* normalMapSRV = texMgr.CreateTexture(texturePath + mats[i].NormalMapName);
		mNormalMapSRVVec.push_back(normalMapSRV);
	}
}

BasicModel::~BasicModel()
{
}
