#pragma once

#include <point3.h>
#include <iparamb2.h>
#include <vector>
#include "Grid.h"
#include "ExternalFile.h"

class iWaveOcean;

/**
Manages the cached geometry for a frame-by-frame simulation that renders a grid mesh,
and shows a progress dialog while simulating.
*/
class Simulator
{
    /** Whether the last simulation is finished. If false, a simulation is running. */
    volatile bool _finished;

    /** Whether the last simulation was cancelled or is being cancelled. */
    volatile bool _cancelled;

    /** The grid to return in place of the simulation when no simulated data is available. */
    Grid* _staticGrid;

    /** The first frame of the simulation. */
    int _cacheStartFrame;

    /** The cached geometry of the simulation. */
    std::vector<Grid*> _cache;

    /** A pointer back to the geometry object that renders the simulation. */
    iWaveOcean* _geom;

    /** Whether data should be saved to an external file instead of to the Max file. */
    bool _saveExternal;

    /** The external file path, if _saveExternal is true. */
    std::wstring _saveExternalPath;

    /** Used for updating UI. */
    static int simStart;

    /** Used for updating UI. */
    static int simLength;

    /** Used for updating UI. */
    static int simCounter;

    /** The main function that does simulation work in a separate thread. */
    static void DoWork(void* ptr);

    /** DlgProc for the task dialog that displays during simulation. */
    static HRESULT CALLBACK SimulateTaskDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData);

    /** Gets whether cancellation has been requested for the running simulation. */
    bool IsCancelled() const;

    /** Gets whether the running simulation has finished. Returns true if no simulation has been started yet. */
    bool IsFinished() const;

    /** Gets whether the simulator's modifier is the top-most world-space modifier. */
    bool IsTopmostModifier() const;

    bool CompleteSelectExternalFile(HWND hDlg, std::wstring file);

public:
    /** Creates a new Simulator for the specified geometry object. */
    Simulator(iWaveOcean* geom);
    ~Simulator(void);

    /**
    Begins a simulation.
    If the previous simulation has not yet ended, does nothing.
    Calling this method from multiple threads is not supported; it is not thread-safe.
    \param hDlg the dialog that will be the parent of the status dialog
    */
    void BeginSimulation(HWND hDlg);

    /**
    Asks the running simulation to cancel and immediately returns control to the caller without waiting for the simulation to cancel.
    Does nothing if no simulation is running.
    */
    void Cancel();

    /** Removes all the cached geometry of the simulation. */
    void Reset();

    /** Gets the first frame number in the simulation. */
    int GetSimulatedStartFrame() const;

    /** Gets the number of simulated frames in the simulation. */
    int GetSimulatedFrameCount() const;

    /**
    Gets the deformed grid as calculated by the simulation at a specified frame.
    Returns a flat plane if no cached frames are available.
    If the index is outside of the simulated range but there are frames available, chooses the closest frame.
    \param frame the frame number to get the simulated grid for
    */
    Grid* GetSimulatedGrid(int frame);

    IOResult Load(ILoad* iload);
    IOResult Save(ISave* isave);
    bool LoadExternal(ExternalFile& file);

    void BeginSelectExternalFile(HWND hDlg);
    void UseNativeStorage();
    bool IsUsingExternalStorage() const;
    std::wstring GetExternalFileName() const;

    void ErrorDialog(HWND hDlg, std::wstring main, std::wstring detail) const;
};

