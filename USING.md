﻿Using the Plugin
================

Creating a New Instance
-----------------------
The plugin appears in the Command Panel > Simulations category > iWaveOcean.
To create a new object instance, simply click on the iWaveOcean button and then drag in a viewport to define the ocean plane.
Note that this object is a "world-space object" and relies on its position in world space in order to calculate collisions.
Thus, you will be able to clone the object as a copy, but not as an instance or reference.

Versions and Compatibility
--------------------------
The binary format of the plugin's simulation data changed in March 2015.
If you used a compiled binary from before March 2015 or the source code from before Git revision `f4158c4`,
be aware that newer versions of the plugin are partially incompatible.
Although settings such as size, wave height, and speed will be preserved between the old and new plugins,
you will have to re-run the wave simulations when switching plugin versions.

Size Rollup
-----------
![Size rollup controls](size.png)

*Width*. The width of the simulation plane.

*Length*. The length of the simulation plane.

*Width Segs*. The number of face segments along the width axis of the plane. (The number of vertices is one greater.)

*Length Segs*. The number of face segments along the length axis of the plane. (The number of vertices is one greater.)

Ambient Rollup
--------------
![Ambient rollup controls](ambient.png)

*Make Ambient Waves*. Check this box if you would like ambient waves, such as those in an ocean or a lake, to be generated.
If no manual simulation has been run and cached, the ambient waves will be generated live as you scrub the time slider or change the ambient wave properties.
Most of the ambient wave properties can also be animated, in contrast to the dynamic wave properties.

*Sim Width*. The width of the plane as used in the ambient wave simulation algorithms.
The length used in simulation will automatically be calculated using the simulation width and the proportions of the geometry.
The basic effect of increasing this factor is to simulate a larger ocean in the same area, making waves appear bunched closer together (denser).
In contrast, decreasing this factor simulates a smaller ocean in the same area, making waves appear spread farther apart (sparser).
Animatable.

Sim Width = 1000.0 in                                                | Sim Width = 3000.0 in
-------------------------------------------------------------------- | --------------------------------------------------------------------
![Rendered waves with actual scale = 1000.0 in](compare_scale_a.png) | ![Rendered waves with actual scale = 3000.0 in](compare_scale_b.png)

*Amplitude*. Actually the semi-amplitude.
The height of the tallest wave, as measured from the origin of the plane.
(Note that you probably want to increase the amplitude in lockstep with the speed.)
Animatable.

Amplitude = 1.0                                                 | Amplitude = 4.0
--------------------------------------------------------------- | ---------------------------------------------------------------
![Rendered waves with amplitude = 1.0](compare_amplitude_a.png) | ![Rendered waves with amplitude = 4.0](compare_amplitude_b.png)

*Smallest Wave*. The smallest wave size that will actually be rendered; any wave smaller than this size will be removed from the ambient simulation.
Animatable.

*Speed*. The speed of the wind blowing on the waves.
Faster speed will give larger waves at higher frequencies.
(Note that you probably want to increase the speed in lockstep with the amplitude.)
Animatable.

Speed = 200.0 in/second                                        | Speed = 400.0 in/second
-------------------------------------------------------------- | --------------------------------------------------------------
![Rendered waves with speed = 200.0 in/s](compare_speed_a.png) | ![Rendered waves with speed = 400.0 in/s](compare_speed_b.png)

*Direction*. The angle (degrees) of the wind blowing on the waves.
Animatable.

*Seed*. An integer seed used for the random number generator used to generate the ambient wave pattern.
Choose a different one to create a different wave pattern.

*Loop Length*. The amount of time after which the animation will loop.
By default, this is specified in frames unless you have changed your 3ds Max Time Configuration.

Dynamics Rollup
---------------
![Dynamics rollup controls](dynamics.png)

The properties specified under the "Dynamics" rollup cannot be previewed live in the viewport, nor can they be animated.
To see their effect on the simulation, you must re-run the simulation by using the "Simulate" rollup (see below).

*Sim Start*. The time on which to start simulating.
By default, this is specified in frames unless you have changed your 3ds Max Time Configuration.

*Sim Length*. The amount of time for which to simulate.
By default, this is specified in frames unless you have changed your 3ds Max Time Configuration.

*Collision Objects*. The objects that interact with the waves.
Use the Add and Remove buttons to choose geometry objects from the scene.

*Damping*. A factor that controls how much the waves are damped; higher values increase damping.

*Smoothing*. A factor that determines the smoothness of the hit-tested outline generated for collision objects.
This factor is simply σ, the standard deviation for the Gaussian distribution.
A higher value of σ gives smoother boundaries around a collision object, while a lower value of σ gives sharper boundaries.

*Height Scale*. A factor by which to multiply the height of dynamically-simulated waves.

Simulate Rollup
---------------
![Simulate rollup controls](simulate.png)

*Simulate Dynamics* button. Click this button to start simulating from the "Sim Start" frame.
This button will bring up a dialog with the simulation progress, wherein you can cancel the simulation midway.

*Clear All* button. Click this button to erase all saved simulation frames.

*Start Frame*. The first frame of the saved simulation data.
This value is specified in frames regardless of the current Time Configuration.
This number may not match the "Sim Start" property if you have changed the "Sim Start" without re-simulating.

*Simulated*. The number of frames simulated thus far.
This value is specified in frames regardless of the current Time Configuration.
This number may not match the "Sim Length" property if you have cancelled a simulation midway, or if you have changed the "Sim Length" property without re-simulating.

Data Location Rollup
--------------------
![Data location rollup controls](dataloc.png)

*In current 3ds Max file* radio option (default). Select this option to keep the simulation data saved in the scene file.
This is the safest option, as the simulation data will always travel with your scene.

*In the specified file* radio option. Select this option to choose a custom location for the simulation data.
You might choose this option if you have a lot of simulation data and you want to keep your scene file light.
If you misplace the simulation file, you will have to re-simulate the water dynamics again.

If you choose a simulation file that already contains simulation data, then the iWave plugin will load the existing simulation data from that file into the scene.

*File selection* button. If you have chosen to use a custom file to save your data, you can select a different file location here.
