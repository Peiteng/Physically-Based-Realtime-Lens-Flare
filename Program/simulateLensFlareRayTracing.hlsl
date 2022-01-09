﻿struct PSInput
{
    float4 pos : SV_POSITION;
    float4 drawInfo : TEXCOORD0;
    float4 coordinates[SAMPLE_LAMBDA_NUM] : WORLD;
    float4 color[SAMPLE_LAMBDA_NUM] : VIEW;
};

struct InfoPerLambda
{
    float4 pos : SV_POSITION;
    float4 drawInfo : TEXCOORD0;
    float4 coordinates : TEXCOORD1;
    float4 reflectance : TEXCOORD2;
};

struct Intersection
{
    float3 pos;
    float3 norm;
    float theta; //incident angle
    bool hit; //intersection found?
    bool inverted; //inverted intersection?
};

struct LensInterface
{
    float3 center; //center of sphere/plane on z-axis
    float radius; //radius of sphere/plane(on xy-plane)
    
    float3 n; //refractive indices (n0 n1 n2) n0 and n2 are the refractive indices of the first and second media,and n1 the refractive index of an anti-reflective coating.
    float sa; //nomial radius (from. opt. axis)
    
    float d1; //coating thicknes = lambda / 4 / n1
    float flat; //is this interface a plane?
    float2 padding;
};

struct Ray
{
    float3 pos;
    float3 dir;
    float4 drawInfo; //xy:aperture tex coord z:reflection w:intensity 
};

struct GhostData
{
    float bounce1;
    float bounce2;
    float2 padding;
};

struct CBuffer
{
    float3 lightDir;
    float spread;
    
    float numInterfaces;
    uint apertureIndex;
    float apertureRadius;
    float padding;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

RWStructuredBuffer<PSInput> traceResult : register(u0);

StructuredBuffer<LensInterface> lensInterface : register(t0);
StructuredBuffer<GhostData> ghostData : register(t1);

SamplerState imageSampler : register(s0);

Intersection intersectionTestFlat(Ray r, LensInterface F)
{
    Intersection i;
    i.pos = r.pos + r.dir * ((F.center.z - r.pos.z) / r.dir.z);
    i.norm = r.dir.z > 0 ? float3(0, 0, -1) : float3(0, 0, 1);
    i.theta = 0; //meaningless
    i.hit = true;
    i.inverted = false;
    return i;
}

Intersection intersectionTestSphere(Ray r, LensInterface F)
{
    const float eps = PI * 0.0001f;
    Intersection i;
    float3 D = r.pos - F.center;
    float B = dot(D, r.dir);
    float C = dot(D, D) - F.radius * F.radius;
    float derterminant = B * B - C;
    if (derterminant < 0)//no intersection
    {
        i.hit = false;
        return i;
    }
    float t = -B + ((F.radius * r.dir.z) > 0 ? 1.f : -1.f) * sqrt(derterminant);
    i.pos = r.dir * t + r.pos;
    i.norm = normalize(i.pos - F.center);
    if (dot(i.norm, r.dir) > 0)
        i.norm *= -1;
    i.theta = acos(dot(-r.dir, i.norm));
    if (abs(i.theta) < eps)//avoid getting hole on grid
    {
        i.theta += eps;
    }
    i.hit = true;
    i.inverted = t < 0; //mark an inverted ray
    return i;
}

Intersection getIntersectionInfo(Ray r, LensInterface F)
{
    Intersection i;
    
    if (F.flat)
        i = intersectionTestFlat(r, F);
    else
        i = intersectionTestSphere(r, F);
    
    return i;
}

float FresnelAR(float theta0RAD, float lambda, float d1, float n0, float n1, float n2)
{
    //refraction angles in coating and the 2nd medium
    float sinTheta0 = sin(theta0RAD);
    float theta1RAD = asin(sinTheta0 * n0 / n1);
    float theta2RAD = asin(sinTheta0 * n0 / n2);

    //amplitude for outer ref1 . / transmission on topmost interface
    float theta0plus1 = theta0RAD + theta1RAD;
    float theta0minus1 = theta0RAD - theta1RAD;
    float rs01 = -sin(theta0minus1) / sin(theta0plus1);
    float rp01 = tan(theta0minus1) / tan(theta0plus1);
    float ts01 = 2 * sin(theta1RAD) * cos(theta0RAD) / sin(theta0plus1);
    float tp01 = ts01 * cos(theta0minus1);

    //amplitude for inner reflection
    float theta1plus2RAD = theta1RAD + theta2RAD;
    float theta1minus2RAD = theta1RAD - theta2RAD;
    float rs12 = -sin(theta1minus2RAD) / sin(theta1plus2RAD);
    float rp12 = +tan(theta1minus2RAD) / tan(theta1plus2RAD);

    //after passing throught first surface twice : 2 transmissions and 1 reflection
    float ris = ts01 * ts01 * rs12;
    float rip = tp01 * tp01 * rp12;

    //phase difference between outer and inner reflections
    float dy = d1 * n1;
    float dx = tan(theta1RAD) * dy;
    float delay = sqrt(dx * dx + dy * dy);
    float cosRelPhase = cos(4 * PI / lambda * (delay - dx * sinTheta0));

    //add up sines of different phase and amplitude
    float outS2 = rs01 * rs01 + ris * ris + 2 * rs01 * ris * cosRelPhase;
    float outP2 = rp01 * rp01 + rip * rip + 2 * rp01 * rip * cosRelPhase;

    //reflectivity
    return (outS2 + outP2) / 2;
}

void makeRayInvisible(inout Ray r)
{
    r.pos = 0;
    r.drawInfo.a = 0; //mark up invalid ray
}

float sqrtNtime(float val, float N)
{
    for (int i = 0; i < N; i++)
    {
        val = sqrt(val);
    }
    return val;
}

void refIndexCorrect(inout float n, float lambda)
{
    const float lambda0 = (LAMBDA_RED + LAMBDA_BLUE) / 2;
    float coef = sqrtNtime(lambda0 / lambda, 6); //if lambda smaler, n is larger
            
    if (n != 1)
    {
        n *= coef;
    }
}

void computeTracedRay(inout Ray r, float lambda, int2 bounces)
{
    const int MAX_LensID_Diff = bounces.x + (bounces.x - bounces.y) + (computeConstants.numInterfaces - bounces.y) - 1;
    int bounceID = 0;
    int lensIDStep = 1;
    int currentLensID = 1;
    
    const float ep = 0.00001f;
    
    LensInterface F;
    int k;
    
    for (k = 0; k < MAX_LensID_Diff; k++, currentLensID += lensIDStep)
    {
        F = lensInterface[currentLensID];

        bool isReflect = (currentLensID == bounces[bounceID]) ? true : false;
        if (isReflect)
        {
            lensIDStep *= -1; //invert direction
            bounceID++;
        }
        
        if (bounceID > MAX_LensID_Diff)
        {
            break;
        }

        Intersection i = getIntersectionInfo(r, lensInterface[currentLensID]);
        
		[branch]
        if (!i.hit)
        {
            makeRayInvisible(r);
            break; // exit upon miss
        }

		// record texture coord . or max. rel . radius
        if (!F.flat)
            r.drawInfo.z = max(r.drawInfo.z, length(i.pos.xy) / F.sa);
        else if (currentLensID == computeConstants.apertureIndex) // iris aperture plane
            r.drawInfo.xy = i.pos.xy / computeConstants.apertureRadius; // update ray lightDir and position

        r.dir = normalize(i.pos - r.pos);

        if (i.inverted)
            r.dir *= -1.f; // correct an inverted ray
		
        r.pos = i.pos;

		// skip reflection / refraction for flat surfaces
        if (F.flat)
            continue;

		// do reflection / refraction for sphere surfaces
        float n0 = r.dir.z < 0.f ? F.n.x : F.n.z;
        float n2 = r.dir.z < 0.f ? F.n.z : F.n.x;
        
#ifdef AR_CORTING
        float n1 = max(sqrt(n0 * n2), 1.38);
#else
        float n1 = F.n.y;
#endif

        [branch]
        if (!isReflect)
        { // refraction
            
            float refraction_nIN = n0;
            float refraction_nOUT = n2;
            
            {//ref idx correction by lambda
                refIndexCorrect(refraction_nIN, lambda);
                refIndexCorrect(refraction_nOUT, lambda);
            }
            
            r.dir = refract(r.dir, i.norm, refraction_nIN / refraction_nOUT);
			[branch]
            if (length(r.dir) < ep)//in the theory, we must use "equal", but, in this case we ease condition because of affection 
            {
                makeRayInvisible(r);
                break; // total reflection
            }
        }
        else
        { 
            r.dir = reflect(r.dir, i.norm);
            r.drawInfo.a *= FresnelAR(i.theta, lambda, F.d1, n0, n1, n2); // update ray intensity
        }
    }
    
    const float invisibleRef = 0;
    [branch]
    if (k < MAX_LensID_Diff || r.drawInfo.a < invisibleRef)
    {
        makeRayInvisible(r);
    }
}

uint GetOffsetAtOneGhost(int2 pos)
{
    return pos.x + pos.y * GRID_DIV;
}

float quadSurface(float a, float b, float c, float d, float thetaRAD1, float thetaRAD2)
{
    float s = (a + b + c + d) / 2;
    float ccos = cos((thetaRAD1 + thetaRAD2) / 2);
    ccos *= ccos;
    return sqrt((s - a) * (s - b) * (s - c) * (s - d) - a * b * c * d * ccos);
}

float theta(float2 v1, float2 v2)
{
    return acos(dot(v1, v2) / (length(v1) * length(v2)));
}

float GetRegion(int2 rayID2D, int offsetAtBuffer)
{
    //↓ray pos
    //A B C
    //D E F
    //G H I
    
    float4 posA = traceResult[GetOffsetAtOneGhost(rayID2D + int2(-1, 1)) + offsetAtBuffer].pos;
    float4 posB = traceResult[GetOffsetAtOneGhost(rayID2D + int2(0, 1)) + offsetAtBuffer].pos;
    float4 posC = traceResult[GetOffsetAtOneGhost(rayID2D + int2(1, 1)) + offsetAtBuffer].pos;
    float4 posD = traceResult[GetOffsetAtOneGhost(rayID2D + int2(-1, 0)) + offsetAtBuffer].pos;
    float4 posE = traceResult[GetOffsetAtOneGhost(rayID2D + int2(0, 0)) + offsetAtBuffer].pos;
    float4 posF = traceResult[GetOffsetAtOneGhost(rayID2D + int2(1, 0)) + offsetAtBuffer].pos;
    float4 posG = traceResult[GetOffsetAtOneGhost(rayID2D + int2(-1, -1)) + offsetAtBuffer].pos;
    float4 posH = traceResult[GetOffsetAtOneGhost(rayID2D + int2(0, -1)) + offsetAtBuffer].pos;
    float4 posI = traceResult[GetOffsetAtOneGhost(rayID2D + int2(1, -1)) + offsetAtBuffer].pos;

    float2 vecDA = posD.xy - posA.xy, vecDE = posD.xy - posE.xy;
    float2 vecBA = posB.xy - posA.xy, vecBE = posB.xy - posE.xy;
    float2 vecEB = posE.xy - posB.xy, vecEF = posE.xy - posF.xy;
    float2 vecCB = posC.xy - posB.xy, vecCF = posC.xy - posF.xy;
    float2 vecGD = posG.xy - posD.xy, vecGH = posG.xy - posH.xy;
    float2 vecED = posE.xy - posD.xy, vecEH = posE.xy - posH.xy;
    float2 vecHE = posH.xy - posE.xy, vecHI = posH.xy - posI.xy;
    float2 vecFE = posF.xy - posE.xy, vecFI = posF.xy - posI.xy;
    
    float ab = length(vecBA), bc = length(vecCB), ad = length(vecDA), be = length(vecBE);
    float cf = length(vecCF), de = length(vecDE), ef = length(vecEF), dg = length(vecGD);
    float eh = length(vecEH), fi = length(vecFI), gh = length(vecGH), hi = length(vecHI);
    
    const float ep = 0.00001;
    float Region = quadSurface(ab, bc, ad, be, theta(vecDA, vecDE), theta(vecBA, vecBE)) * (!(rayID2D.x == 0) && !(rayID2D.y == (GRID_DIV - 1)));
    Region += quadSurface(bc, be, cf, ef, theta(vecEB, vecEF), theta(vecCB, vecCF)) * (!(rayID2D.x == (GRID_DIV - 1)) && !(rayID2D.y == (GRID_DIV - 1)));
    Region += quadSurface(dg, gh, de, eh, theta(vecGD, vecGH), theta(vecED, vecEH)) * (!(rayID2D.x == 0) && !(rayID2D.y == 0));
    Region += quadSurface(eh, hi, ef, fi, theta(vecHE, vecHI), theta(vecFE, vecFI)) * (!(rayID2D.x == (GRID_DIV - 1)) && !(rayID2D.y == 0));
    Region += ep;
    
    float BaseGridWidth = computeConstants.spread / (float) GRID_DIV;
    float BaseRegion = BaseGridWidth * BaseGridWidth;

    const float intensity = BaseRegion / Region;
    
    return isnan(intensity) ? 0.f : intensity;
}

InfoPerLambda Trace(float2 origin, float wavelength, int2 bounces)
{
    float3 originPos = float3(origin * computeConstants.spread, 1000.f);
    
    Ray originRay = { originPos, float3(0, 0, -1.f), 0.xxxx };
    Intersection i = getIntersectionInfo(originRay, lensInterface[0]);
    originPos = i.pos - computeConstants.lightDir.xyz;

    //first intersected ray
    Ray ray = { originPos, computeConstants.lightDir.xyz, float4(0.xxx, 1) };
    computeTracedRay(ray, wavelength, bounces);

    InfoPerLambda result;
    result.pos = float4(ray.pos.xyz, 1.f);
    result.coordinates = float4(origin, ray.drawInfo.xy);
    result.drawInfo = ray.drawInfo;
    result.reflectance = float4(0.xxx, ray.drawInfo.a);

    return result;
}

float3 lambda2RGB(float lambda)
{
    float3 colRGB;

    if (lambda < 350.0)
        colRGB = float3(0.5, 0.0, 1.0);
    else if ((lambda >= 350.0) && (lambda < 440.0))
        colRGB = float3((440.0 - lambda) / 90.0, 0.0, 1.0);
    else if ((lambda >= 440.0) && (lambda <= 490.0))
        colRGB = float3(0.0, (lambda - 440.0) / 50.0, 1.0);
    else if ((lambda >= 490.0) && (lambda < 510.0))
        colRGB = float3(0.0, 1.0, (-(lambda - 510.0)) / 20.0);
    else if ((lambda >= 510.0) && (lambda < 580.0))
        colRGB = float3((lambda - 510.0) / 70.0, 1.0, 0.0);
    else if ((lambda >= 580.0) && (lambda < 645.0))
        colRGB = float3(1.0, (-(lambda - 645.0)) / 65.0, 0.0);
    else
        colRGB = float3(1.0, 0.0, 0.0);

    if (lambda < 350.0)
        colRGB *= 0.3;
    else if ((lambda >= 350.0) && (lambda < 420.0))
        colRGB *= 0.3 + (0.7 * ((lambda - 350.0) / 70.0));
    else if ((lambda >= 420.0) && (lambda <= 700.0))
        colRGB *= 1.0;
    else if ((lambda > 700.0) && (lambda <= 780.0))
        colRGB *= 0.3 + (0.7 * ((780.0 - lambda) / 80.0));
    else
        colRGB *= 0.3;

    return colRGB;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void rayTraceCS(int3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    //offset at buffer
    int ghostID = groupID.x / NUM_GROUPS;
    int ghostOffset = ghostID * GRID_DIV * GRID_DIV;
	
    //offset at ghost
    int2 rayID2D = groupThreadID.xy + (groupID.xy % NUM_GROUPS) * NUM_THREADS; //(0～GRID_DIV - 1, 0～GRID_DIV - 1)
    float2 ray2DPos = (rayID2D / float(GRID_DIV - 1) - 0.5f) * 2.f; //-1～1
    
    const float range = LAMBDA_RED - LAMBDA_BLUE;
    
    int COLOR_ID = groupID.z;
    const float sampleLambda = LAMBDA_RED - COLOR_ID * range / (float) (SAMPLE_LAMBDA_NUM);
    
    int2 bounces = int2(ghostData[ghostID].bounce1, ghostData[ghostID].bounce2);
    InfoPerLambda result = Trace(ray2DPos, sampleLambda, bounces);
	
    uint ID = GetOffsetAtOneGhost(rayID2D) + ghostOffset;
    
    float3 sampleLambdaCol = lambda2RGB(sampleLambda);

    if (COLOR_ID == 0)
    {
        traceResult[ID].pos = result.pos;
        traceResult[ID].drawInfo = float4(result.drawInfo.rgb, GetRegion(rayID2D, ghostOffset));
    }
    traceResult[ID].coordinates[COLOR_ID] = result.coordinates;
    traceResult[ID].color[COLOR_ID].rgb = result.reflectance.a * sampleLambdaCol;
}
