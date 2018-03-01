#include "IkFkSwitchNode.h"

MTypeId IkFkSwitchNode::id(0x0000232);
MObject IkFkSwitchNode::aFakeJoints;
MObject IkFkSwitchNode::aRealJoints;
MObject IkFkSwitchNode::aRealSelected;
MObject IkFkSwitchNode::aFakeSelected;
MObject IkFkSwitchNode::aSwitchBool;

IkFkSwitchNode::IkFkSwitchNode() {
}

IkFkSwitchNode::~IkFkSwitchNode() {

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

	if (plug != aRealSelected && plug != aFakeSelected) {
		MGlobal::displayInfo("SOMETHING ELSE CHANGED");
		return MS::kUnknownParameter;
	}

	MGlobal::displayInfo("SWITCH BOOL CHANGED");

	MDataHandle switchBool = data.inputValue(aSwitchBool, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	bool ik = switchBool.asBool();

	// make this change reflect in realSelected and fakeSelected boolean
	MDataHandle realSelected = data.outputValue(aRealSelected, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MDataHandle fakeSelected = data.outputValue(aFakeSelected, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	realSelected.setBool(ik);
	fakeSelected.setBool(!ik);

	// update the callback functions
	updateMatchTransform(ik);

	data.setClean(plug);

	return MS::kSuccess;
}

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

	// update based on reals initially
	updateMatchTransform(true);

	return MStatus::kUnknownParameter;
}

// updates the match transform constraint using either the real or the fake joints
void IkFkSwitchNode::updateMatchTransform(bool realJoints) {
	// detach callbacks
	detachCallbacks();

	// re-register callbacks with the needed data
	for (int i = 0; i < std::min(connectedFakeJoints.size(), connectedRealJoints.size()); i++) {
		// if the fake or the real mobjecthandle is not a kJoint then it represents the end of the joints
		// to match
		if (connectedFakeJoints[i].object().apiType() != MFn::kJoint || 
			connectedRealJoints[i].object().apiType() != MFn::kJoint) {
			break;
		}
		// attach a callback that makes the fake move with the real.
		// TODO: make this toggleable (real follow fake vs. fake follow real)
		// make a JointMatchInfo object to assist
		if (realJoints) {
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
		}
		else {
			int parentIdx = i + 1;
			if (parentIdx >= connectedRealJoints.size()) {
				parentIdx = connectedRealJoints.size() - 1; // if it's the end of the joint chain then it's its own parent
			}
			MObject targetParent = connectedRealJoints[parentIdx].object();
			MObject targetJoint = connectedRealJoints[i].object();
			MObject thisJoint = connectedFakeJoints[i].object();
			JointMatchInfo *info = new JointMatchInfo(i, connectedFakeJoints, MObjectHandle(targetParent), MObjectHandle(targetJoint));
			MCallbackId id = MNodeMessage::addNodeDirtyPlugCallback(thisJoint, matchTransformCallback, static_cast<void*>(info));
			MGlobal::displayInfo("Attach callback (real follow fake)");
			fakeJointCallbackIds.push_back(id);
		}
	}
}

// Called when a connection is broken, or the node is deleted and connections are broken as a result
MStatus IkFkSwitchNode::connectionBroken(const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// when disconnecting a joint stop all the callbacks
	// this will allow us to delete the whole node without lingering callbacks
	// and the system will still update on new attachments
	if (plug == aFakeJoints || plug == aRealJoints) {
		detachCallbacks();
		MGlobal::displayInfo("detach all callbacks");
		// replaces it with an empty mobject
		if (plug == aRealJoints) {
			connectedRealJoints[plug.logicalIndex()] = MObjectHandle(MObject());
		}
		else {
			connectedFakeJoints[plug.logicalIndex()] = MObjectHandle(MObject());
		}
	}

	return MStatus::kUnknownParameter;
}

MStatus IkFkSwitchNode::initialize() {
	
	MStatus status;

	MFnNumericAttribute attrib;
	MFnMessageAttribute messAttrib;

	// create takes the arguments long name, short name where
	// all short names must be unique
	aFakeJoints = messAttrib.create("fakeJoints", "fakeJoints", &status);
	// don't forget the MStatus
	CHECK_MSTATUS_AND_RETURN_IT(status);
	messAttrib.setArray(true);
	addAttribute(aFakeJoints);
	
	aRealJoints = messAttrib.create("realJoints", "realJoints", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	messAttrib.setArray(true);
	addAttribute(aRealJoints);

	aRealSelected = attrib.create("realSelected", "realSelected", MFnNumericData::kBoolean, false, &status);
	attrib.setKeyable(false);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aRealSelected);

	aFakeSelected = attrib.create("fakeSelected", "fakeSelected", MFnNumericData::kBoolean, false, &status);
	attrib.setKeyable(false);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aFakeSelected);

	aSwitchBool = attrib.create("switchIkFk", "switchIkFk", MFnNumericData::kBoolean, false, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	attrib.setKeyable(false);
	addAttribute(aSwitchBool);
	status = attributeAffects(aSwitchBool, aRealSelected);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = attributeAffects(aSwitchBool, aFakeSelected);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}
