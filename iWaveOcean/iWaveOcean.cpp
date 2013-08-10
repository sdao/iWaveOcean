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

#define PBLOCK_REF	0

#define MIN_WIDTH 1.0f
#define MIN_LENGTH 1.0f

class iWaveOceanClassDesc : public ClassDesc2 
{
public:
    virtual int IsPublic() 							{ return TRUE; }
    virtual void* Create(BOOL /*loading = FALSE*/)  { return new iWaveOcean(); }
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

static ParamBlockDesc2 iwaveocean_param_blk ( iwaveocean_params, _T("params"),  0, GetiWaveOceanDesc(), 
    P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
    //rollout
    IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
    // params
    pb_width,			_T("width"),		TYPE_FLOAT,		0,          		IDS_WIDTH,
        p_default,		100.0f,
        p_range,		MIN_WIDTH,10000.0f,
        p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_WIDTH_EDIT,		IDC_WIDTH_SPIN,			0.1f,
        p_end,
    pb_length,			_T("length"),		TYPE_FLOAT,		0,          		IDS_LENGTH,
        p_default,		100.0f,
        p_range,	    MIN_LENGTH,10000.0f,
        p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_LENGTH_EDIT,	IDC_LENGTH_SPIN,		0.1f,
        p_end,
    pb_width_segs,      _T("width_segs"),   TYPE_FLOAT,     0,                  IDS_WIDTH_SEGS,
        p_default,      100.0f,
        p_range,        10.0f, 1000.0f,
        p_end,
    pb_length_segs,     _T("length_segs"),  TYPE_FLOAT,     0,                  IDS_LENGTH_SEGS,
        p_default,      100.0f,
        p_range,        10.0f, 1000.0f,
        p_end,
    pb_sim_start,       _T("sim_start"),    TYPE_FLOAT,     0,                  IDS_START_FRAME,
        p_default,      0.0f,
        p_end,
    pb_sim_length,      _T("sim_length"),   TYPE_FLOAT,     0,                  IDS_FRAME_COUNT,
        p_default,      100.0f,
        p_end,
    pb_collision_objs,  _T("sim_collision"),TYPE_INODE_TAB, 0,  0,              IDS_COLLISION_OBJS,
        p_ui,           TYPE_NODELISTBOX,   IDC_COLLISION_LIST,IDC_COLLISION_PICK_BUTTON,0,IDC_COLLISION_DELETE_BUTTON,
        p_sclassID,     GEOMOBJECT_CLASS_ID,
        p_end,
    p_end
    );




//--- iWaveOcean -------------------------------------------------------

IObjParam* iWaveOcean::ip = NULL;

iWaveOcean* iWaveOcean::instance = NULL;

ICustButton* iWaveOcean::simButton = NULL;
HWND iWaveOcean::startFrameStatic = NULL;
HWND iWaveOcean::numFramesStatic = NULL;

iWaveOcean::iWaveOcean() : _sim(this)
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

    _simulateRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_SIMULATE), SimulateRollupDlgProc, GetString(IDS_SIMULATE_ROLLUP), (LPARAM)this);
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

INT_PTR CALLBACK iWaveOcean::SimulateRollupDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    { // Respond to the message ...
    case WM_INITDIALOG: // Initialize the Controls here.
        instance = (iWaveOcean*)lParam;

        simButton = GetICustButton(GetDlgItem(hDlg, IDC_SIMULATE_BUTTON));
        startFrameStatic = GetDlgItem(hDlg, IDC_STARTFRAME_STATIC);
        numFramesStatic = GetDlgItem(hDlg, IDC_NUMFRAMES_STATIC);

        UpdateStatus();
        return TRUE;
    case WM_DESTROY: // Release the Controls here.
        ReleaseICustButton(simButton);
        
        instance = NULL;
        simButton = NULL;
        startFrameStatic = NULL;
        numFramesStatic = NULL;
        return FALSE;
    case WM_COMMAND: // Various messages come in this way.
        switch (LOWORD(wParam)) {
        case IDC_SIMULATE_BUTTON:
            instance->_sim.BeginSimulation(hDlg); // Waits for dialog to exit.
            UpdateStatus();
            break;
        }
        break;
    case WM_NOTIFY: // Others this way...
        break;
        // Other cases...
    default:
        break;
    }
    return FALSE;
}

void iWaveOcean::UpdateStatus()
{
    if (instance != NULL) {
        if (startFrameStatic != NULL)
        {
            SetWindowTextInt(startFrameStatic, instance->_sim.GetSimulatedStartFrame());
        }

        if (numFramesStatic != NULL)
        {
            SetWindowTextInt(numFramesStatic, instance->_sim.GetSimulatedFrameCount());
        }
    }
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
    ivalid = Interval(t, t + GetTicksPerFrame() - 1); // Validity interval of one frame.
    
    int frameNumber = t / GetTicksPerFrame(); // ticks * frames/ticks = frames; depends on framerate

    Grid *frameData = _sim.GetSimulatedGrid(frameNumber);
    Point3 *vertices = frameData->GetVertices();

    int faces_x = frameData->GetWidthSegs();
    int faces_y = frameData->GetLengthSegs();
    int vertices_x = frameData->GetWidthVertices();
    int vertices_y = frameData->GetLengthVertices();
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
