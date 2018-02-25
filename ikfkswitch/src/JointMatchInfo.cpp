#include "JointMatchInfo.h"

JointMatchInfo::JointMatchInfo(int placeInChainTemp, std::vector<MObjectHandle> jointsTemp,
							   MObjectHandle targetParentTemp, MObjectHandle targetJointTemp) {
	placeInChain = placeInChainTemp;
	joints = jointsTemp;
	targetParent = targetParentTemp;
	targetJoint = targetJointTemp;
}

std::vector<MObjectHandle> &JointMatchInfo::getJoints() {
	return joints;
}

int JointMatchInfo::getPlaceInChain() {
	return placeInChain;
}

MObjectHandle JointMatchInfo::getTargetParent() {
	return targetParent;
}

MObjectHandle JointMatchInfo::getTargetJoint() {
	return targetJoint;
}
