#pragma once

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>

class GaussianNode : public MPxNode {
public:
	GaussianNode();
	virtual ~GaussianNode();
	static void* creator();
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	// specify inputs and outputs for node
	static MStatus initialize();

	static MTypeId id;
	static MObject aOutValue;
	static MObject aInValue;
	static MObject aMagnitude;
	static MObject aMean;
	static MObject aVariance;
};