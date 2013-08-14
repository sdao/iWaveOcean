#pragma once

//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Includes for Plugins
// AUTHOR: 
//***************************************************************************/

#include "3dsmaxsdk_preinclude.h"
#include "resource.h"
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <maxtypes.h>
#include <Simpobj.h>
#include <mouseman.h>
//SIMPLE TYPE

#include <mouseman.h>
#include <triobj.h>

#include "Simulator.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define iWaveOcean_CLASS_ID	Class_ID(0xd9edfdd9, 0x93776fa5)

class iWaveOcean : public SimpleObject2
{
    // Simulation cache
    Simulator _sim;
    HWND _simulateRollup;

public:
    //Constructor/Destructor
    iWaveOcean();
    virtual ~iWaveOcean();

    // Static stuff for simulation cache
    static iWaveOcean* instance;

    static HWND startFrameStatic;
    static HWND numFramesStatic;

    static INT_PTR CALLBACK SimulateRollupDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static void UpdateStatus();
    static void SimulateProgress(Simulator* simulator);
    static void SimulateDone(Simulator* simulator);

    // Parameter block handled by parent
    static IObjParam *ip; // Access to the interface

    // From BaseObject
    virtual CreateMouseCallBack* GetCreateMouseCallBack();

    // From Object
    virtual BOOL HasUVW();
    virtual void SetGenUVW(BOOL sw);
    virtual int CanConvertToType(Class_ID obtype);
    virtual Object* ConvertToType(TimeValue t, Class_ID obtype);
    virtual void GetCollapseTypes(Tab<Class_ID>& clist,Tab<TSTR*>& nlist);
    virtual int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
    virtual BOOL IsWorldSpaceObject() { return TRUE; }

    // From Animatable
    virtual void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
    virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

    // From SimpleObject
    virtual void BuildMesh(TimeValue t);
    virtual void InvalidateUI();

    // Loading/Saving
    virtual IOResult Load(ILoad *iload);
    virtual IOResult Save(ISave *isave);

    //From Animatable
    virtual Class_ID ClassID() { return iWaveOcean_CLASS_ID; }
    virtual SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }
    virtual void GetClassName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
    virtual const wchar_t* GetObjectName() { return _M("iWave"); }

    virtual RefTargetHandle Clone( RemapDir& remap );

    virtual int NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
    virtual IParamBlock2* GetParamBlock(int /*i*/) { return pblock2; } // return i'th ParamBlock
    virtual IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

    void DeleteThis() { delete this; }
};

enum { iwaveocean_params };

enum { 
    pb_width,
    pb_length,
    pb_width_segs,
    pb_length_segs,
    pb_sim_start,
    pb_sim_length,
    pb_collision_objs,
    pb_wave_damping,
    pb_collision_smoothing,
    pb_wake_power,
    pb_height_scale
};