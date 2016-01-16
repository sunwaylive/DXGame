

#include "SkinnedData.h"
#include "MeshGeometry.h"

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Keyframe::~Keyframe()
{
}
 
float BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	float f = Keyframes.back().TimePos;

	return f;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	// t is before the animation started, so just return the first key frame.
	if( t <= Keyframes.front().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		/*
		Returns the affine transformation matrix, built from the scaling, rotation, and translation information.
		XMMATRIX XMMatrixAffineTransformation(
		[in] XMVECTOR Scaling,
		[in] XMVECTOR RotationOrigin,
		[in] XMVECTOR RotationQuaternion,
		[in] XMVECTOR Translation
		);
		*/

		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	// t is after the animation ended, so just return the last key frame.
	else if( t >= Keyframes.back().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	//t is between two key frames, so interpolate.
	else
	{
		for(UINT i = 0; i < Keyframes.size()-1; ++i)
		{
			//Find the interval of target time
			if( t >= Keyframes[i].TimePos && t <= Keyframes[i+1].TimePos )
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i+1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i+1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i+1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i+1].RotationQuat);

				/*
				XMVECTOR XMVectorLerp(
				[in] XMVECTOR V0,
				[in] XMVECTOR V1,
				[in] float    t
				);

				The following pseudo codes demonstrates the operation of the function:
				XMVECTOR Result;

				Result.x = V0.x + t * (V1.x - V0.x);
				Result.y = V0.y + t * (V1.y - V0.y);
				Result.z = V0.z + t * (V1.z - V0.z);
				Result.w = V0.w + t * (V1.w - V0.w);

				return Result;
				*/

				/*
				Note it is fairly simple to use this function for doing a cubic interpolation instead of a linear interpolation as follows:
				XMVECTOR SmoothStep( XMVECTOR V0, XMVECTOR V1, float t )
				{
				t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
				t = t*t*(3.f - 2.f*t);
				return XMVectorLerp( V0, V1, t );
				}
				*/
				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);

				//Interpolates between two unit quaternions, using spherical linear interpolation.
				//Returns the interpolated quaternion. If Q0 and Q1 are not unit quaternions, the resulting interpolation is undefined.
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = MathHelper::Infinity;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const
{
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
	return mBoneHierarchy.size();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, 
		              std::vector<XMFLOAT4X4>& boneOffsets,
		              std::map<std::string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets   = boneOffsets;
	mAnimations    = animations;
}
 
void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,  std::vector<XMFLOAT4X4>& finalTransforms)const
{
	UINT numBones = mBoneOffsets.size();

	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance.
	auto clip = mAnimations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for(UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// Premultiply by the bone offset transform to get the final transform.
	for(UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}