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
#include "ExternalFile.h"
#include <Windowsx.h>
#include <vector>
#include <limits>
#include <sstream>

#define PBLOCK_REF 0
#define SIM_DATA_CHUNK_V1 1000 
#define SIM_DATA_CHUNK_V2 1001

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

enum { pb_map_size, pb_map_dynamics, pb_map_ambient };

static ParamBlockDesc2 iwaveocean_param_blk ( iwaveocean_params, _T("params"),  0, GetiWaveOceanDesc(), 
    P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, PBLOCK_REF, 
    // multiple rollouts
    3,
    pb_map_size, IDD_PANEL_SIZE, IDS_PARAMS_SIZE, 0, 0, NULL,
    pb_map_ambient, IDD_PANEL_AMBIENT, IDS_PARAMS_AMBIENT, 0, 0, NULL,
    pb_map_dynamics, IDD_PANEL_DYNAMICS, IDS_PARAMS_DYNAMICS, 0, 0, NULL,
    // params
    pb_width,			_T("width"),		    TYPE_FLOAT,		0,          		IDS_WIDTH,
        p_default,		100.0f,
        p_range,		MIN_WIDTH,1000000.0f,
        p_ui,			pb_map_size,            TYPE_SPINNER,   EDITTYPE_UNIVERSE,  IDC_WIDTH_EDIT,		    IDC_WIDTH_SPIN,			0.1f,
        p_end,
    pb_length,			_T("length"),		    TYPE_FLOAT,		0,          		IDS_LENGTH,
        p_default,		100.0f,
        p_range,	    MIN_LENGTH,1000000.0f,
        p_ui,			pb_map_size,            TYPE_SPINNER,   EDITTYPE_UNIVERSE,	IDC_LENGTH_EDIT,	    IDC_LENGTH_SPIN,		0.1f,
        p_end,
    pb_width_segs,      _T("width_segs"),       TYPE_INT,       0,                  IDS_WIDTH_SEGS,
        p_default,      100,
        p_range,        10, 1000,
        p_ui,			pb_map_size,            TYPE_SPINNER,	EDITTYPE_INT,	    IDC_WIDTH_SEGS_EDIT,    IDC_WIDTH_SEGS_SPIN,    0.05f,
        p_end,
    pb_length_segs,     _T("length_segs"),      TYPE_INT,       0,                  IDS_LENGTH_SEGS,
        p_default,      100,
        p_range,        10, 1000,
        p_ui,			pb_map_size,            TYPE_SPINNER,   EDITTYPE_INT,	    IDC_LENGTH_SEGS_EDIT,   IDC_LENGTH_SEGS_SPIN,	0.05f,
        p_end,


    pb_sim_start,       _T("sim_start"),        TYPE_INT,       0,                  IDS_START_FRAME,
        p_default,      SecToTicks(0),
        p_range,        INT_MIN, INT_MAX,
        p_ui,           pb_map_dynamics,        TYPE_SPINNER,   EDITTYPE_TIME,      IDC_START_EDIT,         IDC_START_SPIN,         (float)GetTicksPerFrame(),
        p_end,
    pb_sim_length,      _T("sim_length"),       TYPE_INT,       0,                  IDS_FRAME_COUNT,
        p_default,      SecToTicks(5),
        p_range,        SecToTicks(1), INT_MAX,
        p_ui,           pb_map_dynamics,        TYPE_SPINNER,   EDITTYPE_TIME,      IDC_FRAMES_EDIT,        IDC_FRAMES_SPIN,        (float)GetTicksPerFrame(),
        p_end,
    pb_collision_objs,  _T("sim_collision"),    TYPE_INODE_TAB, 0,  0,              IDS_COLLISION_OBJS,
        p_ui,           pb_map_dynamics,        TYPE_NODELISTBOX,IDC_COLLISION_LIST,IDC_COLLISION_PICK_BUTTON,0,IDC_COLLISION_DELETE_BUTTON,
        p_sclassID,     GEOMOBJECT_CLASS_ID,
        p_end,
    pb_wave_damping,    _T("wave_damping"),     TYPE_FLOAT,     0,                  IDS_WAVE_DAMPING,
        p_default,      0.3f,
        p_range,        0.01f, 1.00f,
        p_ui,           pb_map_dynamics,        TYPE_SPINNER,   EDITTYPE_FLOAT,     IDC_DAMPING_EDIT,       IDC_DAMPING_SPIN,       0.1f,
        p_end,
    pb_collision_smoothing,_T("collision_smooth"),TYPE_FLOAT,   0,                  IDS_COLLISION_SMOOTHING,
        p_default,      1.00f,
        p_range,        0.01f, 10.00f,
        p_ui,           pb_map_dynamics,        TYPE_SPINNER,   EDITTYPE_FLOAT,     IDC_SMOOTHING_EDIT,     IDC_SMOOTHING_SPIN,     0.01f,
        p_end,
    pb_wake_power,      _T("wake_pow"),         TYPE_FLOAT,     0,                  IDS_WAKE_POWER,
        p_default,      1.00f,
        p_range,        1.00f, 5.00f,
        p_ui,           pb_map_dynamics,        TYPE_SPINNER,   EDITTYPE_FLOAT,     IDC_WAKE_POWER_EDIT,    IDC_WAKE_POWER_SPIN,    0.01f,
        p_end,
    pb_height_scale,    _T("height_scale"),     TYPE_FLOAT,     0,                  IDS_HEIGHT_SCALE,
        p_default,      1.00f,
        p_range,        0.00f, 10.00f,
        p_ui,           pb_map_dynamics,        TYPE_SPINNER,   EDITTYPE_FLOAT,     IDC_HEIGHT_EDIT,        IDC_HEIGHT_SPIN,        0.01f,
        p_end,


    pb_ambient_on,      _T("make_ambient"),     TYPE_BOOL,      0,                  IDS_AMBIENT_ON,
        p_default,      TRUE,
        p_ui,           pb_map_ambient,         TYPE_SINGLECHEKBOX,IDC_AMBIENT_CHECK,
        p_end,
    pb_ambient_scale,   _T("ambient_scale"),	TYPE_FLOAT,		P_ANIMATABLE,		IDS_AMBIENT_SCALE,
        p_default,		2000.0f,
        p_range,		MIN_WIDTH,500000.0f,
        p_ui,			pb_map_ambient,         TYPE_SPINNER,   EDITTYPE_UNIVERSE,  IDC_SCALE_EDIT,		    IDC_SCALE_SPIN,			0.1f,
        p_end,
    pb_ambient_height,	_T("ambient_height"),   TYPE_FLOAT, 	P_ANIMATABLE, 		IDS_AMPLITUDE, 
        p_default, 		1.0f, 
        p_range, 		0.00f,100.00f, 
        p_ui, 			pb_map_ambient,         TYPE_SPINNER,	EDITTYPE_UNIVERSE,  IDC_AMPLITUDE_EDIT,	    IDC_AMPLITUDE_SPIN,		0.01f, 
        p_end,
    pb_min_wave_size,	_T("min_wave_size"),    TYPE_FLOAT,		P_ANIMATABLE,		IDS_MIN_SIZE,
        p_default,		10.0f,
        p_range,		0.0f,2000.0f,
        p_ui,			pb_map_ambient,         TYPE_SPINNER,	EDITTYPE_UNIVERSE,  IDC_MIN_SIZE_EDIT,	    IDC_MIN_SIZE_SPIN,		0.1f,
        p_end,
    pb_wind_speed,		_T("wind_speed"),	    TYPE_FLOAT,		P_ANIMATABLE,		IDS_SPEED,
        p_default,		150.0f,
        p_range,		0.0f,1000.0f,
        p_ui,			pb_map_ambient,         TYPE_SPINNER,	EDITTYPE_UNIVERSE,	IDC_SPEED_EDIT,		    IDC_SPEED_SPIN,			0.1f,
        p_end,
    pb_wind_direction,	_T("wind_dir"),		    TYPE_ANGLE,		P_ANIMATABLE,		IDS_DIRECTION,
        p_default,		0.0f,
        p_ui,			pb_map_ambient,         TYPE_SPINNER,	EDITTYPE_FLOAT,	    IDC_DIRECTION_EDIT,	    IDC_DIRECTION_SPIN,		1.0f,
        p_end,
    pb_seed,			_T("seed"),			    TYPE_INT,		0,		            IDS_SEED,
        p_default,		1,
        p_range,		1,INT_MAX,
        p_ui,			pb_map_ambient,         TYPE_SPINNER,   EDITTYPE_INT,	    IDC_SEED_EDIT,		    IDC_SEED_SPIN,			0.05f,
        p_end,
    pb_duration,		_T("duration"),	        TYPE_INT,        0,          		IDS_DURATION,
        p_default,		SecToTicks(600), // 10 min
        p_range,		SecToTicks(1),SecToTicks(60000), // 1000 min
        p_ui,			pb_map_ambient,         TYPE_SPINNER,	EDITTYPE_TIME,	    IDC_DURATION_EDIT,      IDC_DURATION_SPIN,  	(float)GetTicksPerFrame(),
        p_end,

    p_end
    );




//--- iWaveOcean -------------------------------------------------------

IObjParam* iWaveOcean::ip = NULL;

iWaveOcean* iWaveOcean::instanceForSimulate = NULL;
iWaveOcean* iWaveOcean::instanceForSaveData = NULL;

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
	_saveDataRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_SAVEDATA), SaveDataRollupDlgProc, GetString(IDS_SAVEDATA_ROLLUP), (LPARAM)this);
}

void iWaveOcean::EndEditParams( IObjParam* ip, ULONG flags, Animatable* next )
{
    ip->DeleteRollupPage(_simulateRollup);
    _simulateRollup = NULL;

	ip->DeleteRollupPage(_saveDataRollup);
	_saveDataRollup = NULL;

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
        instanceForSimulate = (iWaveOcean*)lParam;

        startFrameStatic = GetDlgItem(hDlg, IDC_STARTFRAME_STATIC);
        numFramesStatic = GetDlgItem(hDlg, IDC_NUMFRAMES_STATIC);

        UpdateStatus();
        return TRUE;
    case WM_DESTROY: // Release the Controls here.
        instanceForSimulate = NULL;
        startFrameStatic = NULL;
        numFramesStatic = NULL;
        return FALSE;
    case WM_COMMAND: // Various messages come in this way.
        switch (LOWORD(wParam)) {
        case IDC_SIMULATE_BUTTON:
            instanceForSimulate->_sim.BeginSimulation(hDlg); // Waits for dialog to exit.
            UpdateStatus();
            return TRUE;
        case IDC_CLEAR_BUTTON:
            instanceForSimulate->_sim.Reset();
            UpdateStatus();
			return TRUE;
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

INT_PTR CALLBACK iWaveOcean::SaveDataRollupDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    { // Respond to the message ...
    case WM_INITDIALOG: // Initialize the Controls here.
        instanceForSaveData = (iWaveOcean*)lParam;
		UpdateSaveInfo(hDlg);
        return TRUE;
    case WM_DESTROY: // Release the Controls here.
		instanceForSaveData = NULL;
        return FALSE;
    case WM_COMMAND: // Various messages come in this way.
        switch (LOWORD(wParam)) {
        case IDC_BROWSE_BUTTON:
            instanceForSaveData->_sim.BeginSelectExternalFile(hDlg);
			UpdateSaveInfo(hDlg);
			UpdateStatus();
            return TRUE;
		case IDC_RADIO_MAXFILE:
		case IDC_RADIO_EXTERNALFILE:
			{
				HWND radioMaxFile = GetDlgItem(hDlg, IDC_RADIO_MAXFILE);
				if (Button_GetCheck(radioMaxFile)) {
					instanceForSaveData->_sim.UseNativeStorage();
				} else {
					instanceForSaveData->_sim.BeginSelectExternalFile(hDlg);
				}
				UpdateSaveInfo(hDlg);
				UpdateStatus();
			}
			return TRUE;
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
    if (instanceForSimulate != NULL) {
        if (startFrameStatic != NULL)
        {
            SetWindowTextInt(startFrameStatic, instanceForSimulate->_sim.GetSimulatedStartFrame());
        }

        if (numFramesStatic != NULL)
        {
            SetWindowTextInt(numFramesStatic, instanceForSimulate->_sim.GetSimulatedFrameCount());
        }
    }
}

void iWaveOcean::UpdateSaveInfo(HWND hDlg)
{
	if (instanceForSaveData) {
		HWND radioMaxFile = GetDlgItem(hDlg, IDC_RADIO_MAXFILE);
		HWND radioExternalFile = GetDlgItem(hDlg, IDC_RADIO_EXTERNALFILE);
		HWND browseButton = GetDlgItem(hDlg, IDC_BROWSE_BUTTON);

		if (!instanceForSaveData->_sim.IsUsingExternalStorage()) {
			Button_SetCheck(radioMaxFile, BST_CHECKED);
			Button_SetCheck(radioExternalFile, BST_UNCHECKED);
			Button_Enable(browseButton, FALSE);
			Button_SetText(browseButton, _T("(no file)"));
		} else {
			Button_SetCheck(radioMaxFile, BST_UNCHECKED);
			Button_SetCheck(radioExternalFile, BST_CHECKED);
			Button_Enable(browseButton, TRUE);
			Button_SetText(browseButton, instanceForSaveData->_sim.GetExternalFileName().c_str());
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

    Grid* frameData = _sim.GetSimulatedGrid(frameNumber);
    float* vertexHeights = frameData->GetVertexHeights();

    int faces_x = frameData->GetWidthSegs();
    int faces_y = frameData->GetLengthSegs();
    int vertices_x = frameData->GetWidthVertices();
    int vertices_y = frameData->GetLengthVertices();
    int numVerts = vertices_x * vertices_y;
    int numFaces = faces_x * faces_y * 2; // Double number of quads to make tris.

    float width = frameData->GetWidth();
    float length = frameData->GetLength();
    float halfWidth = width / 2.0f;
    float halfLength = length / 2.0f;

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
            Point3 pt(i * width / faces_x - halfWidth, j * length / faces_y - halfLength, vertexHeights[vtx]);
            mesh.setVert(vtx, pt);
            mesh.setTVert(vtx, i / max_u, j / max_v, 0.0f);
            ++vtx;
        }
    }

    int face = 0;
    for (int i = 0; i < faces_x; i++)
    {
        for (int j = 0; j < faces_y; j++)
        {
            int pt1 = (i+1) * vertices_y + j;
            int pt2 = (i+1) * vertices_y + j + 1;
            int pt3 = i * vertices_y + j + 1;
            int pt4 = i * vertices_y + j;

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

IOResult iWaveOcean::Load(ILoad* iload)
{
    IOResult res;

    while (IO_OK == (res = iload->OpenChunk()))
    {
        switch(iload->CurChunkID())
        {
        case SIM_DATA_CHUNK_V2:
            res = _sim.Load(iload);
            break;
        }
        iload->CloseChunk();
        if (res != IO_OK) return res;
    }

    return IO_OK;
}

IOResult iWaveOcean::Save(ISave* isave)
{
    IOResult res;

    isave->BeginChunk(SIM_DATA_CHUNK_V2);
    res = _sim.Save(isave);
    if (res != IO_OK) return res;
    isave->EndChunk();

    return IO_OK;
}
