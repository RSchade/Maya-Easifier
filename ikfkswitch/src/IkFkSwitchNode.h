#pragma once

#include <maya/MPxNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MVector.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MMessage.h>
#include <maya/MPlugArray.h>
#include <maya/MGlobal.h>
#include <maya/MFnIkJoint.h>
#include <maya/MNodeMessage.h>
#include <maya/MObjectHandle.h>
#include <maya/MSelectionList.h>
#include <algorithm>
#include <vector>
#include "JointMatchInfo.h"

class IkFkSwitchNode : public MPxNode {
public:
	IkFkSwitchNode();
	virtual ~IkFkSwitchNode();
	static void* creator();
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static MStatus initialize();
	virtual MStatus connectionMade(const MPlug &plug, const MPlug &otherPlug, bool asSrc);
	virtual MStatus connectionBroken(const MPlug &plug, const MPlug &otherPlug, bool asSrc);
	void updateMatchTransform(bool realJoints);

	static MTypeId id;

	static MObject aFakeJoints, aRealJoints, aRealSelected, 
				   aFakeSelected, aSwitchBool;

	// vectors which contain the real and fake joints connected
	// to this node instance
	std::vector<MObjectHandle> connectedFakeJoints;
	std::vector<MObjectHandle> connectedRealJoints;

	// vector that contains all of the callback ids
	std::vector<MCallbackId> fakeJointCallbackIds;
	std::vector<MCallbackId> realJointCallbackIds;
private:
	void detachCallbacks();
};