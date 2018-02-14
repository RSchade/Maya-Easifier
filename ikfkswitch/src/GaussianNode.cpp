#include "GaussianNode.h"

MTypeId GaussianNode::id(0x0000231);
MObject GaussianNode::aOutValue;
MObject GaussianNode::aInValue;
MObject GaussianNode::aMagnitude;
MObject GaussianNode::aMean;
MObject GaussianNode::aVariance;

GaussianNode::GaussianNode() {
}

GaussianNode::~GaussianNode() {
}

void* GaussianNode::creator() {
	return new GaussianNode();
}

MStatus GaussianNode::compute(const MPlug & plug, MDataBlock & data) {
	MStatus status;

	if (plug != aOutValue) {
		return MS::kUnknownParameter;
	}

	// check the status code

	float inputValue = data.inputValue(aInValue, &status).asFloat();
	float magnitude = data.inputValue(aMagnitude, &status).asFloat();
	float mean = data.inputValue(aMean, &status).asFloat();
	float variance = data.inputValue(aVariance, &status).asFloat();

	if (variance <= 0.0f) {
		variance = 0.001f;
	}

	float xMinusB = inputValue - mean;
	float output = magnitude*exp(-xMinusB*xMinusB) / 2.0f*variance;

	MDataHandle hOutput = data.outputValue(aOutValue, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	hOutput.setFloat(output);
	hOutput.setClean(); // more efficient than working with plugs
	data.setClean(plug);

	return MS::kSuccess;
}

MStatus GaussianNode::initialize() {
	MStatus status;

	// this is where we specify the inputs and outputs of our
	// gaussian node, defining the interface

	MFnNumericAttribute nAddr;

	aOutValue = nAddr.create("outValue", "outValue", MFnNumericData::kFloat);
	nAddr.setWritable(false);
	nAddr.setStorable(false);
	addAttribute(aOutValue);

	aInValue = nAddr.create("inValue", "inValue", MFnNumericData::kFloat);
	nAddr.setKeyable(true);
	addAttribute(aInValue);
	attributeAffects(aInValue, aOutValue);

	aMagnitude = nAddr.create("magnitude", "magnitude", MFnNumericData::kFloat);
	nAddr.setKeyable(true);
	addAttribute(aMagnitude);
	attributeAffects(aMagnitude, aOutValue);

	addAttribute(aOutValue);
	aMean = nAddr.create("mean", "mean", MFnNumericData::kFloat);
	nAddr.setKeyable(true);
	addAttribute(aMean);
	attributeAffects(aMean, aOutValue);

	aVariance = nAddr.create("variance", "variance", MFnNumericData::kFloat);
	nAddr.setKeyable(true);
	addAttribute(aVariance);
	attributeAffects(aVariance, aOutValue);

	return MS::kSuccess;
}
