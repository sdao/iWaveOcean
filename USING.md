Using the Plugin
================

Creating a New Instance
-----------------------
The plugin appears in the Command Panel > Simulations category > iWaveOcean.
To create a new object instance, simply click on the iWaveOcean button and then drag in a viewport to define the ocean plane.
Note that this object is a "world-space object" and relies on its position in world space in order to calculate collisions.
Thus, you will be able to clone the object as a copy, but not as an instance or reference.

Size Rollup
-----------
*Width*. The width of the simulation plane.

*Length*. The length of the simulation plane.

*Width Segs*. The number of face segments along the width axis of the plane. (The number of vertices is one greater.)

*Length Segs*. The number of face segments along the length axis of the plane. (The number of vertices is one greater.)

Ambient Rollup
--------------
*Make Ambient Waves*. Check this box if you would like ambient waves, such as those in an ocean or a lake, to be generated.
If no manual simulation has been run and cached, the ambient waves will be generated live as you scrub the time slider or change the ambient wave properties.
Most of the ambient wave properties can also be animated, in contrast to the dynamic wave properties.

*Actual Scale*. The width of the plane (meters) used in the ambient wave simulation algorithms.
The basic effect of increasing this factor is to simulate a larger ocean in the same area, making waves appear smaller and shallower.
In contrast, increasing this factor simulates a smaller ocean in the same area, making waves appear larger and more powerful.
Animatable.

*Amplitude*. A factor (unitless) that influences the heights of the waves. The factor is proportional to the wave height.
Animatable.

*Smallest Wave*. The smallest wave size (meters) that will actually be rendered; any wave smaller than this size will be removed from the ambient simulation.
Animatable.

*Speed*. The speed (meters per second) of the wind blowing on the waves.
Animatable.

*Direction*. The angle (degrees) of the wind blowing on the waves.
Animatable.

*Seed*. An integer seed (unitless) used for the random number generator used to generate the ambient wave pattern.
Choose a different one to create a different wave pattern.

*Loop Frames*. The number of frames after which the animation will loop.

Dynamics Rollup
---------------

Simulate Rollup
---------------