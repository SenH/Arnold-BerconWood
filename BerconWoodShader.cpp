// MIT License

// Copyright (c) 2017 Sen Haerens

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <ai.h>
#include "BerconWood.h"

#define SHADER_INFO "The Bercon Maps were originally developed by Jerry Ylilammi as open-source plug-ins for 3ds Max. Source code ported to Arnold by Sen Haerens."
AI_SHADER_NODE_EXPORT_METHODS(berconWoodMethods)

enum noiseSpace
{
	NS_WORLD, 
	NS_OBJECT, 
	NS_PREF, 
	NS_UV,
};

static const char* noiseSpaceNames[] =
{
	"world", 
	"object", 
	"Pref", 
	"UV", 
	NULL
};

static const char* woodTypeNames[] = 
{
	"Radial wood", 
	"Perlin wood", 
	"Simplex wood", 
	"Linear wood", 
	NULL
};

enum berconWoodParams
{
	p_space,
	p_offset,
	p_scale,
	p_rotate,
	p_P,
	
	p_color_1,
	p_color_2,
	p_color_grain,
	
	p_seed,
	p_size,
	p_wood_type,
	p_low_threshold,
	p_high_threshold,
	p_skew,
	p_width_variation,
	p_gain_variation,
	
	p_trunk_wobble_strength,
	p_trunk_wobble_frequency,
	
	p_radial_strength,
	p_radial_frequency,
	p_radial_z_frequency,
	
	p_angular_strength,
	p_angular_frequency,
	p_angular_radius,
	
	p_grain_strength,
	p_grain_frequency,
	
	p_info
};

node_parameters
{
	AiParameterEnum("space", NS_OBJECT, noiseSpaceNames);
	AiParameterVec("offset", 0.0f, 0.0f, 0.0f);
	AiParameterVec("scale", 1.0, 1.0, 1.0);
	AiParameterVec("rotate", 0.0f, 0.0f, 0.0f);
	AiParameterVec("P", 0.0f, 0.0f, 0.0f);
	
	AiParameterRGB("color_1", 0.76, 0.56, 0.25);
	AiParameterRGB("color_2", 0.39, 0.24, 0.0);
	AiParameterRGB("color_grain", 0.54, 0.33, 0.0);
	
	AiParameterFlt("seed", 12.345);
	AiParameterFlt("size", 10.0);
	AiParameterEnum("wood_type", 0, woodTypeNames);
	AiParameterFlt("low_threshold", 0.3);
	AiParameterFlt("high_threshold", 1.0);
	AiParameterFlt("skew", 0.75);
	AiParameterFlt("width_variation", 0.5);
	AiParameterFlt("gain_variation", 0.75);
	
	AiParameterFlt("trunk_wobble_strength", 1.0);
	AiParameterFlt("trunk_wobble_frequency", 0.04);
	
	AiParameterFlt("radial_strength", 0.25);
	AiParameterFlt("radial_frequency", 0.1);
	AiParameterFlt("radial_z_frequency", 0.01);
	
	AiParameterFlt("angular_strength", 0.1);
	AiParameterFlt("angular_frequency", 1.0);
	AiParameterFlt("angular_radius", 15.0);
	
	AiParameterFlt("grain_strength", 0.2);
	AiParameterFlt("grain_frequency", 5.0);
	
	AiParameterStr("info", SHADER_INFO);
}

struct ShaderData
{
	int space;
	WoodParams wp;
	float grainStr;
	float grainFreq;
};

node_initialize
{
	ShaderData* data = new ShaderData;
	AiNodeSetLocalData(node, data);
}

node_update
{
	ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

	data->space = AiNodeGetInt(node, "space");

	data->wp.randSeed = AiNodeGetFlt(node, "seed");	
	data->wp.woodType = AiNodeGetInt(node, "wood_type");
	data->wp.lowTresh = AiNodeGetFlt(node, "low_threshold");
	data->wp.highTresh = AiNodeGetFlt(node, "high_threshold");
	data->wp.skew = AiNodeGetFlt(node, "skew");
	data->wp.widthVar = AiNodeGetFlt(node, "width_variation");
	data->wp.gainVar = AiNodeGetFlt(node, "gain_variation");
	
	data->wp.trunkStr = AiNodeGetFlt(node, "trunk_wobble_strength");
	data->wp.trunkFreq = AiNodeGetFlt(node, "trunk_wobble_frequency");
	
	data->wp.radialStr = AiNodeGetFlt(node, "radial_strength");
	data->wp.radialFreq = AiNodeGetFlt(node, "radial_frequency");
	data->wp.radialZ = AiNodeGetFlt(node, "radial_z_frequency");
	
	data->wp.angleStr = AiNodeGetFlt(node, "angular_strength");
	data->wp.angleFreq = AiNodeGetFlt(node, "angular_frequency");
	data->wp.angleRad = AiNodeGetFlt(node, "angular_radius");
	
	data->grainStr = AiNodeGetFlt(node, "grain_strength");
	data->grainFreq = AiNodeGetFlt(node, "grain_frequency");
}

node_finish
{
	ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
	delete data;
}

shader_evaluate
{
	ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
	
	// Evaluate coordinates
	AtVector P, rot;
	AtVector Pin = AiShaderEvalParamVec(p_P);
	static AtString str_Pref("Pref");

	if (AiNodeIsLinked(node, "P"))
	{
		P = Pin;
	}
	else
	{
		switch (data->space)
		{
		case NS_OBJECT:
			P = sg->Po;
			break;
		case NS_UV:
			P.x = sg->u;
			P.y = sg->v;
			P.z = 0.0f;
			break;
		case NS_PREF:
			if (!AiUDataGetVec(str_Pref, P))
				P = sg->Po;
			break;
		default:
			P = sg->P;
			break;
		}
	}
	
	P += AiShaderEvalParamVec(p_offset);
	P *= AiShaderEvalParamVec(p_scale) * AiShaderEvalParamFlt(p_size);
	rot = AiShaderEvalParamVec(p_rotate);
	if (rot != AI_P3_ZERO) 
		P = AiM4VectorByMatrixMult(AiM4Mult(AiM4Mult(AiM4RotationX(rot.x), AiM4RotationY(rot.y)), AiM4RotationZ(rot.z)), P);

	// Evaluate wood & grain
	AtVector Pg;
	float dist = woodNoise(P, Pg, data->wp);
	float grain = (data->grainStr > .001f) ? grainNoise(Pg, data->grainStr, data->grainFreq) : 0.f;
	
	// Evaluate color
	AtRGB color_out;
	color_out = AiLerp(dist, AiShaderEvalParamRGB(p_color_1), AiShaderEvalParamRGB(p_color_2));
	if (grain > 0.f) color_out = AiLerp(grain, color_out, AiShaderEvalParamRGB(p_color_grain));
	
	sg->out.RGB() = color_out;
}

node_loader
{
	if (i > 0)
		return false;

	node->methods	  = berconWoodMethods;
	node->output_type = AI_TYPE_RGB;
	node->name		  = "BerconWood";
	node->node_type	  = AI_NODE_SHADER;
	strcpy(node->version, AI_VERSION);
	return true;
}