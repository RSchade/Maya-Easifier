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

class IkFkSwitchNode : public MPxNode {
public:
	IkFkSwitchNode();
	virtual ~IkFkSwitchNode();
	static void* creator();
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static MStatus initialize();
	virtual MStatus connectionMade(const MPlug &plug, const MPlug &otherPlug, bool asSrc);
	virtual MStatus connectionBroken(const MPlug &plug, const MPlug &otherPlug, bool asSrc);

	MObject inputJoint;
	MObject followerJoint;
	bool inputJointConnected = false;
	bool followerJointConnected = false;

	static MTypeId id;
	static MObject aFollowerOut;
	static MObject aTargetM;
	static MObject aToggleMatch;
	static MObject aFollowerInvWorldM;
	static MObject inputMessage;
	static MObject aFollowerMessage;

	static MObject aFakeJoints;
};