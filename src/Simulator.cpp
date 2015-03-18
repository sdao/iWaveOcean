#define _WIN32_WINNT _WIN32_WINNT_VISTA

#define CheckResult(res) if (res != IO_OK) return res;

#define FRIENDLY_ERRORS

#include "Simulator.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <process.h>
#include <units.h>
#include <Path.h>
#include "iWaveOcean.h"
#include "Ambient.h"
#include "Dynamics.h"

int Simulator::simStart;
int Simulator::simLength;
int Simulator::simCounter;

Simulator::Simulator(iWaveOcean* geom) : _cache(), _cacheStartFrame(0), _staticGrid(NULL), _geom(geom), _finished(true), _cancelled(false), _saveExternal(false), _saveExternalPath(L"")
{
}

Simulator::~Simulator(void)
{
    Reset();
}

void Simulator::DoWork(void* ptr)
{
    Simulator* instance = (Simulator*)ptr;
    iWaveOcean* modifier = instance->_geom;
    TimeValue ticksPerFrame = GetTicksPerFrame();

    instance->Reset();

    simStart = (int)((float)modifier->pblock2->GetInt(pb_sim_start, 0) / (float)ticksPerFrame);     // Round to nearest frame.
    simLength = (int)((float)modifier->pblock2->GetInt(pb_sim_length, 0) / (float)ticksPerFrame);   // Round to nearest # of frames.
    int widthSegs = modifier->pblock2->GetInt(pb_width_segs, 0);
    int lengthSegs = modifier->pblock2->GetInt(pb_length_segs, 0);
    float alpha = modifier->pblock2->GetFloat(pb_wave_damping, 0);
    float sigma = modifier->pblock2->GetFloat(pb_collision_smoothing, 0);
    float wakePower = modifier->pblock2->GetFloat(pb_wake_power, 0);
    float heightScale = modifier->pblock2->GetFloat(pb_height_scale, 0);
    float width = modifier->pblock2->GetFloat(pb_width, 0); // width = plane X width
    float length = modifier->pblock2->GetFloat(pb_length, 0); // length = plane Y length

    int collisionNodeCount = modifier->pblock2->Count(pb_collision_objs);
    INode** collisionNodes = new INode*[collisionNodeCount];
    for (int i = 0; i < collisionNodeCount; i++)
    {
        INode* n = modifier->pblock2->GetINode(pb_collision_objs, 0, i);
        collisionNodes[i] = n;
    }

    Ambient* amb = NULL;
    if (modifier->pblock2->GetInt(pb_ambient_on, 0))
    {
        int ambientSeed = modifier->pblock2->GetInt(pb_seed, 0);
        float ambientDuration = TicksToSec(modifier->pblock2->GetInt(pb_duration, 0));
        amb = new Ambient(width, length, widthSegs, lengthSegs, ambientSeed, ambientDuration, Ambient::GRAVITY_US); 
    }

    Dynamics dyn(simStart, width, length, widthSegs, lengthSegs, heightScale, TicksToSec(ticksPerFrame), alpha, sigma, wakePower, instance->_geom->GetWorldSpaceObjectNode(), collisionNodes, collisionNodeCount, amb);

    for (simCounter = simStart; simCounter < simStart + simLength; simCounter++)
    {
        if (instance->_cancelled)
        {
            break;
        }

        if (amb)
        {
            TimeValue t = simCounter * ticksPerFrame;
            amb->Simulate(TicksToSec(t),
                modifier->pblock2->GetFloat(pb_wind_speed, t),
                modifier->pblock2->GetFloat(pb_wind_direction, t),
                modifier->pblock2->GetFloat(pb_ambient_scale, t),
                modifier->pblock2->GetFloat(pb_min_wave_size, t),
                modifier->pblock2->GetFloat(pb_ambient_height, t));
        }

        Grid *data = dyn.NextGrid();
        instance->_cache.push_back(data);
    }

    delete [] collisionNodes;
    delete amb;
    instance->_cacheStartFrame = simStart;
    instance->_finished = true;
}

void Simulator::BeginSimulation(HWND hDlg)
{
    if (!_finished) {
        return;
    }

    if (IsTopmostModifier())
    {
        _cancelled = false;
        _finished = false;

        TASKDIALOGCONFIG config = {0};
        const TASKDIALOG_BUTTON buttons[] = {{ IDCANCEL, _T("Stop simulation" )}};
        config.cbSize = sizeof(config);
        config.hInstance = hInstance;
        config.hwndParent = hDlg;
        config.pszWindowTitle = _T("iWave");
        config.pszMainInstruction = _T("Simulating iWave, please wait...");
        config.pszContent = _T("Starting");
        config.dwFlags = TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER;
        config.pfCallback = SimulateTaskDlgProc;
        config.lpCallbackData = (LONG_PTR)this;
        config.pButtons = buttons;
        config.cButtons = ARRAYSIZE(buttons);
        TaskDialogIndirect(&config, NULL, NULL, NULL);
    }
    else
    {
        // Currently, we can't get the world-space node if there are other modifiers above it, 
        // so prevent simulations from starting in that case.
        ErrorDialog(hDlg,
            L"iWave is not the topmost modifier.",
            L"Delete the modifiers above the iWave modifier and try simulating again.");
    }
}

void Simulator::Cancel()
{
    if (_finished) return;
    _cancelled = true;
}

bool Simulator::IsCancelled() const
{
    return _cancelled;
}

bool Simulator::IsFinished() const
{
    return _finished;
}

bool Simulator::IsTopmostModifier() const
{
    return _geom->GetWorldSpaceObjectNode() != NULL;
}

void Simulator::Reset()
{
    delete _staticGrid;
    _staticGrid = NULL;

    for (int i = 0; i < _cache.size(); i++)
    {
        Grid* ref = _cache[i];
        delete ref; // Delete reference to plane first before the cache clears the pointer.
    }
    _cache.clear();
}

int Simulator::GetSimulatedStartFrame() const
{
    return _cacheStartFrame;
}

int Simulator::GetSimulatedFrameCount() const
{
    return (int)_cache.size();
}

Grid* Simulator::GetSimulatedGrid(int frame)
{
    if (!_cache.empty())
    {
        frame = max(frame, GetSimulatedStartFrame());
        frame = min(frame, GetSimulatedStartFrame() + GetSimulatedFrameCount() - 1);

        return _cache[frame - GetSimulatedStartFrame()];
    }
    else
    {
        delete _staticGrid;

        float width = _geom->pblock2->GetFloat(pb_width, 0); // width = plane X width
        float length = _geom->pblock2->GetFloat(pb_length, 0); // length = plane Y length
        int widthSegs = _geom->pblock2->GetInt(pb_width_segs, 0);
        int lengthSegs = _geom->pblock2->GetInt(pb_length_segs, 0);

        if (_geom->pblock2->GetInt(pb_ambient_on, 0))
        {
            TimeValue ticksPerFrame = GetTicksPerFrame();
            int ambientSeed = _geom->pblock2->GetInt(pb_seed, 0);
            float ambientDuration = TicksToSec(_geom->pblock2->GetInt(pb_duration, 0));

            Ambient* amb = new Ambient(width, length, widthSegs, lengthSegs, ambientSeed, ambientDuration, Ambient::GRAVITY_US);
            TimeValue t = frame * ticksPerFrame;
            amb->Simulate(TicksToSec(t),
                _geom->pblock2->GetFloat(pb_wind_speed, t),
                _geom->pblock2->GetFloat(pb_wind_direction, t),
                _geom->pblock2->GetFloat(pb_ambient_scale, t),
                _geom->pblock2->GetFloat(pb_min_wave_size, t),
                _geom->pblock2->GetFloat(pb_ambient_height, t));

            _staticGrid = amb;
        }
        else
        {
            _staticGrid = new Grid(width, length, widthSegs, lengthSegs);
            _staticGrid->Clear();
        }

        return _staticGrid;
    }
}

HRESULT CALLBACK Simulator::SimulateTaskDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM /*lParam*/, LONG_PTR dwRefData)
{
    WStr str;
    Simulator* instance = (Simulator*)dwRefData;

    switch (message)
    {
    case TDN_CREATED:
        _beginthread(DoWork, 0, instance);
        break;
    case TDN_TIMER:
        if (instance->_finished)
        {
            SendMessage(hWnd, TDM_CLICK_BUTTON, IDCANCEL, 0);
        }
        else
        {
            str = WStr();
            str.printf(L"Frame %d (%d frames remaining)", simCounter, simStart + simLength - simCounter);
            SendMessage(hWnd, TDM_SET_ELEMENT_TEXT, TDE_CONTENT, (LPARAM)str.data());
            SendMessage(hWnd, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(simStart, simStart + simLength));
            SendMessage(hWnd, TDM_SET_PROGRESS_BAR_POS, simCounter, 0);
        }
        break;
    case TDN_BUTTON_CLICKED:
        if (wParam == IDCANCEL && !instance->_finished)
        {
            instance->Cancel();
            return S_FALSE;
        }
        break;
    }
    return S_OK;
}

IOResult Simulator::Load(ILoad* iload)
{
    Reset();

    ULONG nb;
    IOResult res;
    
    // Save external (bool).
    res = iload->Read(&_saveExternal, sizeof(bool), &nb);
    CheckResult(res);
    
    // Save path (int size; wchar_t buf string).
    int pathLength;
    res = iload->Read(&pathLength, sizeof(int), &nb);
    CheckResult(res);

    if (pathLength > 0) {
        wchar_t* pathBuf = new wchar_t[pathLength];
        res = iload->Read(pathBuf, sizeof(wchar_t) * pathLength, &nb);
        CheckResult(res);

        std::wstring relativeExternalWstring(pathBuf, pathLength);
        delete [] pathBuf;

        MaxSDK::Util::Path absoluteExternalPath(relativeExternalWstring.c_str());
        absoluteExternalPath.ConvertToAbsolute();
        _saveExternalPath = std::wstring(absoluteExternalPath.GetString());
    }

    if (_saveExternal) {
        ExternalFile ef(_saveExternalPath);
        res = LoadExternal(ef) ? IO_OK : IO_ERROR;
#ifdef FRIENDLY_ERRORS
        if (res != IO_OK) {
            std::wstringstream errorWss;
            errorWss << L"Error reading simulation data from "
                     << PathFindFileNameW(_saveExternalPath.c_str());
            std::wstring error = errorWss.str();
            ErrorDialog(
                GetCOREInterface7()->GetMAXHWnd(),
                error.c_str(),
                L"The iWave plugin could not read the external simulation cache. The plugin has been reset to save data in the current 3ds Max scene instead.");
            
            _saveExternal = false;
            _saveExternalPath = L"";

            return IO_OK; // Tell Max things are OK so the modifier will load.
        }
#else
        CheckResult(res);
#endif
    } else {
        // Cache start frame (int).
        res = iload->Read(&_cacheStartFrame, sizeof(int), &nb);
        CheckResult(res);
    
        // Cache size (int).
        int size;
        res = iload->Read(&size, sizeof(int), &nb);
        CheckResult(res);
    
        // Cache data (float buf).
        for (int i = 0; i < size; i++)
        {
            float width;
            float length;
            int widthSegs;
            int lengthSegs;

            res = iload->Read(&width, sizeof(float), &nb);
            CheckResult(res);

            res = iload->Read(&length, sizeof(float), &nb);
            CheckResult(res);

            res = iload->Read(&widthSegs, sizeof(int), &nb);
            CheckResult(res);

            res = iload->Read(&lengthSegs, sizeof(int), &nb);
            CheckResult(res);

            int numVertices = (widthSegs + 1) * (lengthSegs + 1);
            float* vertexHeights = new float[numVertices];
            res = iload->Read(vertexHeights, sizeof(float) * numVertices, &nb);
            CheckResult(res);

            Grid* grid = new Grid(width, length, widthSegs, lengthSegs, vertexHeights);
            _cache.push_back(grid);
        }
    }

    return IO_OK;
}

IOResult Simulator::Save(ISave* isave)
{
    ULONG nb;
    IOResult res;

    // Save external (bool).
    res = isave->Write(&_saveExternal, sizeof(bool), &nb);
    CheckResult(res);

    // Save path (int size; wchar_t buf string).
    if (_saveExternalPath.size() > 0) {
        MaxSDK::Util::Path relativeExternalPath(_saveExternalPath.c_str());
        relativeExternalPath.ConvertToRelativeToProjectFolder();
        std::wstring relativeExternalWstring(relativeExternalPath.GetString());
        
        int pathLength = relativeExternalWstring.size();
        res = isave->Write(&pathLength, sizeof(int), &nb);
        CheckResult(res);
        
        const wchar_t* pathBuf = relativeExternalWstring.c_str();
        res = isave->Write(pathBuf, sizeof(wchar_t) * pathLength, &nb);
        CheckResult(res);
    } else {
        int pathLength = 0;
        res = isave->Write(&pathLength, sizeof(int), &nb);
        CheckResult(res);
    }

    if (_saveExternal) {
        ExternalFile ef(_saveExternalPath);
        res = ef.Write(_cacheStartFrame, _cache) ? IO_OK : IO_ERROR;
#ifdef FRIENDLY_ERRORS
        if (res != IO_OK) {
            std::wstringstream errorWss;
            errorWss << L"Error saving simulation data to "
                     << PathFindFileNameW(_saveExternalPath.c_str());
            std::wstring error = errorWss.str();
            ErrorDialog(
                GetCOREInterface7()->GetMAXHWnd(),
                error.c_str(),
                L"The iWave plugin could not save to the external simulation cache. Double-check your iWave data location settings.");
            
            return IO_ERROR; // Tell Max things have gone awry even though it seems to ignore this :(
        }
#else
        CheckResult(res);
#endif
    } else {
        // Cache start frame (int).
        res = isave->Write(&_cacheStartFrame, sizeof(int), &nb);
        CheckResult(res);

        // Cache size (int).
        int size = (int)_cache.size();
        res = isave->Write(&size, sizeof(int), &nb);
        CheckResult(res);

        // Cache data (float buf).
        for (int i = 0; i < size; i++)
        {
            Grid* grid = _cache[i];
            float width = grid->GetWidth();
            float length = grid->GetLength();
            int widthSegs = grid->GetWidthSegs();
            int lengthSegs = grid->GetLengthSegs();
            float* vertexHeights = grid->GetVertexHeights();

            res = isave->Write(&width, sizeof(float), &nb);
            CheckResult(res);

            res = isave->Write(&length, sizeof(float), &nb);
            CheckResult(res);

            res = isave->Write(&widthSegs, sizeof(int), &nb);
            CheckResult(res);

            res = isave->Write(&lengthSegs, sizeof(int), &nb);
            CheckResult(res);

            res = isave->Write(vertexHeights, sizeof(float) * (widthSegs + 1) * (lengthSegs + 1), &nb);
            CheckResult(res);
        }
    }

    return IO_OK;
}

void Simulator::BeginSelectExternalFile(HWND hDlg) {
    MSTR filename = L"";
    MSTR initialDirectory = L"";

    FilterList filterList;
    filterList.Append(_M("iWave simulation cache (*.iwdata)"));
    filterList.Append(_M("*.iwdata"));

    bool gotFile = GetCOREInterface8()->DoMaxSaveAsDialog(hDlg,
        L"Choose Simulation Cache",
        filename,
        initialDirectory,
        filterList);

    if (gotFile) {
        std::wstring filenameWstring(filename);
        if (CompleteSelectExternalFile(hDlg, filenameWstring)) {
            _saveExternal = true;
            _saveExternalPath = filenameWstring;
        }
    }
}

bool Simulator::CompleteSelectExternalFile(HWND hDlg, std::wstring file) {
    ExternalFile saveExternalFile(file);
    
    int numFramesFile = saveExternalFile.CheckValidity();
    int numFramesScene = GetSimulatedFrameCount();

    bool needFileRead = false;
    if (numFramesFile < 0) {
        // File error--doesn't appear valid.
        ErrorDialog(hDlg,
            L"The file could not be read.",
            L"It doesn't appear to be a valid iWave simulation cache. Please try again with another file.");
        return false;
    } else if (numFramesFile > 0 && numFramesScene == 0) {
        // Read frames from file into the scene.
        needFileRead = true;
    } else if (numFramesFile > 0 && numFramesScene > 0) {
        // Ask user to choose.
        std::wstringstream keepSceneTextWss;
        keepSceneTextWss << L"Keep data from scene (" << numFramesScene << " frames)\n"
                         << L"The next time the 3ds Max scene is saved, the simulation in the external file will be replaced.";
        std::wstring keepSceneText = keepSceneTextWss.str();

        std::wstringstream keepFileTextWss;
        keepFileTextWss << L"Keep data from " << PathFindFileNameW(file.c_str()) << " (" << numFramesFile << " frames)\n"
                        << L"The simulation data from the external file will be read immediately, and the simulation in the scene will be replaced.";
        std::wstring keepFileText = keepFileTextWss.str();

        int nButtonPressed = 0;
        TASKDIALOGCONFIG config = {0};
        const TASKDIALOG_BUTTON buttons[] = { 
            { IDYES, keepSceneText.c_str() },
            { IDNO, keepFileText.c_str() }
        };
        config.cbSize = sizeof(config);
        config.hInstance = hInstance;
        config.hwndParent = hDlg;
        config.dwCommonButtons = TDCBF_CANCEL_BUTTON;
        config.pszMainIcon = TD_WARNING_ICON;
        config.pszMainInstruction = L"Which data should be kept?";
        config.pszContent = L"Both the current scene and the selected file contain simulation data, but you can only keep one set of data.";
        config.pszWindowTitle = L"iWave";
        config.pButtons = buttons;
        config.cButtons = ARRAYSIZE(buttons);
        config.dwFlags = TDF_USE_COMMAND_LINKS;
        config.nDefaultButton = IDCANCEL;

        TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
        switch (nButtonPressed)
        {
            case IDYES:
                // The user chose to keep the current scene data.
                // We don't have to do anything right now.
                break;
            case IDNO:
                // The user chose to keep the file's data.
                needFileRead = true;
                break;
            default:
                return false;
        }
    }

    if (needFileRead && !LoadExternal(saveExternalFile)) {
        // Error reading file.
        ErrorDialog(hDlg,
            L"The file could not be read.",
            L"It may be corrupted. Try again with a different file.");
        return false;
    }

    return true;
}

void Simulator::UseNativeStorage() {
    _saveExternal = false;
    _saveExternalPath = L"";
}

bool Simulator::IsUsingExternalStorage() const {
    return _saveExternal;
}

std::wstring Simulator::GetExternalFileName() const {
    if (_saveExternalPath.size() > 0) {
        return PathFindFileNameW(_saveExternalPath.c_str());
    } else {
        return L"";
    }
}

void Simulator::ErrorDialog(HWND hDlg, std::wstring main, std::wstring detail) const {
    TASKDIALOGCONFIG config = {0};
    const TASKDIALOG_BUTTON buttons[] = {{ IDCANCEL, _T("OK" )}};
    config.cbSize = sizeof(config);
    config.hInstance = hInstance;
    config.hwndParent = hDlg;
    config.pszWindowTitle = _T("iWave");
    config.pszMainInstruction = main.c_str();
    config.pszMainIcon = TD_WARNING_ICON;
    config.pszContent = detail.c_str();
    config.dwFlags = 0;
    config.pfCallback = NULL;
    config.lpCallbackData = NULL;
    config.pButtons = buttons;
    config.cButtons = ARRAYSIZE(buttons);
    TaskDialogIndirect(&config, NULL, NULL, NULL);
}

bool Simulator::LoadExternal(ExternalFile& file) {
    if (file.Read(&_cacheStartFrame, &_cache)) {
        delete _staticGrid;
        _staticGrid = NULL;
        return true;
    }

    return false;
}
