#define _WIN32_WINNT _WIN32_WINNT_VISTA

#define CheckResult(res) if (res != IO_OK) return res;

#include "Simulator.h"
#include <Windows.h>
#include <process.h>
#include <units.h>
#include "iWaveOcean.h"
#include "Ocean.h"

int Simulator::simStart;
int Simulator::simLength;
int Simulator::simCounter;

Simulator::Simulator(iWaveOcean* geom) : _cache(), _cacheStartFrame(0), _grid_0(1.0, 1.0, 0, 0), _geom(geom), _finished(true), _cancelled(false)
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

    int widthSegs, lengthSegs;
    float alpha, sigma, wakePower, heightScale;
    modifier->pblock2->GetValue(pb_sim_start, 0, simStart, modifier->ivalid);
    modifier->pblock2->GetValue(pb_sim_length, 0, simLength, modifier->ivalid);
    modifier->pblock2->GetValue(pb_width_segs, 0, widthSegs, modifier->ivalid);
    modifier->pblock2->GetValue(pb_length_segs, 0, lengthSegs, modifier->ivalid);
    modifier->pblock2->GetValue(pb_wave_damping, 0, alpha, modifier->ivalid);
    modifier->pblock2->GetValue(pb_collision_smoothing, 0, sigma, modifier->ivalid);
    modifier->pblock2->GetValue(pb_wake_power, 0, wakePower, modifier->ivalid);
    modifier->pblock2->GetValue(pb_height_scale, 0, heightScale, modifier->ivalid);

    float width, length;
    modifier->pblock2->GetValue(pb_width, 0, width, modifier->ivalid); // width = plane X width
    modifier->pblock2->GetValue(pb_length, 0, length, modifier->ivalid); // length = plane Y length

    int collisionNodeCount = modifier->pblock2->Count(pb_collision_objs);
    INode** collisionNodes = new INode*[collisionNodeCount];
    for (int i = 0; i < collisionNodeCount; i++)
    {
        INode* n = modifier->pblock2->GetINode(pb_collision_objs, 0, i);
        collisionNodes[i] = n;
    }

    float secondsPerFrame = 1.0 / GetFrameRate();
    Ocean oc(widthSegs + 1, lengthSegs + 1, width, length, heightScale, secondsPerFrame, alpha, sigma, wakePower, instance->_geom->GetWorldSpaceObjectNode(), collisionNodes, collisionNodeCount);

    for (simCounter = simStart; simCounter < simStart + simLength; simCounter++)
    {
        if (instance->_cancelled)
        {
            break;
        }

        TimeValue t = simCounter * GetTicksPerFrame();
        oc.UpdateObstructions(t);

        Grid *data = oc.NextGrid();
        instance->_cache.push_back(data);
    }

    delete [] collisionNodes;
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
        float width, length;
        int widthSegs, lengthSegs;
        _geom->pblock2->GetValue(pb_width, 0, width, _geom->ivalid); // width = plane X width
        _geom->pblock2->GetValue(pb_length, 0, length, _geom->ivalid); // length = plane Y length
        _geom->pblock2->GetValue(pb_width_segs, 0, widthSegs, _geom->ivalid);
        _geom->pblock2->GetValue(pb_length_segs, 0, lengthSegs, _geom->ivalid);

        _grid_0.Redim(width, length, widthSegs, lengthSegs);
        _grid_0.Clear();
        return &_grid_0;
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