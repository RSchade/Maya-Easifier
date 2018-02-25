#include "IkFkSwitchNode.h"

MTypeId IkFkSwitchNode::id(0x0000232);
/*MObject IkFkSwitchNode::aFollowerOut;
MObject IkFkSwitchNode::aTargetM;
MObject IkFkSwitchNode::aToggleMatch;
MObject IkFkSwitchNode::aFollowerInvWorldM;
MObject IkFkSwitchNode::inputMessage;
MObject IkFkSwitchNode::aFollowerMessage;*/

MObject IkFkSwitchNode::aFakeJoints;
MObject IkFkSwitchNode::aRealJoints;

MCallbackId inputId;

IkFkSwitchNode::IkFkSwitchNode() {
}

IkFkSwitchNode::~IkFkSwitchNode() {
	detachCallbacks();
}

void* IkFkSwitchNode::creator() {
	return new IkFkSwitchNode();
}

void IkFkSwitchNode::detachCallbacks() {
	// unregister all callbacks
	for (int i = 0; i < fakeJointCallbackIds.size(); i++) {
		MGlobal::displayInfo("Detach callback (fake)");
		MNodeMessage::removeCallback(fakeJointCallbackIds[i]);
	}
	for (int i = 0; i < realJointCallbackIds.size(); i++) {
		MGlobal::displayInfo("Detach callback (real)");
		MNodeMessage::removeCallback(realJointCallbackIds[i]);
	}
}

// Called when this node is dirtied
MStatus IkFkSwitchNode::compute(const MPlug & plug, MDataBlock & data) {
	MStatus status;

	/*if (plug != aFollowerMessage) {
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

	/*if (followerJointConnected) {
		MGlobal::displayInfo("follower joint connected, setting transform");
		MFnIkJoint followerJoint = this->followerJoint;
		followerJoint.set(o);
	}*/

	data.setClean(plug);

	return MS::kSuccess;
}

// Called when input node is dirty
/*void inputNodeDirty(MObject &node, MPlug &plug, void *clientData) {
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
}*/

// Match transforms the input nodes with the array of
// joints contained in clientData
void matchTransformCallback(MObject &node, MPlug &plug, void *clientData) {
	MFnIkJoint thisJoint(node);
	JointMatchInfo *info = static_cast<JointMatchInfo*>(clientData);
	MFnIkJoint followerJoint = info->getTargetJoint().object();

	// set that to the transformation of the follower joint
	followerJoint.set(thisJoint.transformation());

	MGlobal::displayInfo("callback for joint transform");
}

// Called when a connection is made
MStatus IkFkSwitchNode::connectionMade(const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	MObject inputNode = otherPlug.node();

	// a fake joint is connected
	if (plug == aFakeJoints) {
		MGlobal::displayInfo("Fake Joint Connected");
		// Get the position of the fake joint connected
		unsigned int jointIdx = plug.logicalIndex();

		// Turn the MObject into a MObjectHandle
		// Then place in a vector
		MObjectHandle objHandle(inputNode);
		if (jointIdx >= connectedFakeJoints.size()) { // need to add it to the end
			connectedFakeJoints.push_back(objHandle);
		} else { // replace old joint
			connectedFakeJoints[jointIdx] = objHandle;
		}

		// DEBUG
		cerr << endl << "Changed " << jointIdx << endl;
	}

	// a real joint is connected
	if (plug == aRealJoints) {
		MGlobal::displayInfo("Real Joint Connected");
		// Get the position of the fake joint connected
		unsigned int jointIdx = plug.logicalIndex();

		// Turn the MObject into a MObjectHandle
		// Then place in a vector
		MObjectHandle objHandle(inputNode);
		if (jointIdx >= connectedRealJoints.size()) { // need to add it to the end
			connectedRealJoints.push_back(objHandle);
		} else { // replace old joint
			connectedRealJoints[jointIdx] = objHandle;
		}

		// DEBUG
		cerr << endl << "Changed " << jointIdx << endl;
	}

	// DEBUG: Print out what's in the arrays
	for (int i = 0; i < connectedFakeJoints.size(); i++) {
		cerr << "fake " << i << ": " << connectedFakeJoints.at(i).object().apiTypeStr() << endl;
	}
	for (int i = 0; i < connectedRealJoints.size(); i++) {
		cerr << "real " << i << ": " << connectedRealJoints.at(i).object().apiTypeStr() << endl;
	}

	// detach callbacks
	detachCallbacks();

	// re-register callbacks with the needed data
	for (int i = 0; i < std::min(connectedFakeJoints.size(), connectedRealJoints.size()); i++) {
		// attach a callback that makes the fake move with the real.
		// TODO: make this toggleable (real follow fake vs. fake follow real)
		// make a JointMatchInfo object to assist
		if (plug == aRealJoints) {
			int parentIdx = i + 1;
			if (parentIdx >= connectedFakeJoints.size()) {
				parentIdx = connectedFakeJoints.size() - 1; // if it's the end of the joint chain then it's its own parent
			}
			MObject targetParent = connectedFakeJoints[parentIdx].object();
			MObject targetJoint = connectedFakeJoints[i].object();
			MObject thisJoint = connectedRealJoints[i].object();
			JointMatchInfo *info = new JointMatchInfo(i, connectedRealJoints, MObjectHandle(targetParent), MObjectHandle(targetJoint));
			MCallbackId id = MNodeMessage::addNodeDirtyPlugCallback(thisJoint, matchTransformCallback, static_cast<void*>(info));
			MGlobal::displayInfo("Attach callback (fake follow real)");
			realJointCallbackIds.push_back(id);
		} else if (plug == aFakeJoints) {
			MGlobal::displayInfo("Attach callback (real follow fake)");
		}
	}

	return MStatus::kUnknownParameter;
}

// Called when a connection is broken
MStatus IkFkSwitchNode::connectionBroken(const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// when disconnecting a joint stop the
	// callback function but keep the
	// pointer to the joint in the array (but allow replacement)
	// because it still needs to be used for calculations
	if (plug == aFakeJoints || plug == aRealJoints) {
		// if there is a callback connected
		unsigned int jointIdx = plug.logicalIndex();
		if (plug == aFakeJoints && fakeJointCallbackIds.size() > jointIdx) {
			MGlobal::displayInfo("Detach real to fake callback (disconnected node)");
			MNodeMessage::removeCallback(fakeJointCallbackIds[jointIdx]);
		}
		if(plug == aRealJoints && realJointCallbackIds.size() > jointIdx) {
			MGlobal::displayInfo("Detach fake to real callback (disconnected node)");
			MNodeMessage::removeCallback(realJointCallbackIds[jointIdx]);
		}
		// is this below functionality needed?
		// mark this interrupt specially to not reconnect it when a new joint is connected
	}

	return MStatus::kUnknownParameter;
}

MStatus IkFkSwitchNode::initialize() {
	
	MStatus status;

	/*MFnNumericAttribute numAttrib;
	MFnTypedAttribute attrib;*/
	MFnMessageAttribute messAttrib;

	/*aFollowerOut = numAttrib.create("followerOut", "followerOut", MFnNumericData::k3Double, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numAttrib.setStorable(false);
	numAttrib.setWritable(false);
	numAttrib.setKeyable(false);
	addAttribute(aFollowerOut);*/

	/*aFollowerMessage = messAttrib.create("followerMessage", "followerMessage", &status);
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
	attributeAffects(aToggleMatch, aFollowerMessage);*/

	aFakeJoints = messAttrib.create("fakeJoints", "fakeJoints", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	messAttrib.setArray(true);
	addAttribute(aFakeJoints);
	
	aRealJoints = messAttrib.create("realJoints", "realJoints", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	messAttrib.setArray(true);
	addAttribute(aRealJoints);

	return MS::kSuccess;
}
