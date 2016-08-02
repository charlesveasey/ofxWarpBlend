/*
 Original work Copyright (c) 2010-2015, Paul Houx - All rights reserved.
 Modified work Copyright (c) 2015-2016, Charles Veasey - All rights reserved.
 
 This code is intended for use with the openFrameworks C++ library: http://openframeworks.cc/
 
 This file is part of ofxWarpBlend.
 
 ofxWarpBlend is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 ofxWarpBlend is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with ofxWarpBlend.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Warp.h"

WarpPerspectiveBilinear::WarpPerspectiveBilinear()
	: WarpBilinear()
{
	// change type 
	mType = PERSPECTIVE_BILINEAR;

	// create perspective warp
	mWarp = WarpPerspectiveRef( new WarpPerspective() );
}

WarpPerspectiveBilinear::~WarpPerspectiveBilinear()
{
}

ofXml WarpPerspectiveBilinear::toXml() const
{
	//FIX
	ofXml xml = WarpBilinear::toXml();
	
	// set corners
	for( unsigned i = 0; i < 4; ++i ) {
		ofVec2f corner = mWarp->getControlPoint( i );

		ofXml cp;
        cp.addChild( "corner" );
		cp.setTo( "corner" );
		cp.setAttribute( "x", ofToString(corner.x) );
		cp.setAttribute( "y", ofToString(corner.y) );

		xml.addXml( cp );
	}

	return xml;
}

void WarpPerspectiveBilinear::fromXml( ofXml &xml )
{
	WarpBilinear::fromXml( xml );
	unsigned i = 0;
    if (xml.exists("corner")){
        
        int cornerChildren = xml.getNumChildren("corner");
        
        for(int i=0; i<cornerChildren; i++){
            string s = ofToString(i);
            float x = xml.getFloatValue("corner["+s+"][@x]");
            float y = xml.getFloatValue("corner["+s+"][@y]");
			mWarp->setControlPoint(i, ofVec2f(x, y));
        }
    }
}

void WarpPerspectiveBilinear::draw( bool controls )
{
	// apply perspective transform
    glMatrixMode(GL_MODELVIEW);
    ofPushMatrix();
    ofMultMatrix(toOf( mWarp->getTransform()) );
    
	// draw bilinear warp
	WarpBilinear::draw( false );

	// restore transform
    ofPopMatrix();
    
	// draw edit interface
	if( isEditModeEnabled() ) {
		if( controls && mSelected < mPoints.size() ) {
            mControlPoints.clear();

			// draw control points
            for( unsigned i = 0; i < mPoints.size(); ++i )
                queueControlPoint( getControlPoint( i ) * mWindowSize, mSelected == i );
            
            drawControlPoints();
        }
	}
}

void WarpPerspectiveBilinear::mouseMove( ofMouseEventArgs &event )
{
	mWarp->mouseMove( event );
	Warp::mouseMove( event );
}

void WarpPerspectiveBilinear::mouseDown(ofMouseEventArgs &event )
{
	if( !isEditModeEnabled() ) return;
	if( mSelected >= mPoints.size() ) return;

	// depending on selected control point, let perspective or bilinear warp handle it
	if( isCorner( mSelected ) ) {
		mWarp->mouseDown( event );
	}
	else {
		Warp::mouseDown( event );
	}
}

void WarpPerspectiveBilinear::mouseDrag(ofMouseEventArgs &event )
{
	if( !isEditModeEnabled() ) return;
	if( mSelected >= mPoints.size() ) return;

	// depending on selected control point, let perspective or bilinear warp handle it
	if( isCorner( mSelected ) ) {
		mWarp->mouseDrag( event );
	}
	else {
		Warp::mouseDrag( event );
	}
}

void WarpPerspectiveBilinear::keyDown( ofKeyEventArgs &event )
{
	if( !isEditModeEnabled() ) return;
	if( mSelected >= mPoints.size() ) return;

	switch( event.key ) {
	case OF_KEY_UP:
	case OF_KEY_DOWN:
	case OF_KEY_LEFT:
	case OF_KEY_RIGHT:
		// make sure cursor keys are handled by 1 warp only
		if( isCorner( mSelected ) )
			mWarp->keyDown( event );
		else if ( event.key != -1 )
            WarpBilinear::keyDown( event );
		break;
	case OF_KEY_F9:
	case OF_KEY_F10:
		// let only the Perspective warp handle rotating 
		mWarp->keyDown( event );
		break;
	case OF_KEY_F11:
	case OF_KEY_F12:
		// let only the Bilinear warp handle flipping
		WarpBilinear::keyDown( event );
		break;
	default:
		// let both warps handle the other keyDown events
		mWarp->keyDown( event );
		WarpBilinear::keyDown( event );
		break;
	}
}

void WarpPerspectiveBilinear::resize()
{
	// make content size compatible with WarpBilinear's mWindowSize
	//mWarp->setSize( ofGetWindowSize() );
	//mWarp->resize();
	//WarpBilinear::resize();
}

void WarpPerspectiveBilinear::setSize( int w, int h )
{
	mWarp->setSize( w, h );
	WarpBilinear::setSize( w, h );
}

glm::vec2 WarpPerspectiveBilinear::getControlPoint( unsigned index ) const
{
	// depending on index, return perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply return one of the corners
		return mWarp->getControlPoint( convertIndex( index ) );
	}
	else {
		// bilinear: transform control point from warped space to normalized screen space
		ofVec2f p = Warp::getControlPoint( index ) * ofVec2f( mWarp->getSize() );
		glm::vec4 pt = mWarp->getTransform() * glm::vec4( p.x, p.y, 0, 1 );

		if( pt.w != 0 ) pt.w = 1 / pt.w;
		pt *= pt.w;

		return ofVec2f( pt.x, pt.y ) / mWindowSize;
	}
}

void WarpPerspectiveBilinear::setControlPoint( unsigned index, const glm::vec2 &pos )
{
	// depending on index, set perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply set the control point
		mWarp->setControlPoint( convertIndex( index ), pos );
	}
	else {
		// bilinear:: transform control point from normalized screen space to warped space
		ofVec2f p = pos * mWindowSize;
		glm::vec4 pt = mWarp->getInvertedTransform() * glm::vec4( p.x, p.y, 0, 1 );

		if( pt.w != 0 ) pt.w = 1 / pt.w;
		pt *= pt.w;

		ofVec2f size( mWarp->getSize() );
		Warp::setControlPoint( index, ofVec2f( pt.x, pt.y ) / size );
                
	}
}

void WarpPerspectiveBilinear::moveControlPoint( unsigned index, const glm::vec2 &shift )
{
	// depending on index, move perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply move the control point
		mWarp->moveControlPoint( convertIndex( index ), shift );
	}
	else {
		// bilinear: transform control point from normalized screen space to warped space
		ofVec2f pt = getControlPoint( index );
		setControlPoint( index, pt + shift );
	}
}

void WarpPerspectiveBilinear::selectControlPoint( unsigned index )
{
	// depending on index, select perspective or bilinear control point
	if( isCorner( index ) ) {
		mWarp->selectControlPoint( convertIndex( index ) );
	}
	else {
		mWarp->deselectControlPoint();
	}

	// always select bilinear control point, which we use to keep track of editing
	Warp::selectControlPoint( index );
}

void WarpPerspectiveBilinear::deselectControlPoint()
{
	mWarp->deselectControlPoint();
	Warp::deselectControlPoint();
}

bool WarpPerspectiveBilinear::isCorner( unsigned index ) const
{
	unsigned numControls = (unsigned) ( mControlsX * mControlsY );

	return ( index == 0 || index == ( numControls - mControlsY ) || index == ( numControls - 1 ) || index == ( mControlsY - 1 ) );
}

unsigned WarpPerspectiveBilinear::convertIndex( unsigned index ) const
{
	unsigned numControls = (unsigned) ( mControlsX * mControlsY );
	if( index == 0 ) return 0;
	else if( index == ( numControls - mControlsY ) ) return 1;
	else if( index == ( numControls - 1 ) ) return 2;
	else if( index == ( mControlsY - 1 ) ) return 3;
	else return index;
}
