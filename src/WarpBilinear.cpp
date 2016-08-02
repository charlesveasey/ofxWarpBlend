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
#include "BSpline.h"

#define STRINGIFY(A) #A

bool sswitch = false;
bool gswitch = false;
glm::vec2 p2;

WarpBilinear::WarpBilinear()
	: Warp( BILINEAR )
	, mIsLinear( false )
	, mIsAdaptive( false )
	, mX1( 0.0f )
	, mY1( 0.0f )
	, mX2( 1.0f )
	, mY2( 1.0f )
	, mResolutionX( 0 )
	, mResolutionY( 0 )
	, mResolution( 16 ) // higher value is coarser mesh
{
    reset();
    
    
        storedPositions.clear();
        for( int x = 0; x < 100000; x++ ) {
            glm::vec2 v;
            v.x = 0;
            v.y = 0;
            storedPositions.push_back(v);
        }
}

WarpBilinear::~WarpBilinear()
{
}

ofXml WarpBilinear::toXml() const
{
	ofXml xml = Warp::toXml();
	return xml;
}

void WarpBilinear::fromXml( ofXml &xml )
{
	Warp::fromXml( xml );
}

void WarpBilinear::reset()
{
	mPoints.clear();
	for( int x = 0; x < mControlsX; x++ ) {
		for( int y = 0; y < mControlsY; y++ ) {
			mPoints.push_back( glm::vec2( x / float( mControlsX - 1 ), y / float( mControlsY - 1 ) ) );
		}
	}

	mIsDirty = true;
}

void WarpBilinear::draw( const ofTexture &texture, const ofRectangle &srcArea, const ofRectangle &destRect )
{    
	//texture.bind();

        //FIX
        // clip against bounds
        //ofRectangle	area = srcArea;
        //ofRectangle	rect = destRect;
        //clip( area, rect );

        // set texture coordinates
        //float w = static_cast<float>( texture.getWidth() );
        //float h = static_cast<float>( texture.getHeight() );

        //if( texture.texData.textureTarget == GL_TEXTURE_RECTANGLE_ARB )
        //    setTexCoords( (float)area.x, (float)area.y, (float)area.width + (float)area.x, (float)area.height + (float)area.y);
        //else
        //    setTexCoords( area.x / w, area.y / h, (float)area.width + (float)area.x / w, (float)area.height + (float)area.y / h );

        // draw
        draw();
    
	//texture.unbind();
}

void WarpBilinear::begin()
{
	// check if the FBO was created and is of the correct size
    
    // adjust brightness
    /*if( mBrightness < 1.f ) {
        ofColor drawColor = ofColor(mBrightness * 255, mBrightness * 255, mBrightness * 255);
        drawColor.a = 255;
        ofSetColor(drawColor);
    }
    else {
        ofSetColor(255);
    }*/
    ofSetColor(255);
    
    
    if (!mFbo){
        mFbo = make_shared<ofFbo>();
    }
	if( !mFbo->isAllocated() ) {
		try {
			ofFbo::Settings settings;
			settings.width = mWidth;
			settings.height = mHeight;
            
            settings.internalformat = GL_RGB;
			mFbo->allocate(settings);
		}
		catch( ... ) {
			// try creating Fbo with default format settings
			try {
				mFbo->allocate( mWidth, mHeight );
			}
			catch( ... ) {
				return;
			}
		}
	}

	mFbo->begin();
    ofClear(255);
}

void WarpBilinear::end()
{
    mFbo->end();
    

    // draw flipped
    ofRectangle srcArea = ofRectangle(0, 0, mFbo->getWidth(), mFbo->getHeight());
   int32_t t = srcArea.y; srcArea.y = srcArea.height + srcArea.y; srcArea.height = t;
   draw( mFbo->getTexture(), srcArea, ofRectangle( getBounds() ) );

	//draw();
}

void WarpBilinear::bind(){
    mFbo->getTexture().bind();
}

void WarpBilinear::unbind(){
    mFbo->getTexture().unbind();
}

void WarpBilinear::draw( bool controls )
{
	createShader();
    createBuffers();

	if( !mVboMesh ) return;
    
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_WRITEMASK);
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

	// draw warp mesh
	ofSetColor(255);
	mFbo->getTexture().bind();
       mShader->begin();
        mShader->setUniformTexture( "uTex0", mFbo->getTexture(), 0 );
        mShader->setUniform4f( "uExtends", ofVec4f( float(mWidth), float(mHeight), mWidth / float( mControlsX - 1 ), mHeight / float( mControlsY - 1 ) ) );
        mShader->setUniform3f( "uLuminance", mLuminance );
        mShader->setUniform3f( "uGamma", mGamma );
        mShader->setUniform4f( "uEdges", mEdges );
        mShader->setUniform1f( "uExponent", mExponent );
        mShader->setUniform1i( "uEditMode", (bool)isEditModeEnabled() );
        mShader->setUniform2f( "uResolution", ofVec2f( float(mWidth), float(mHeight) ));
        mShader->setUniform1f( "uBrightness", mBrightness );
        mShader->setUniform1f( "uContrast", mContrast );
        mShader->setUniform1f( "uSaturation", mSaturation );
		mShader->setUniform1i( "uUseColorLut", sUseColorLut );
		if (sUseColorLut) {
			mShader->setUniformTexture( "uColorLut", lutTexture, 1 );
			mShader->setUniform1f( "uMapdim", 256.0);
		}
		//mShader->setUniformTexture( "uBlendTexture", blendTexture, 2 );
		mVboMesh->draw();
	   mShader->end();
	mFbo->getTexture().unbind();
    
	// draw edit interface
	if (isEditModeEnabled() && controls && mSelected < mPoints.size()) {
        mControlPoints.clear();
        for( unsigned i = 0; i < mPoints.size(); i++ )
            queueControlPoint( getControlPoint( i ) * mWindowSize, i == mSelected );
		drawControlPoints();
	}
}

void WarpBilinear::keyDown( ofKeyEventArgs &event )
{
	// let base class handle keys first
	Warp::keyDown( event );
	
	if( event.key == -1)
		return;

	// disable keyboard input when not in edit mode
	if( !isEditModeEnabled() ) return;

	// do not listen to key input if not selected
	if( mSelected >= mPoints.size() ) return;

	// in case we need to find the closest control point
	float distance;
	ofVec2f  pt = (ofVec2f) mMouse;

	switch( event.key) {
		case OF_KEY_F1:
			// reduce the number of horizontal control points
			if( !ofGetKeyPressed(OF_KEY_SHIFT))
				setNumControlX( ( mControlsX + 1 ) / 2 );
			else setNumControlX( mControlsX - 1 );
			// find closest control point
			mSelected = findControlPoint( pt, &distance );
			break;
		case OF_KEY_F2:
			// increase the number of horizontal control points
			if( !ofGetKeyPressed(OF_KEY_SHIFT))
				setNumControlX( 2 * mControlsX - 1 );
			else setNumControlX( mControlsX + 1 );
			// find closest control point
			mSelected = findControlPoint( pt, &distance );
			break;
		case OF_KEY_F3:
			// reduce the number of vertical control points
			if( !ofGetKeyPressed(OF_KEY_SHIFT))
				setNumControlY( ( mControlsY + 1 ) / 2 );
			else setNumControlY( mControlsY - 1 );
			// find closest control point
			mSelected = findControlPoint( pt, &distance );
			break;
		case OF_KEY_F4:
			// increase the number of vertical control points
			if( !ofGetKeyPressed(OF_KEY_SHIFT) )
				setNumControlY( 2 * mControlsY - 1 );
			else setNumControlY( mControlsY + 1 );
			// find closest control point
			mSelected = findControlPoint( pt, &distance );
			break;
		case OF_KEY_F5:
			// decrease the mesh resolution
			if( mResolution < 64 ) {
				mResolution += 4;
				mIsDirty = true;
			}
			break;
		case OF_KEY_F6:
			// increase the mesh resolution
			if( mResolution > 4 ) {
				mResolution -= 4;
				mIsDirty = true;
			}
			break;
		case OF_KEY_F7:
			// toggle adaptive mesh resolution
			mIsAdaptive = !mIsAdaptive;
			mIsDirty = true;
			break;
		case OF_KEY_F9:
			// rotate content ccw
            sswitch = !sswitch;
			break;
		case OF_KEY_F10:
			// rotate content cw
            gswitch = !gswitch;
			break;

		case 's':
			// rotate content ccw
			sswitch = !sswitch;
			break;
		case 'g':
			// rotate content cw
			gswitch = !gswitch;
			break;


		case OF_KEY_F11:
		{
			// flip control points horizontally
			std::vector<glm::vec2> points;
			for( int x = mControlsX - 1; x >= 0; --x ) {
				for( int y = 0; y < mControlsY; ++y ) {
					int i = ( x * mControlsY + y );
					points.push_back( mPoints[i] );
				}
			}
			mPoints = points;
			mIsDirty = true;
			// find closest control point
			mSelected = findControlPoint( pt, &distance );
		}
            break;
		case OF_KEY_F12:
		{
			// flip control points vertically
			std::vector<glm::vec2> points;
			for( int x = 0; x < mControlsX; ++x ) {
				for( int y = mControlsY - 1; y >= 0; --y ) {
					int i = ( x * mControlsY + y );
					points.push_back( mPoints[i] );
				}
			}
			mPoints = points;
			mIsDirty = true;
			// find closest control point
			mSelected = findControlPoint( pt, &distance );
		}
            break;
		default:
			return;
	}

    event.key = -1;
}

void WarpBilinear::createBuffers()
{
	if( mIsDirty ) {
		if( mIsAdaptive ) {
			// determine a suitable mesh resolution based on width/height of the window
			// and the size of the mesh in pixels
			ofRectangle rect = getMeshBounds();
			createMesh( (int)( rect.getWidth() / mResolution ), (int)( rect.getHeight() / mResolution ) );
		}
		else {
			// use a fixed mesh resolution
			createMesh( mWidth / mResolution, mHeight / mResolution );
		}
		updateMesh();
	}
}

void WarpBilinear::createMesh( int resolutionX, int resolutionY )
{
	// convert from number of quads to number of vertices
	++resolutionX;	++resolutionY;

	// find a value for resolutionX and resolutionY that can be
	// evenly divided by mControlsX and mControlsY
	if( mControlsX < resolutionX ) {
		int dx = ( resolutionX - 1 ) % ( mControlsX - 1 );
		if( dx >= ( mControlsX / 2 ) ) dx -= ( mControlsX - 1 );
		resolutionX -= dx;
	}
	else {
		resolutionX = mControlsX;
	}

	if( mControlsY < resolutionY ) {
		int dy = ( resolutionY - 1 ) % ( mControlsY - 1 );
		if( dy >= ( mControlsY / 2 ) ) dy -= ( mControlsY - 1 );
		resolutionY -= dy;
	}
	else {
		resolutionY = mControlsY;
	}

	//
	mResolutionX = resolutionX;
	mResolutionY = resolutionY;

	//
	int numVertices = ( resolutionX * resolutionY );
	int numTris = 2 * ( resolutionX - 1 ) * ( resolutionY - 1 );
	int numIndices = numTris * 3;
    
    if( !mVboMesh ) {
        if( !mVboMesh ) {
            mVboMesh = make_shared<ofVboMesh>();
			mVboMesh->setUsage(GL_STATIC_DRAW);
        }
    }
    
	if( !mVboMesh ) return;

	int width = mWidth;
	int height = mHeight;

	int rows = resolutionX;
	int columns = resolutionY;

	ofVec3f vert;
	ofVec3f normal(0, 0, 1); // always facing forward //
	ofVec2f texcoord;

	mVboMesh->clear();

    //FIX - orientation
    
	// the origin of the plane is the center //
	float halfW = width / 2.f;
	float halfH = height / 2.f;
	// add the vertexes //
	for (int iy = 0; iy < rows; iy++) {
		for (int ix = 0; ix < columns; ix++) {

			// normalized tex coords //
			texcoord.x = ((float)ix / ((float)columns - 1.f));
			texcoord.y = ((float)iy / ((float)rows - 1.f));

            vert.x = texcoord.x * width;
            vert.y = texcoord.y * height;

			mVboMesh->addVertex(vert);
			mVboMesh->addTexCoord(texcoord);
			mVboMesh->addNormal(normal);
		}
	}

	// Triangles //
	for (int y = 0; y < rows - 1; y++) {
		for (int x = 0; x < columns - 1; x++) {
			// first triangle //
			mVboMesh->addIndex((y)*columns + x);
			mVboMesh->addIndex((y)*columns + x + 1);
			mVboMesh->addIndex((y + 1)*columns + x);

			// second triangle //
			mVboMesh->addIndex((y)*columns + x + 1);
			mVboMesh->addIndex((y + 1)*columns + x + 1);
			mVboMesh->addIndex((y + 1)*columns + x);
		}
	}

	mIsDirty = true;
}

void WarpBilinear::updateMesh()
{
	if( !mVboMesh ) return;
	if( !mIsDirty ) return;

	glm::vec2			p;
	float			u, v;
	int				col, row;
    int             index = 0;

	std::vector<glm::vec2>       cols, rows;
	std::vector<glm::vec3>    positions( mResolutionX * mResolutionY );
	std::vector<glm::vec2>    texc(mResolutionX * mResolutionY);


    int ii = 0;

	for( int x = 0; x < mResolutionX; x++ ) {
		for( int y = 0; y < mResolutionY; y++ ) {
            
			// transform coordinates to [0..numControls]
			u = x * ( mControlsX - 1 ) / (float)( mResolutionX - 1 );
			v = y * ( mControlsY - 1 ) / (float)( mResolutionY - 1 );


			// determine col and row
			col = (int)( u );
			row = (int)( v );

			// normalize coordinates to [0..1]
			u -= col;
			v -= row;
            

			if( mIsLinear ) {
				// perform linear interpolation
				glm::vec2 p1 = ( 1.0f - u ) * getPoint( col, row ) + u * getPoint( col + 1, row );
				glm::vec2 p2 = ( 1.0f - u ) * getPoint( col, row + 1 ) + u * getPoint( col + 1, row + 1 );
				p = ( ( 1.0f - v ) * p1 + v * p2 ) * glm::vec2(mWindowSize.x, mWindowSize.y);
			}
			else {
				// perform bicubic interpolation
				rows.clear();
                
                
                
     
				for( int i = -1; i < 3; ++i ) {
					cols.clear();
                    
					for( int j = -1; j < 3; ++j ) {
                        glm::vec2 pt =  getPoint( col + i, row + j );
						cols.push_back( pt );
					}
					rows.push_back( cubicInterpolate( cols, v ) );
				}
                
                
                //cout << p << endl;

                p = cubicInterpolate( rows, u ) * glm::vec2(mWindowSize.x, mWindowSize.y);
                
			}


            //
            float uu = (float)x / ((float)mResolutionX - 1.f);
            float vv = (float)y / ((float)mResolutionY - 1.f);
            
            if (sswitch){
                glm::vec2 p2 = cubicInterpolate( rows, u );
                uu = p2.x;
                vv = p2.y;
            }

            
            if (gswitch){
                p += (p - storedPositions[ii]);
            }
            else {
                storedPositions[ii] = glm::vec2(p.x, p.y);
                p += (p - storedPositions[ii]);
            }
            ii++;

            
            texc[index] = ofVec2f(uu,vv);
            positions[index++] = ofVec3f(p.x, p.y, 0);
		}
	}

	mVboMesh->clearTexCoords();
	mVboMesh->clearVertices();
   
	mVboMesh->addVertices(positions);
	mVboMesh->addTexCoords(texc);

    
	mIsDirty = false;
}

glm::vec2 WarpBilinear::getPoint( int col, int row ) const
{
	int maxCol = mControlsX - 1;
	int maxRow = mControlsY - 1;

	// here's the magic: extrapolate points beyond the edges
	if( col < 0 ) return 2.0f * getPoint( 0, row ) - getPoint( 0 - col, row );
	if( row < 0 ) return 2.0f * getPoint( col, 0 ) - getPoint( col, 0 - row );
	if( col > maxCol ) return 2.0f * getPoint( maxCol, row ) - getPoint( 2 * maxCol - col, row );
	if( row > maxRow ) return 2.0f * getPoint( col, maxRow ) - getPoint( col, 2 * maxRow - row );

	// points on the edges or within the mesh can simply be looked up
	return mPoints[( col * mControlsY ) + row];
}

// from http://www.paulinternet.nl/?page=bicubic : fast catmull-rom calculation
glm::vec2 WarpBilinear::cubicInterpolate( const std::vector<glm::vec2> &knots, float t ) const
{
	assert( knots.size() >= 4 );

	return knots[1] + 0.5f * t*( knots[2] - knots[0] + t*( 2.0f*knots[0] - 5.0f*knots[1] +
														   4.0f*knots[2] - knots[3] + t*( 3.0f*( knots[1] - knots[2] ) + knots[3] - knots[0] ) ) );
}

void WarpBilinear::setNumControlX( int n )
{
	// there should be a minimum of 2 control points
	n = std::max( 2, n );

	// prevent overflow
	if( ( n * mControlsY ) > MAX_NUM_CONTROL_POINTS )
		return;
    
	// create a list of new points
	std::vector<glm::vec2> temp( n * mControlsY );

	// perform spline fitting
	for( int row = 0; row < mControlsY; ++row ) {
		std::vector<glm::vec2> points;
		if( mIsLinear ) {
			// construct piece-wise linear spline
			for( int col = 0; col < mControlsX; ++col ) {
				points.push_back( getPoint( col, row ) );
			}

			cinder::BSpline2f s(points, 1, false, true);

			// calculate position of new control points
			float length = s.getLength( 0.0f, 1.0f );
			float step = 1.0f / ( n - 1 );
			for( int col = 0; col < n; ++col ) {
				temp[( col * mControlsY ) + row] = s.getPosition( s.getTime( length * col * step ) );
			}
		}
		else {
			// construct piece-wise catmull-rom spline
			for( int col = 0; col < mControlsX; ++col ) {
				glm::vec2 p0 = getPoint( col - 1, row );
				glm::vec2 p1 = getPoint( col, row );
				glm::vec2 p2 = getPoint( col + 1, row );
				glm::vec2 p3 = getPoint( col + 2, row );

				// control points according to an optimized Catmull-Rom implementation
				glm::vec2 b1 = p1 + ( p2 - p0 ) / 6.0f;
				glm::vec2 b2 = p2 - ( p3 - p1 ) / 6.0f;

				points.push_back( p1 );

				if( col < ( mControlsX - 1 ) ) {
					points.push_back( b1 );
					points.push_back( b2 );
				}
			}

			cinder::BSpline2f s( points, 3, false, true );

			// calculate position of new control points
			float length = s.getLength( 0.0f, 1.0f );
			float step = 1.0f / ( n - 1 );
			for( int col = 0; col < n; ++col ) {
				temp[( col * mControlsY ) + row] = s.getPosition( s.getTime( length * col * step ) );
			}
		}
	}

    mPoints.clear();
    
	// copy new control points 
	mPoints = temp;
	mControlsX = n;

    mControlPoints.clear();

    for (unsigned i = 0; i < mPoints.size(); i++) {
        queueControlPoint(getControlPoint(i) * ofVec2f(mWidth, mHeight) , (i == mSelected));
    }
    
	mIsDirty = true;
}

void WarpBilinear::setNumControlY( int n )
{
	// there should be a minimum of 2 control points
	n = std::max( 2, n );

	// prevent overflow
	if( ( mControlsX * n ) > MAX_NUM_CONTROL_POINTS )
		return;
    
	// create a list of new points
	std::vector<glm::vec2> temp( mControlsX * n );

	// perform spline fitting
	for( int col = 0; col < mControlsX; col++ ) {
		std::vector<glm::vec2> points;
		if( mIsLinear ) {
			// construct piece-wise linear spline
			for( int row = 0; row < mControlsY; ++row )
				points.push_back( getPoint( col, row ) );

			cinder::BSpline2f s( points, 1, false, true );

			// calculate position of new control points
			float length = s.getLength( 0.0f, 1.0f );
			float step = 1.0f / ( n - 1 );
			for( int row = 0; row < n; ++row ) {
				temp[( col * n ) + row] = s.getPosition( s.getTime( length * row * step ) );
			}
		}
		else {
			// construct piece-wise catmull-rom spline
			for( int row = 0; row < mControlsY; row++ ) {
				glm::vec2 p0 = getPoint( col, row - 1 );
				glm::vec2 p1 = getPoint( col, row );
				glm::vec2 p2 = getPoint( col, row + 1 );
				glm::vec2 p3 = getPoint( col, row + 2 );

				// control points according to an optimized Catmull-Rom implementation
				glm::vec2 b1 = p1 + ( p2 - p0 ) / 6.0f;
				glm::vec2 b2 = p2 - ( p3 - p1 ) / 6.0f;

				points.push_back( p1 );

				if( row < ( mControlsY - 1 ) ) {
					points.push_back( b1 );
					points.push_back( b2 );
				}
			}

			cinder::BSpline2f s( points, 3, false, true );

			// calculate position of new control points
			float length = s.getLength( 0.0f, 1.0f );
			float step = 1.0f / ( n - 1 );
			for( int row = 0; row < n; row++ ) {
				temp[( col * n ) + row] = s.getPosition( s.getTime( length * row * step ) );
			}
		}
	}

    mPoints.clear();
    
	// copy new control points 
	mPoints = temp;
	mControlsY = n;

    mControlPoints.clear();
    
    for (unsigned i = 0; i < mPoints.size(); i++) {
        queueControlPoint(getControlPoint(i) * ofVec2f(mWidth, mHeight) , (i == mSelected));
    }
    
	mIsDirty = true;
}

void WarpBilinear::createShader()
{
	if( mShader )
		return;

    mShader = make_shared<ofShader>();

	static string VertShader = R"END(
		#version 150
        uniform mat4 modelViewMatrix;
        uniform mat4 projectionMatrix;
        uniform mat4 textureMatrix;
        uniform mat4 modelViewProjectionMatrix;
        uniform vec2 uResolution;
  
        in vec4 position;
        in vec4 color;
        in vec4 normal;
        in vec2 texcoord;
		
        out vec2 varyingtexcoord;
        out vec4 texColor;
    
		void main(void) {
            varyingtexcoord = vec2(texcoord.x*uResolution.x, texcoord.y*uResolution.y);
            texColor = color;
            gl_Position = modelViewProjectionMatrix * position;
		}
    )END";

	static string FragShader = R"END(
		#version 150
        uniform sampler2DRect tex0;
        uniform vec4		uExtends;
        uniform vec3		uLuminance;
        uniform vec3		uGamma;
        uniform vec4		uEdges;
        uniform float		uExponent;
        uniform bool		uEditMode;
        uniform vec2		uResolution;       
		uniform float		uBrightness;
		uniform float		uContrast;
		uniform float		uSaturation;

		uniform sampler2DRect	uColorLut;
		uniform float			uMapdim = 256.0;
		uniform bool			uUseColorLut = false;
		const int				interp = 1;
		const float				amt = 1.0;

		//uniform sampler2DRect	uBlendTexture;

		in vec2 varyingtexcoord;
    	in vec4 vertColor;
        out vec4 outputColor;

		const float AvgLumR = 0.5;
		const float AvgLumG = 0.5;
		const float AvgLumB = 0.5;
		const vec3 LumCoeff = vec3 (0.2125, 0.7154, 0.0721);

		vec4 set;
		float rout;
		float gout;	
		float bout;
		vec4 mapped;
		vec4 t2;

		vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con) {
			vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
			vec3 brtColor = color * brt;
			vec3 intensity = vec3(dot(brtColor, LumCoeff));
			vec3 satColor = mix(intensity, brtColor, sat);
			vec3 conColor = mix(AvgLumin, satColor, con);
			return conColor;
		}
       
        float grid( in vec2 uv, in vec2 size )
        {
            vec2 coord = uv / size;
            vec2 grid = abs( fract( coord - 0.5 ) - 0.5 ) / ( 2.0 * fwidth( coord ) );
            float line = min( grid.x, grid.y );
            return 1.0 - min( line, 1.0 );
        }
        
        void main()
        {
            vec4 texColor = texture( tex0, varyingtexcoord );
            vec3 cCSB = ContrastSaturationBrightness(texColor.rgb, uBrightness, uSaturation, uContrast);
            
            texColor = vec4(cCSB.rgb, texColor.a);
            t2 = texColor;
            
			if (uUseColorLut){
				set = mix(floor(t2*uMapdim),t2*uMapdim,float(interp));
				rout = float (texture(uColorLut, vec2(set.r,0.)).r);
				gout = float (texture(uColorLut, vec2(set.g,0.)).g);
				bout = float (texture(uColorLut, vec2(set.b,0.)).b);
				mapped = vec4 (rout, gout, bout,t2.a);
				t2 = mix(t2, mapped, amt);
        
				set = mix(floor(t2*uMapdim),t2*uMapdim,float(interp));
				rout = float (texture(uColorLut, vec2(set.r,0.)).a);
				gout = float (texture(uColorLut, vec2(set.g,0.)).a);
				bout = float (texture(uColorLut, vec2(set.b,0.)).a);
				mapped = vec4 (rout, gout, bout,t2.a);
				texColor = mix(t2, mapped, amt);
			}

            float a = 1.0;
            if( uEdges.x > 0.0 ) a *= clamp( varyingtexcoord.x/uResolution.x / uEdges.x, 0.0, 1.0 );
            if( uEdges.y > 0.0 ) a *= clamp( varyingtexcoord.y/uResolution.y / uEdges.y, 0.0, 1.0 );
            if( uEdges.z > 0.0 ) a *= clamp( ( 1.0 - varyingtexcoord.x/uResolution.x ) / uEdges.z, 0.0, 1.0 );
            if( uEdges.w > 0.0 ) a *= clamp( ( 1.0 - varyingtexcoord.y/uResolution.y ) / uEdges.w, 0.0, 1.0 );
            
            const vec3 one = vec3( 1.0 );
            vec3 blend = ( a < 0.5 ) ? ( uLuminance * pow( 2.0 * a, uExponent ) )
            : one - ( one - uLuminance ) * pow( 2.0 * ( 1.0 - a ), uExponent );
   			
			//blend = texture(uBlendTexture, varyingtexcoord).rgb;
			texColor.rgb *= pow( blend, one / uGamma );
          
            if( uEditMode ) {
                vec2 v = vec2(varyingtexcoord.x/uResolution.x, varyingtexcoord.y/uResolution.y);
                float f = grid(v.xy * uExtends.xy, uExtends.zw );
                vec4 gridColor = vec4( 1 );
                outputColor = mix( texColor, gridColor, f );
            }
            else {
                outputColor = texColor;
            }
        }
    
	)END";
	try {
        mShader->setupShaderFromSource(GL_VERTEX_SHADER, VertShader);
        mShader->setupShaderFromSource(GL_FRAGMENT_SHADER, FragShader);
        mShader->bindDefaults();
        mShader->linkProgram();

	}
	catch( const std::exception &e ) {
		cout << e.what() << std::endl;
	}
}

ofRectangle WarpBilinear::getMeshBounds() const
{
	ofVec2f min = ofVec2f( 1 );
	ofVec2f max = ofVec2f( 0 );

	for( unsigned i = 0; i < mPoints.size(); ++i ) {
		min.x = std::min( mPoints[i].x, min.x );
		min.y = std::min( mPoints[i].y, min.y );
		max.x = std::max( mPoints[i].x, max.x );
		max.y = std::max( mPoints[i].y, min.y );
	}

	return ofRectangle( min * mWindowSize, max * mWindowSize );
}

void WarpBilinear::setTexCoords( float x1, float y1, float x2, float y2 )
{
	mIsDirty |= ( x1 != mX1 || y1 != mY1 || x2 != mX2 || y2 != mY2 );
	if( !mIsDirty ) return;

	mX1 = x1;
	mY1 = y1;
	mX2 = x2;
	mY2 = y2;
}
