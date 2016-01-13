#ifndef BASICMODEL_H
#define BASICMODEL_H

#include "MeshGeometry.h"
#include "TextureMgr.h"
#include "Vertex.h"

class BasicModel
{
public:
	BasicModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~BasicModel();

	UINT mSubsetCount;

	std::vector<Material> mMatVec;
	std::vector<ID3D11ShaderResourceView*> mDiffuseMapSRVVec;
	std::vector<ID3D11ShaderResourceView*> mNormalMapSRVVec;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTan> mVertices;
	std::vector<USHORT> mIndices;
	std::vector<MeshGeometry::Subset> mSubsets;

	MeshGeometry mModelMesh;
};

struct BasicModelInstance
{
	BasicModel* Model;
	XMFLOAT4X4 World;
};

#endif // SKINNEDMODEL_H