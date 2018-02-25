#pragma once
#include <maya/MObjectHandle.h>
#include <vector>


// Contains information about a joint to follow
class JointMatchInfo {
	private:
		std::vector<MObjectHandle> joints;
		int placeInChain;
		MObjectHandle targetParent, targetJoint;
	public:
		JointMatchInfo(int placeInChainTemp, std::vector<MObjectHandle> jointsTemp, 
					   MObjectHandle targetParentTemp, MObjectHandle targetJointTemp);
		std::vector<MObjectHandle> &getJoints();
		int getPlaceInChain();
		MObjectHandle getTargetParent();
		MObjectHandle getTargetJoint();
};