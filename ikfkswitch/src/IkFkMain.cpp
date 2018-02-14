#include <maya/MIOStream.h>
#include <maya/MSimple.h>
#include "GaussianNode.h"
#include "IkFkSwitchNode.h"

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin fnPlugin(obj, "Raymond Schade", "1.0", "Any");

	status = fnPlugin.registerNode("gaussian", 
		GaussianNode::id, 
		GaussianNode::creator,
		GaussianNode::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerNode("ikfkswitch",
		IkFkSwitchNode::id,
		IkFkSwitchNode::creator,
		IkFkSwitchNode::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin fnPlugin(obj);

	status = fnPlugin.deregisterNode(GaussianNode::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterNode(IkFkSwitchNode::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}
