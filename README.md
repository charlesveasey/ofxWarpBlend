# ofxWarpBlend
Warp and blend addon for openFrameworks

This is extended port of the [Cinder-Warping](https://github.com/paulhoux/Cinder-Warping) block by Paul Houx.

Projection warping is the process of manipulating an image so that it can be projected onto flat or curved surfaces without distortion. There are three types of warps available:
* **Perspective warp**: performs image correction for projecting onto flat surfaces in case the projector is not horizontally and/or vertically aligned with the wall. Use it to exactly project your content on a rectangular area on a wall or building, regardless of how your projector is setup. For best results, disable any keystone correction on the projector. Then simply drag the four corners of your content where you want them to be on the wall.
* **Bilinear warp**: inferior to perspective warping on flat surfaces, but allows projecting onto curved surfaces. Simply add control points to the warp and drag them to where you want your content to be. 
* **Perspective-bilinear warp**: a combination of both techniques, where you first drag the four corners of your content to the desired location on the wall, then adjust for curvatures using the additional control points. If you (accidentally) move your projector later, all you need to do is adjust the four corners and the projection should perfectly fit on the curved wall again.

Projection blending is the process of creating a single image from multiple virtual or physical projections. Blending can be accomplished with this addon through the edge blending functions or by assigning a blend LUT texture. Proper blending may also require color calibration between physical projectors. This addon supports HSB adjustments as well as the assignment of a color LUT texture.

#####Controls
The addon provides a built-in editor to adjust your warps. When activated using the ```Warp::enableEditMode(true)``` function, these are the keys you can use:
* Use mouse or cursor keys to move the currently selected control point
* Press - or + to change brightness
* Press R to reset the warp to its default settings
* Press F1 to reduce the number of horizontal control points
* Press F2 to increase the number of horizontal control points
* Press F3 to reduce the number of vertical control points
* Press F4 to increase the number of vertical control points
* Press F5 to decrease the mesh resolution
* Press F6 to increase the mesh resolution
* Press F7 to toggle adaptive mesh resolution
* Press TAB to select the next control point
* Press M to toggle between linear and curved mapping (unavailable for Perspective warps)
* Press F9 to rotate content counter-clockwise (unavailable for non-Perspective warps)
* Press F10 to rotate content clockwise (unavailable for non-Perspective warps)
* Press F11 to flip content horizontally (unavailable for non-Perspective warps)
* Press F12 to flip content vertically (unavailable for non-Perspective warps)

#####To-Do's
* Add example project
* Better interpolation for mesh resolution adjustments

#####Copyright notice
Copyright (c) 2010-2015, Paul Houx - All rights reserved.
Copyright (c) 2015-2016, Charles Veasey - All rights reserved.
This code is intended for use with the openFrameworks C++ library: http://openframeworks.cc/

ofxWarpBlend is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 
ofxWarpBlend is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along with Cinder-Warping.  If not, see <http://www.gnu.org/licenses/>.
