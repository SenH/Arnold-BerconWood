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

// The Bercon Maps were originally developed by Jerry Ylilammi as open-source plug-ins for 3ds Max.
// https://github.com/Bercon/BerconMaps

#include <ai.h>
#include "BerconWood.h"

#define FASTFLOOR(x) (((x) > 0) ? ((int)x) : (((int)x) - 1))

// https://www.codeproject.com/Tips/700780/Fast-floor-ceiling-functions
// inline int fastfloor(x)
// {
// 	x > 0 ? (int)x : (int)x - 1;
// 	return x;
// }

inline float smooth(float d, float low, float high)
{
	d = (d-low) / (high-low);
	d = AiClamp(d, 0.f, 1.f);
	return AiHerp01(d);
}

float woodNoise(AtVector p, AtVector &g, WoodParams &wp)
{
	// Radial noise
	AtVector radialP = AtVector(p.x*wp.radialFreq, p.y*wp.radialFreq, p.z*wp.radialZ);
	p.x += AiPerlin4(radialP, wp.randSeed-1.125f) * wp.radialStr;
	p.y += AiPerlin4(radialP, wp.randSeed+1.125f) * wp.radialStr;
	g = p; // Store radial component for grain

	// Trunk wobble
	float angle = AiPerlin2(AtVector2(p.z*wp.trunkFreq, wp.randSeed+3.f)) * AI_PI; // Offset so we dont get the same value as below
	p += AtVector(cos(angle), sin(angle), 0.f) * AiPerlin2(AtVector2(p.z*wp.trunkFreq, wp.randSeed-5.5f)) * wp.trunkStr;
	g = .5f*g + .5f*p; // We don't want trunk wobble to affect too much to grain

	// Distance
	float dist = 0.f;
	switch (wp.woodType)
	{
		// Radial wood
		case 0:
		dist = sqrt(p.x*p.x + p.y*p.y);
		break;
		// Perlin wood
		case 1:
		p *= .05f;
		dist = (AiPerlin4(p, wp.randSeed-7.1f) + 1.f) / .5f * 15.f;
		break;
		// Simplex wood
		case 2:
		p *= .05f;
		dist = (AiPerlin4(p, wp.randSeed+3.15f) + 1.f) / .5f * 15.f;
		break;
		// Linear wood
		case 3:
		dist = (p.x < 0.f) ? -p.x : p.x;
		break;
	}

	// Width variation
	dist += AiPerlin2(AtVector2(dist+wp.randSeed*2.f, 0.f)) * wp.widthVar;

	// Angular noise
	float d = dist;
	if (d > wp.angleRad) d = wp.angleRad;
	AtVector angleP = AtVector(p.x*wp.angleFreq, p.y*wp.angleFreq, p.z*wp.angleFreq*.5f);
	dist += AiHerp01(d/wp.angleRad) * AiPerlin4(angleP, wp.randSeed+10.6f) * wp.angleStr;

	// Mod
	float ipart = (float)(int)dist;
	dist -= ipart;

	// Skew
	if (dist < wp.skew)
		dist *= .5f / wp.skew;
	else
		dist = dist*.5f / (1.f-wp.skew) - wp.skew * (.5f/(1.f-wp.skew)) + .5f;

	// Reverse
	dist *= 2.f;
	if (dist > 1.f)
		dist = 2.f-dist;

	// Smooth and scale
	dist = smooth(dist, wp.lowTresh, wp.highTresh);

	// Gain variation
	float p1 = (ipart + wp.randSeed) * 314.134f;
	float gain = (AiPerlin2(AtVector2(p1, 0.f)) + 1.f) * .5f;
	dist *= (1.f-wp.gainVar) + gain * wp.gainVar;
	
	return dist;
}

float fBmNoise(AtVector p, float levels, float lacunarity, float H)
{
	float sum = 0.0f;
	float pwr = 1.0f;
	float pwHL = pow(lacunarity, -H);	

	for (int i=0; i<(int)levels; i++)
	{
		sum += AiPerlin3(p) * pwr;
		pwr *= pwHL;
		p *= lacunarity;
	}

	float rmd = levels - FASTFLOOR(levels);
	if (rmd!=0.f) sum += rmd * AiPerlin3(p) * pwr;

	return (sum+1.f)*.5f;
}

float grainNoise(AtVector p, float amount, float freq)
{
		p *= freq;
		p.z *= .05f;
		float g = (fBmNoise(p, 4.f, 2.f, .5f)+1.f)*.5f;
		return smooth(g, (1.f-amount), 1.f);
}
