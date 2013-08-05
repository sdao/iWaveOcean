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
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "iWaveOcean.h"
#include "Simulator.h"
#include <vector>

#define iWaveOcean_CLASS_ID	Class_ID(0xd9edfdd9, 0x93776fa5)

#define PBLOCK_REF	0

#define MIN_WIDTH 1.0f
#define MIN_LENGTH 1.0f

class iWaveOcean : public SimpleObject2
{
public:
    //Constructor/Destructor
    iWaveOcean();
    virtual ~iWaveOcean();

    // Simulation cache
    std::vector<Point3*> _cache;
 
    HWND _simulateRollup;
    static iWaveOcean* instance;
    static INT_PTR CALLBACK SimulateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

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

    // From Animatable
    virtual void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
    virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

    // From SimpleObject
    virtual void BuildMesh(TimeValue t);
    virtual void BuildPlaneMesh(Point3* vertices, int faces_x, int faces_y);
    virtual void InvalidateUI();

    //From Animatable
    virtual Class_ID ClassID() {return iWaveOcean_CLASS_ID;}
    virtual SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }
    virtual void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

    virtual RefTargetHandle Clone( RemapDir& remap );

    virtual int NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
    virtual IParamBlock2* GetParamBlock(int /*i*/) { return pblock2; } // return i'th ParamBlock
    virtual IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

    void DeleteThis() { delete this; }
};



class iWaveOceanClassDesc : public ClassDesc2 
{
public:
    virtual int IsPublic() 							{ return TRUE; }
    virtual void* Create(BOOL /*loading = FALSE*/) 		{ return new iWaveOcean(); }
    virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
    virtual SClass_ID SuperClassID() 				{ return GEOMOBJECT_CLASS_ID; }
    virtual Class_ID ClassID() 						{ return iWaveOcean_CLASS_ID; }
    virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

    virtual const TCHAR* InternalName() 			{ return _T("iWaveOcean"); }	// returns fixed parsable name (scripter-visible name)
    virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
    

};


ClassDesc2* GetiWaveOceanDesc() { 
    static iWaveOceanClassDesc iWaveOceanDesc;
    return &iWaveOceanDesc; 
}





enum { iwaveocean_params };


//TODO: Add enums for various parameters
enum { 
    pb_width,
    pb_length
};




static ParamBlockDesc2 iwaveocean_param_blk ( iwaveocean_params, _T("params"),  0, GetiWaveOceanDesc(), 
    P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
    //rollout
    IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
    // params
    pb_width,			_T("width"),		TYPE_FLOAT,		P_ANIMATABLE,		IDS_WIDTH,
        p_default,		100.0f,
        p_range,		MIN_WIDTH,10000.0f,
        p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_WIDTH_EDIT,		IDC_WIDTH_SPIN,			0.1f,
        p_end,
    pb_length,			_T("length"),		TYPE_FLOAT,		P_ANIMATABLE,		IDS_LENGTH,
        p_default,		100.0f,
        p_range,	    MIN_LENGTH,10000.0f,
        p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_LENGTH_EDIT,	IDC_LENGTH_SPIN,		0.1f,
        p_end,
    p_end
    );




//--- iWaveOcean -------------------------------------------------------

IObjParam* iWaveOcean::ip = NULL;
iWaveOcean* iWaveOcean::instance = NULL;

iWaveOcean::iWaveOcean() : _cache()
{
    GetiWaveOceanDesc()->MakeAutoParamBlocks(this);
}

iWaveOcean::~iWaveOcean()
{
}

void iWaveOcean::BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev)
{
    this->ip = ip;

    SimpleObject2::BeginEditParams(ip,flags,prev);
    GetiWaveOceanDesc()->BeginEditParams(ip, this, flags, prev);

    _simulateRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_SIMULATE), SimulateDlgProc, GetString(IDS_SIMULATE_ROLLUP), (LPARAM)this);
}

void iWaveOcean::EndEditParams( IObjParam* ip, ULONG flags, Animatable* next )
{
    ip->DeleteRollupPage(_simulateRollup);
    _simulateRollup = NULL;

    //TODO: Save plugin parameter values into class variables, if they are not hosted in ParamBlocks. 
    SimpleObject2::EndEditParams(ip,flags,next);
    GetiWaveOceanDesc()->EndEditParams(ip, this, flags, next);

    this->ip = NULL;
}

INT_PTR CALLBACK iWaveOcean::SimulateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    { // Respond to the message ...
    case WM_INITDIALOG: // Initialize the Controls here.
        instance = (iWaveOcean*)lParam;
        return TRUE;
    case WM_DESTROY: // Release the Controls here.
        instance = NULL;
        return FALSE;
    case WM_COMMAND: // Various messages come in this way.
        break;
    case WM_NOTIFY: // Others this way...
        break;
        // Other cases...
    default:
        break;
    }
    return FALSE;
}

//From Object
BOOL iWaveOcean::HasUVW() 
{ 
    //TODO: Return whether the object has UVW coordinates or not
    return TRUE; 
}

void iWaveOcean::SetGenUVW(BOOL sw) 
{
    if (sw==HasUVW()) 
        return;
    //TODO: Set the plugin's internal value to sw
}

//Class for interactive creation of the object using the mouse
class iWaveOceanCreateCallBack : public CreateMouseCallBack {
    IPoint2 sp0;              //First point in screen coordinates
    iWaveOcean* ob; //Pointer to the object 
    Point3 p0;                //First point in world coordinates
    Point3 p1;                //Second point in world coordinates
public:	
    int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
    void SetObj(iWaveOcean *obj) {ob = obj;}
};

int iWaveOceanCreateCallBack::proc(ViewExp *vpt,int msg, int point, int /*flags*/, IPoint2 m, Matrix3& mat )
{
    if ( ! vpt || ! vpt->IsAlive() )
    {
        // why are we here
        DbgAssert(!_T("Invalid viewport!"));
        return FALSE;
    }

    if (msg == MOUSE_POINT || msg == MOUSE_MOVE) {
        switch(point) {
            case 0: // only happens with MOUSE_POINT msg
                ob->suspendSnap = TRUE;
                sp0 = m;
                p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
                mat.SetTrans(p0);

                //Set a default overall size in the parameter block
                ob->pblock2->SetValue(pb_width, ob->ip->GetTime(), 0.0f);
                ob->pblock2->SetValue(pb_length, ob->ip->GetTime(), 0.0f);
                break;
            case 1:
            {
                ob->suspendSnap = TRUE; 
                p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
                Point3 diff = p1 - p0;
                mat.SetTrans(p0 + diff / 2.0f);
                
                //Set the overall size in parameter block
                ob->pblock2->SetValue(pb_width, ob->ip->GetTime(), abs(diff.x));
                ob->pblock2->SetValue(pb_length, ob->ip->GetTime(), abs(diff.y));

                //Invalidate and display the mesh in the viewport
                iwaveocean_param_blk.InvalidateUI();
                break;
            }
            case 2: // happens when user releases mouse
            {
                ob->suspendSnap = TRUE; 
                p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
                Point3 diff = p1 - p0;
                
                if (abs(diff.x) < MIN_WIDTH || abs(diff.y) < MIN_LENGTH)
                {
                    return CREATE_ABORT; // abort if the size is too small
                }
                else
                {
                    return CREATE_STOP;
                }
            }
        }
    } else {
        if (msg == MOUSE_ABORT) return CREATE_ABORT;
    }

    return TRUE;
}

static iWaveOceanCreateCallBack iWaveOceanCreateCB;

//From BaseObject
CreateMouseCallBack* iWaveOcean::GetCreateMouseCallBack() 
{
    iWaveOceanCreateCB.SetObj(this);
    return(&iWaveOceanCreateCB);
}

//From SimpleObject
void iWaveOcean::BuildMesh(TimeValue t)
{
    //TODO: Implement the code to build the mesh representation of the object
    //      using its parameter settings at the time passed. The plug-in should 
    //      use the data member mesh to store the built mesh.
    //      SimpleObject ivalid member should be updated. This can be done while
    //      retrieving values from pblock2.
    ivalid = FOREVER;

    float width, length;
    pblock2->GetValue(pb_width, t, width, ivalid); // width = plane X width
    pblock2->GetValue(pb_length, t, length, ivalid); // length = plane Y length

    int faces_x = 10;
    int faces_y = 10;
    
    Point3 *frameData;

    if (_cache.empty())
    {
        frameData = new Point3[(faces_x + 1) * (faces_y + 1)];
        Simulator::GetPlane(width, length, 10, 10, frameData);

        this->BuildPlaneMesh(frameData, faces_x, faces_y);

        delete [] frameData;
    }
    else
    {
        int frameNumber = t / GetTicksPerFrame(); // ticks * frames/ticks = frames; depends on framerate 
        frameNumber = max(frameNumber, 0);
        frameNumber = min(frameNumber, _cache.size() - 1);

        frameData = _cache[frameNumber];

        this->BuildPlaneMesh(frameData, faces_x, faces_y);
    }
}

void iWaveOcean::BuildPlaneMesh(Point3* vertices, int faces_x, int faces_y)
{
    int vertices_x = faces_x + 1;
    int vertices_y = faces_y + 1;
    int numVerts = vertices_x * vertices_y;
    int numFaces = faces_x * faces_y * 2; // Double number of quads to make tris.

    mesh.setNumVerts(numVerts);
    mesh.setNumFaces(numFaces);
    mesh.setNumTVerts(numVerts);
    mesh.setNumTVFaces(numFaces);

    int vtx = 0;
    float max_u = (float)faces_x, max_v = (float)faces_y;
    for (int i = 0; i < vertices_x; i++)
    {
        for (int j = 0; j < vertices_y; j++)
        {
            mesh.setVert(vtx, vertices[vtx]);
            mesh.setTVert(vtx, i / max_u, j / max_v, 0.0f);
            ++vtx;
        }
    }

    int face = 0;
    for (int i = 0; i < faces_x; i++)
    {
        for (int j = 0; j < faces_y; j++)
        {
            int pt1 = i * vertices_y + j;
            int pt2 = i * vertices_y + j + 1;
            int pt3 = (i+1) * vertices_y + j + 1;
            int pt4 = (i+1) * vertices_y + j;

            mesh.faces[face].setVerts(pt1, pt2, pt3);
            mesh.faces[face].setEdgeVisFlags(1, 1, 0);
            mesh.faces[face].setSmGroup(1);
            mesh.tvFace[face].setTVerts(pt1, pt2, pt3);
            ++face;

            mesh.faces[face].setVerts(pt3, pt4, pt1);
            mesh.faces[face].setEdgeVisFlags(1, 1, 0);
            mesh.faces[face].setSmGroup(1);
            mesh.tvFace[face].setTVerts(pt3, pt4, pt1);
            ++face;
        }
    }

    mesh.InvalidateGeomCache();
}

void iWaveOcean::InvalidateUI() 
{
    // Hey! Update the param blocks
    pblock2->GetDesc()->InvalidateUI();
}

Object* iWaveOcean::ConvertToType(TimeValue t, Class_ID obtype)
{
    //TODO: If the plugin can convert to a nurbs surface then check to see 
    //      whether obtype == EDITABLE_SURF_CLASS_ID and convert the object
    //      to nurbs surface and return the object
    //      If no special conversion is needed remove this implementation.
    
    return SimpleObject::ConvertToType(t,obtype);
}

int iWaveOcean::CanConvertToType(Class_ID obtype)
{
    //TODO: Check for any additional types the plugin supports
    //      If no special conversion is needed remove this implementation.
    return SimpleObject::CanConvertToType(obtype);
}

// From Object
int iWaveOcean::IntersectRay(TimeValue /*t*/, Ray& /*ray*/, float& /*at*/, Point3& /*norm*/)
{
    //TODO: Return TRUE after you implement this method
    return FALSE;
}

void iWaveOcean::GetCollapseTypes(Tab<Class_ID>& clist,Tab<TSTR*>& nlist)
{
    Object::GetCollapseTypes(clist, nlist);
    //TODO: Append any any other collapse type the plugin supports
}

// From ReferenceTarget
RefTargetHandle iWaveOcean::Clone(RemapDir& remap) 
{
    iWaveOcean* newob = new iWaveOcean();	
    //TODO: Make a copy all the data and also clone all the references
    newob->ReplaceReference(0,remap.CloneRef(pblock2));
    newob->ivalid.SetEmpty();
    BaseClone(this, newob, remap);
    return(newob);
}
