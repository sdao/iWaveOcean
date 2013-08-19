#define _WIN32_WINNT _WIN32_WINNT_VISTA

#define CheckResult(res) if (res != IO_OK) return res;

#include "Simulator.h"
#include <Windows.h>
#include <process.h>
#include <units.h>
#include "iWaveOcean.h"
#include "Ocean.h"
#include "Ambient.h"

int Simulator::simStart;
int Simulator::simLength;
int Simulator::simCounter;

Simulator::Simulator(iWaveOcean* geom) : _cache(), _cacheStartFrame(0), _staticGrid(NULL), _geom(geom), _finished(true), _cancelled(false)
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

    instance->Reset();

    float frameRate = GetFrameRate();
    float ticksPerFrame = GetTicksPerFrame();

    simStart = modifier->pblock2->GetInt(pb_sim_start, 0) / GetTicksPerFrame();
    simLength = modifier->pblock2->GetInt(pb_sim_length, 0) / GetTicksPerFrame();
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
        amb = new Ambient(width, length, widthSegs + 1, lengthSegs + 1, ambientSeed, ambientDuration, Ambient::GRAVITY_US); 
    }

    Ocean oc(simStart, widthSegs + 1, lengthSegs + 1, width, length, heightScale, 1.0 / frameRate, alpha, sigma, wakePower, instance->_geom->GetWorldSpaceObjectNode(), collisionNodes, collisionNodeCount, amb);

    for (simCounter = simStart; simCounter < simStart + simLength; simCounter++)
    {
        if (instance->_cancelled)
        {
            break;
        }

        if (amb)
        {
            TimeValue t = simCounter * ticksPerFrame;

            amb->Simulate(simCounter / frameRate,
                modifier->pblock2->GetFloat(pb_wind_speed, t),
                modifier->pblock2->GetFloat(pb_wind_direction, t),
                modifier->pblock2->GetFloat(pb_ambient_scale, t),
                modifier->pblock2->GetFloat(pb_min_wave_size, t),
                modifier->pblock2->GetFloat(pb_ambient_height, t));
        }

        Grid *data = oc.NextGrid();
        instance->_cache.push_back(data);
    }

    delete [] collisionNodes;
    delete amb;
    instance->_finished = true;
}

void Simulator::BeginSimulation(HWND hDlg)
{
    if (!_finished) {
        return;
    }

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

void Simulator::Cancel()
{
    if (_finished) return;
    _cancelled = true;
}

bool Simulator::IsCancelled()
{
    return _cancelled;
}

bool Simulator::IsFinished()
{
    return _finished;
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

int Simulator::GetSimulatedStartFrame()
{
    return _cacheStartFrame;
}

int Simulator::GetSimulatedFrameCount()
{
    return _cache.size();
}

Grid* Simulator::GetSimulatedGrid(int frame)
{
    if (!_cache.empty())
    {
        frame = max(frame, _cacheStartFrame);
        frame = min(frame, _cacheStartFrame + _cache.size() - 1);

        return _cache[frame - _cacheStartFrame];
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
            float frameRate = GetFrameRate();
            float ticksPerFrame = GetTicksPerFrame();
            int ambientSeed = _geom->pblock2->GetInt(pb_seed, 0);
            float ambientDuration = TicksToSec(_geom->pblock2->GetInt(pb_duration, 0));

            Ambient* amb = new Ambient(width, length, widthSegs + 1, lengthSegs + 1, ambientSeed, ambientDuration, Ambient::GRAVITY_US);
            TimeValue t = frame * ticksPerFrame;

            amb->Simulate(frame / frameRate,
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

HRESULT CALLBACK Simulator::SimulateTaskDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData)
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

    res = iload->Read(&_cacheStartFrame, sizeof(int), &nb);
    CheckResult(res);

    int size;
    res = iload->Read(&size, sizeof(int), &nb);
    CheckResult(res);

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
        float *vertexHeights = new float[numVertices];
        res = iload->Read(vertexHeights, sizeof(float) * numVertices, &nb);
        CheckResult(res);

        Grid* grid = new Grid(width, length, widthSegs, lengthSegs, vertexHeights);
        _cache.push_back(grid);
    }

    return IO_OK;
}

IOResult Simulator::Save(ISave* isave)
{
    ULONG nb;
    IOResult res;

    res = isave->Write(&_cacheStartFrame, sizeof(int), &nb);
    CheckResult(res);

    int size = _cache.size();
    res = isave->Write(&size, sizeof(int), &nb);
    CheckResult(res);

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

    return IO_OK;
}