iWave Ocean Simulation Plugin
=============================

This plugin for Autodesk 3ds Max simulates both ambient waves and dynamic, collision-driven waves.
To install it, simply copy the `iWaveOcean.dlo` file to your 3ds Max plugins folder (for 3ds Max 2014, this should be `C:\Program Files\Autodesk\3ds Max 2014\plugins`).
The plugin will appear in the Command Panel > Simulations category > iWaveOcean.

License
-------
This software is licensed under the GNU General Public License, version 3.
For more information, please see the LICENSE file included with the source code.

Compiling
---------
This plugin is designed to work with the Autodesk 3ds Max 2014 SDK or later.
This SDK version requires that you have the Microsoft Visual Studio 2010 build tools installed.
However, you can still use later versions of Visual Studio, such as VS 2012, to edit the project so long as you have the 2010 build tools on your system.
Note that, in most cases, you cannot debug the plugin from VS using the "Debug" configuration, since you probably don't have the debug symbols for `3dsmax.exe`.
You will have to use the Hybrid configuration to load the debugger only for the plugin's code and not for Max.

Credits
-------
The iWave plugin uses [FFTW](http://fftw.org) in order to simulate ambient waves using Tessendorf's FFT-based method.
The FFTW binary DLLs are included in the `src/fftw-3.3.3-dll64` folder.

The plugin uses algorithms published by Jerry Tessendorf, chief amongst which are the FFT method for simulating ambient waves and the iWave method for dynamical waves.
The papers and course notes explaining these algorithms are available in the `notes` folder, or you can visit [Tessendorf's website](http://jerrytessendorf.blogspot.com).