#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include "Simulator.h"
#include <Windows.h>
#include <process.h>
#include <units.h>
#include "iWaveOcean.h"
#include "Ocean.h"

int Simulator::simStart;
int Simulator::simLength;
int Simulator::simCounter;

Simulator::Simulator(iWaveOcean* geom) : _cache(), _cacheStartFrame(0), _grid_0(0, 0), _geom(geom), _finished(true), _cancelled(false)
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

    float simStartFloat, simLengthFloat, widthSegsFloat, lengthSegsFloat;
    modifier->pblock2->GetValue(pb_sim_start, 0, simStartFloat, modifier->ivalid);
    modifier->pblock2->GetValue(pb_sim_length, 0, simLengthFloat, modifier->ivalid);
    modifier->pblock2->GetValue(pb_width_segs, 0, widthSegsFloat, modifier->ivalid);
    modifier->pblock2->GetValue(pb_length_segs, 0, lengthSegsFloat, modifier->ivalid);

    float width, length;
    modifier->pblock2->GetValue(pb_width, 0, width, modifier->ivalid); // width = plane X width
    modifier->pblock2->GetValue(pb_length, 0, length, modifier->ivalid); // length = plane Y length

    simStart = simStartFloat;
    simLength = simLengthFloat;
    int widthSegs = widthSegsFloat;
    int lengthSegs = lengthSegsFloat;

    std::vector<INode*> collisionObjs;
    int collisionObjsCount = modifier->pblock2->Count(pb_collision_objs);
    for (int i = 0; i < collisionObjsCount; i++) {
        INode* n = modifier->pblock2->GetINode(pb_collision_objs, 0, i);
        collisionObjs.push_back(n);
    }

    Ocean oc(widthSegs + 1, lengthSegs + 1, width, length, 1.0, 1/30.0, 0.3);

    for (simCounter = simStart; simCounter < simStart + simLength; simCounter++)
    {
        if (instance->_cancelled)
        {
            instance->_finished = true;
            return;
        }

        TimeValue t = simCounter * GetTicksPerFrame();

        Grid *data = oc.NextGrid();
        instance->_cache.push_back(data);
    }

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

        return _cache.at(frame - _cacheStartFrame);
    }
    else
    {
        float width, length, widthSegs, lengthSegs;
        _geom->pblock2->GetValue(pb_width, 0, width, _geom->ivalid); // width = plane X width
        _geom->pblock2->GetValue(pb_length, 0, length, _geom->ivalid); // length = plane Y length
        _geom->pblock2->GetValue(pb_width_segs, 0, widthSegs, _geom->ivalid);
        _geom->pblock2->GetValue(pb_length_segs, 0, lengthSegs, _geom->ivalid);

        _grid_0.Redim((int)widthSegs, (int)lengthSegs);
        _grid_0.MakePlanar(width, length);
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