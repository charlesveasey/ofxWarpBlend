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
#define STRINGIFY(A) #A

WarpPerspective::WarpPerspective( void )
	: Warp( PERSPECTIVE )
{
	mSource[0].x = 0.0f;
	mSource[0].y = 0.0f;
	mSource[1].x = (float)mWidth;
	mSource[1].y = 0.0f;
	mSource[2].x = (float)mWidth;
	mSource[2].y = (float)mHeight;
	mSource[3].x = 0.0f;
	mSource[3].y = (float)mHeight;

	reset();
}

WarpPerspective::~WarpPerspective( void )
{
}

glm::mat4 WarpPerspective::getTransform()
{
    mWindowSize = glm::vec2(mWidth, mHeight);
    
	// calculate warp matrix
	if( mIsDirty ) {
		// update source size
		mSource[1].x = (float)mWidth;
		mSource[2].x = (float)mWidth;
		mSource[2].y = (float)mHeight;
		mSource[3].y = (float)mHeight;

		// convert corners to actual destination pixels
		mDestination[0].x = mPoints[0].x * mWindowSize.x;
		mDestination[0].y = mPoints[0].y * mWindowSize.y;
		mDestination[1].x = mPoints[1].x * mWindowSize.x;
		mDestination[1].y = mPoints[1].y * mWindowSize.y;
		mDestination[2].x = mPoints[2].x * mWindowSize.x;
		mDestination[2].y = mPoints[2].y * mWindowSize.y;
		mDestination[3].x = mPoints[3].x * mWindowSize.x;
		mDestination[3].y = mPoints[3].y * mWindowSize.y;

		// calculate warp matrix
		mTransform = getPerspectiveTransform( mSource, mDestination );
		mInverted = glm::inverse( mTransform );
		mIsDirty = false;
	}

	return mTransform;
}

void WarpPerspective::reset()
{
	mPoints.clear();
	mPoints.push_back( glm::vec2( 0.0f, 0.0f ) );
	mPoints.push_back( glm::vec2( 1.0f, 0.0f ) );
	mPoints.push_back( glm::vec2( 1.0f, 1.0f ) );
	mPoints.push_back( glm::vec2( 0.0f, 1.0f ) );

	mIsDirty = true;
}

void WarpPerspective::bind()
{
}

void WarpPerspective::unbind()
{
}

void WarpPerspective::draw( const ofTexture &texture, const ofRectangle &srcArea, const ofRectangle &destRect )
{
	// clip against bounds
	ofRectangle	area = srcArea;
	ofRectangle	rect = destRect;
	clip( area, rect );

	// save current drawing color
	
	//FIX
	//const ofColor &currentColor = gl::context()->getCurrentColor();
	//gl::ScopedColor color( currentColor );
	const ofColor currentColor = ofColor(255);

	// adjust brightness
	if (mBrightness < 1.f) {
		ofColor drawColor = ofColor(mBrightness * currentColor.r, mBrightness * currentColor.g, mBrightness * currentColor.b);
		drawColor.a = currentColor.a;
		ofSetColor(drawColor);
	}

	// create shader if necessary
	createShader();

	// draw texture
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	ofMultMatrix( toOf(getTransform()) );

	//texture.bind();

    //mShader->begin();
       // mShader->setUniform3f( "uLuminance", mLuminance );
       // mShader->setUniform3f( "uGamma", mGamma );
       // mShader->setUniform4f( "uEdges", mEdges );
       // mShader->setUniform1f( "uExponent", mExponent );

	//FIX
	//auto coords = texture->getAreaTexCoords( srcArea );
	//gl::drawSolidRect( rect, coords.getUpperLeft(), coords.getLowerRight() );
	
    ofDrawRectangle(rect);
	
    //mShader->end();    
    glPopMatrix();

	// draw interface
	draw();
}

void WarpPerspective::begin()
{
	glMatrixMode(GL_MODELVIEW);
    ofPushMatrix();
	ofMultMatrix(toOf(getTransform()));
}

void WarpPerspective::end()
{
	// restore warp
    ofPopMatrix();

	// draw interface
	draw();
}

void WarpPerspective::draw( bool controls )
{
	// only draw grid while editing
	if( isEditModeEnabled() ) {
		glMatrixMode(GL_MODELVIEW);
        ofPushMatrix();
        
		ofMultMatrix(toOf(getTransform()));
		ofSetLineWidth(1.0f);

		glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

		ofSetColor(255);

		for( int i = 0; i <= 1; i++ ) {
			float s = i / 1.0f;
			glm::vec2 m = glm::vec2(s * (float)mWidth, 0.0f);
			glm::vec2 n = glm::vec2(s * (float)mWidth, (float)mHeight);

			glm::vec2 o = glm::vec2(0.0f, s * (float)mHeight);
			glm::vec2 p = glm::vec2((float)mWidth, s * (float)mHeight);

			ofDrawLine(float(m.x), float(m.y), float(n.x), float(n.y));
			ofDrawLine(float(o.x), float(o.y), float(p.x), float(p.y));
		}

		glm::vec2 m = glm::vec2(0.0f, 0.0f);
		glm::vec2 n = glm::vec2((float)mWidth, (float)mHeight);

		glm::vec2 o = glm::vec2((float)mWidth, 0.0f);
		glm::vec2 p = glm::vec2(0.0f, (float)mHeight);

		ofDrawLine(float(m.x), float(m.y), float(n.x), float(n.y));
		ofDrawLine(float(o.x), float(o.y), float(p.x), float(p.y));

        ofPopMatrix();

        if( controls && mSelected < mPoints.size() ) {
            // draw control points
            mControlPoints.clear();

            for( int i = 0; i < 4; i++ ) {
                queueControlPoint( mDestination[i], i == mSelected );
            }
            drawControlPoints();
        }
	}
}

void WarpPerspective::keyDown( ofKeyEventArgs &event )
{
	// let base class handle keys first
	Warp::keyDown( event );
    
	if( event.key == -1)
		return;

	// disable keyboard input when not in edit mode
	if( !isEditModeEnabled() ) return;

	// do not listen to key input if not selected
	if( mSelected >= mPoints.size() ) return;

	switch( event.key ) {
		case OF_KEY_F9:
			// rotate content ccw
			std::swap( mPoints[1], mPoints[2] );
			std::swap( mPoints[0], mPoints[1] );
			std::swap( mPoints[3], mPoints[0] );
			mSelected = ( mSelected + 1 ) % 4;
			mIsDirty = true;
			break;
		case OF_KEY_F10:
			// rotate content cw
			std::swap( mPoints[3], mPoints[0] );
			std::swap( mPoints[0], mPoints[1] );
			std::swap( mPoints[1], mPoints[2] );
			mSelected = ( mSelected + 3 ) % 4;
			mIsDirty = true;
			break;
		case OF_KEY_F11:
			// flip content horizontally
			std::swap( mPoints[0], mPoints[1] );
			std::swap( mPoints[2], mPoints[3] );
			if( mSelected % 2 ) mSelected--;
			else mSelected++;
			mIsDirty = true;
			break;
		case OF_KEY_F12:
			// flip content vertically
			std::swap( mPoints[0], mPoints[3] );
			std::swap( mPoints[1], mPoints[2] );
			mSelected = ( (unsigned)mPoints.size() - 1 ) - mSelected;
			mIsDirty = true;
			break;
		default:
			return;
	}

    event.key = -1;
}

// Adapted from code found here: http://forum.openframeworks.cc/t/quad-warping-homography-without-opencv/3121/19
glm::mat4 WarpPerspective::getPerspectiveTransform( const glm::vec2 src[4], const glm::vec2 dst[4] ) const
{
	float p[8][9] = {
		{ -src[0][0], -src[0][1], -1,   0,   0,  0, src[0][0] * dst[0][0], src[0][1] * dst[0][0], -dst[0][0] }, // h11
		{ 0,   0,  0, -src[0][0], -src[0][1], -1, src[0][0] * dst[0][1], src[0][1] * dst[0][1], -dst[0][1] }, // h12
		{ -src[1][0], -src[1][1], -1,   0,   0,  0, src[1][0] * dst[1][0], src[1][1] * dst[1][0], -dst[1][0] }, // h13
		{ 0,   0,  0, -src[1][0], -src[1][1], -1, src[1][0] * dst[1][1], src[1][1] * dst[1][1], -dst[1][1] }, // h21
		{ -src[2][0], -src[2][1], -1,   0,   0,  0, src[2][0] * dst[2][0], src[2][1] * dst[2][0], -dst[2][0] }, // h22
		{ 0,   0,  0, -src[2][0], -src[2][1], -1, src[2][0] * dst[2][1], src[2][1] * dst[2][1], -dst[2][1] }, // h23
		{ -src[3][0], -src[3][1], -1,   0,   0,  0, src[3][0] * dst[3][0], src[3][1] * dst[3][0], -dst[3][0] }, // h31
		{ 0,   0,  0, -src[3][0], -src[3][1], -1, src[3][0] * dst[3][1], src[3][1] * dst[3][1], -dst[3][1] }, // h32
	};

	gaussianElimination( &p[0][0], 9 );

	glm::mat4 result = glm::mat4( p[0][8], p[3][8], 0, p[6][8],
						p[1][8], p[4][8], 0, p[7][8],
						0, 0, 1, 0,
						p[2][8], p[5][8], 0, 1 );

	return result;
}

// Adapted from code found here: http://forum.openframeworks.cc/t/quad-warping-homography-without-opencv/3121/19
void WarpPerspective::gaussianElimination( float *a, int n ) const
{
	int i = 0;
	int j = 0;
	int m = n - 1;

	while( i < m && j < n ) {
		int maxi = i;
		for( int k = i + 1; k<m; ++k ) {
			if( fabs( a[k*n + j] ) > fabs( a[maxi*n + j] ) ) {
				maxi = k;
			}
		}

		if( a[maxi*n + j] != 0 ) {
			if( i != maxi )
				for( int k = 0; k < n; k++ ) {
					float aux = a[i*n + k];
					a[i*n + k] = a[maxi*n + k];
					a[maxi*n + k] = aux;
				}

			float a_ij = a[i*n + j];
			for( int k = 0; k < n; k++ ) {
				a[i*n + k] /= a_ij;
			}

			for( int u = i + 1; u < m; u++ ) {
				float a_uj = a[u*n + j];
				for( int k = 0; k < n; k++ ) {
					a[u*n + k] -= a_uj*a[i*n + k];
				}
			}

			++i;
		}
		++j;
	}

	for( int i = m - 2; i >= 0; --i ) {
		for( int j = i + 1; j < n - 1; j++ ) {
			a[i*n + m] -= a[i*n + j] * a[j*n + m];
		}
	}
}

void WarpPerspective::createShader()
{
	if( mShader)
		return;

	static string VertShader = STRINGIFY(
		'#version 150'
		uniform mat4 ciModelViewProjection;

		in vec4 ciPosition;
		in vec2 ciTexCoord0;
		in vec4 ciColor;

		out vec2 vertTexCoord0;
		out vec4 vertColor;

		void main( void )
		{
			vertColor = ciColor;
			vertTexCoord0 = ciTexCoord0;

			gl_Position = ciModelViewProjection * ciPosition;
		}
	);
	static string FragShader = STRINGIFY(
        '#version 150'

		uniform sampler2D  uTex0;
		uniform vec3       uLuminance;
		uniform vec3       uGamma;
		uniform vec4       uEdges;
		uniform float      uExponent;

		in vec2 vertTexCoord0;
		in vec4 vertColor;

		out vec4 fragColor;

		void main( void )
		{
			vec4 texColor = texture( uTex0, vertTexCoord0 );

			float a = 1.0;
			if( uEdges.x > 0.0 ) a *= clamp( vertTexCoord0.x / uEdges.x, 0.0, 1.0 );
			if( uEdges.y > 0.0 ) a *= clamp( vertTexCoord0.y / uEdges.y, 0.0, 1.0 );
			if( uEdges.z > 0.0 ) a *= clamp( ( 1.0 - vertTexCoord0.x ) / uEdges.z, 0.0, 1.0 );
			if( uEdges.w > 0.0 ) a *= clamp( ( 1.0 - vertTexCoord0.y ) / uEdges.w, 0.0, 1.0 );

			const vec3 one = vec3( 1.0 );
			vec3 blend = ( a < 0.5 ) ? ( uLuminance * pow( 2.0 * a, uExponent ) )
				: one - ( one - uLuminance ) * pow( 2.0 * ( 1.0 - a ), uExponent );

			texColor.rgb *= pow( blend, one / uGamma );

			fragColor = texColor;
		}
	);
	try {
		mShader->load(VertShader, FragShader);
	}
	catch( const std::exception &e ) {
		cout << e.what() << std::endl;
	}
}

