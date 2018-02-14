#include "IkFkSwitchNode.h"

MTypeId IkFkSwitchNode::id(0x0000232);
MObject IkFkSwitchNode::aFollowerOut;
MObject IkFkSwitchNode::aTargetM;
MObject IkFkSwitchNode::aToggleMatch;
MObject IkFkSwitchNode::aFollowerInvWorldM;
MObject IkFkSwitchNode::inputMessage;
MObject IkFkSwitchNode::aFollowerMessage;

MObject IkFkSwitchNode::aFakeJoints;

MCallbackId inputId;

IkFkSwitchNode::IkFkSwitchNode() {
}

IkFkSwitchNode::~IkFkSwitchNode() {
}

void* IkFkSwitchNode::creator() {
	return new IkFkSwitchNode();
}

// Called on dirty
MStatus IkFkSwitchNode::compute(const MPlug & plug, MDataBlock & data) {
	MStatus status;

	if (plug != aFollowerMessage) {
		return MS::kUnknownParameter;
	}

	// Toggle match has been changed, that means we should match transform
	// the two objects together

	MDataHandle targetWorld = data.inputValue(aTargetM, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MDataHandle outputWorldInv = data.inputValue(aFollowerInvWorldM, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	bool toggleMatch = data.inputValue(aToggleMatch, &status).asBool();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnMatrixData targetWorldD(targetWorld.data(), &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MFnMatrixData outputInvWorldD(outputWorldInv.data(), &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	double target[3];

	MMatrix outputM = targetWorldD.matrix()*outputInvWorldD.matrix();

	MTransformationMatrix o(outputM);
	/*o.getTranslation(MSpace::kWorld).get(target);


	MDataHandle output = data.outputValue(aFollowerOut, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	output.set3Double(target[0], target[1], target[2]);
	output.setClean();*/

	if (followerJointConnected) {
		MGlobal::displayInfo("follower joint connected, setting transform");
		MFnIkJoint followerJoint = this->followerJoint;
		followerJoint.set(o);
	}

	data.setClean(plug);

	return MS::kSuccess;
}

// Called when input node is dirty
void inputNodeDirty(MObject &node, MPlug &plug, void *clientData) {
	MStatus status;

	MFnIkJoint joint(node);

	double worldTranslation[3];
	joint.getTranslation(MSpace::kObject, &status).get(worldTranslation);
	
	MString str = "X: ";
	str += worldTranslation[0];
	str += " Y: ";
	str += worldTranslation[1];
	str += " Z: ";
	str += worldTranslation[2];
	MGlobal::displayInfo(str);

	// match the input with the follower node
	IkFkSwitchNode *n = (IkFkSwitchNode *)clientData;
	if (n->followerJointConnected) {
		MGlobal::displayInfo("follower joint connected, matching");
		MFnIkJoint follower = n->followerJoint;
		follower.set(joint.transformationMatrix());
	}
}

// Called when a connection is made
MStatus IkFkSwitchNode::connectionMade(const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	if (plug == inputMessage) {
		MGlobal::displayInfo("Input node connected");
		MObject inputNode = otherPlug.node();

		if (inputNode.hasFn(MFn::kJoint)) {
			MGlobal::displayInfo("joint connected");
			MGlobal::displayInfo(inputNode.apiTypeStr());

			// store in object state
			this->inputJoint = inputNode;
			inputJointConnected = true;

			// connect callback, clientData is the node
			inputId = MNodeMessage::addNodeDirtyPlugCallback(inputNode, inputNodeDirty, this);
		}
	}
	else if (plug == aFollowerMessage) {
		MObject inputNode = otherPlug.node();
		if (inputNode.hasFn(MFn::kJoint)) {
			this->followerJoint = inputNode;
			followerJointConnected = true;
		}                                      
	}

	return MStatus::kUnknownParameter;
}

// Called when a connection is broken
MStatus IkFkSwitchNode::connectionBroken(const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	if (plug == inputMessage) {
		MGlobal::displayInfo("Input node disconnected");
		
		// disconnect callback
		MNodeMessage::removeCallback(inputId);
		inputJointConnected = false;
	}
	else if (plug == aFollowerMessage) {
		followerJointConnected = false;
	}

	return MStatus::kUnknownParameter;
}

MStatus IkFkSwitchNode::initialize() {
	
	MStatus status;

	MFnNumericAttribute numAttrib;
	MFnTypedAttribute attrib;
	MFnMessageAttribute messAttrib;

	/*aFollowerOut = numAttrib.create("followerOut", "followerOut", MFnNumericData::k3Double, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numAttrib.setStorable(false);
	numAttrib.setWritable(false);
	numAttrib.setKeyable(false);
	addAttribute(aFollowerOut);*/

	aFollowerMessage = messAttrib.create("followerMessage", "followerMessage", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aFollowerMessage);

	aTargetM = attrib.create("targetWorldMatrix", "targetWorldMatrix", MFnData::kMatrix, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aTargetM);
	attributeAffects(aTargetM, aFollowerMessage);
	attributeAffects(aFollowerInvWorldM, aFollowerMessage);

	aFollowerInvWorldM = attrib.create("followerParentInvM", "followerParentInvM", MFnData::kMatrix, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aFollowerInvWorldM);
	attributeAffects(aFollowerInvWorldM, aFollowerMessage);

	inputMessage = messAttrib.create("inputMessage", "inputMessage", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(inputMessage);

	aToggleMatch = numAttrib.create("toggleMatch", "toggleMatch", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numAttrib.setKeyable(true);
	addAttribute(aToggleMatch);
	attributeAffects(aToggleMatch, aFollowerMessage);

	aFakeJoints = messAttrib.create("fakeJoints", "fakeJoints", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	messAttrib.setArray(true);
	addAttribute(aFakeJoints);

	return MS::kSuccess;
}
