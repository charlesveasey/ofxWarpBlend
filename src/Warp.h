/*
Original work Copyright (c) 2010-2015, Paul Houx - All rights reserved.
Modified work Copyright (c) 2015-2016, Charles Veasey - All rights reserved.

This code is intended for use with the openFrameworks C++ library: http://openframeworks.cc/

This file is part of ofxWarpBlend.

ofxWarping is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ofxWarping is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ofxWarping.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "ofMain.h"
#include <atomic>
#include <vector>
#include "glm/glm.hpp"

typedef std::shared_ptr<class Warp>			WarpRef;
typedef std::vector<WarpRef>				WarpList;
typedef WarpList::iterator					WarpIter;
typedef WarpList::const_iterator			WarpConstIter;
typedef WarpList::reverse_iterator			WarpReverseIter;
typedef WarpList::const_reverse_iterator	WarpConstReverseIter;

class Warp : public std::enable_shared_from_this <Warp> {
	public:
		typedef enum { UNKNOWN, BILINEAR, PERSPECTIVE, PERSPECTIVE_BILINEAR } WarpType;
	public:
		Warp(WarpType type = UNKNOWN);
		virtual ~Warp(void);
    
		//!
		bool				isEditModeEnabled() { return (bool)sIsEditMode; };
		void				enableEditMode(bool enabled = true) { sIsEditMode = enabled; };
		void				disableEditMode() { sIsEditMode = false; };
		void				toggleEditMode() { sIsEditMode = !sIsEditMode; };

		//! returns the type of the warp
		WarpType			getType() const { return mType; };

		//! returns a shared pointer to this warp
		WarpRef				getPtr() { return shared_from_this(); }

		//!
		virtual ofXml		toXml() const;
		//!
		virtual void		fromXml(ofXml &xml);

		//! get the width of the content in pixels
		int					getWidth() const { return mWidth; };
		//! get the height of the content in pixels
		int					getHeight() const { return mHeight; };
		//! get the width and height of the content in pixels
		glm::vec2				getSize() const { return glm::vec2(mWidth, mHeight); };
		//! get the width and height of the content in pixels
		ofRectangle			getBounds() const { return ofRectangle(0, 0, mWidth, mHeight); };

		//! set the width of the content in pixels
		virtual void		setWidth(int w) { setSize(w, mHeight); }
		//! set the height of the content in pixels
		virtual void		setHeight(int h) { setSize(mWidth, h); }
		//! set the width and height of the content in pixels
		virtual void		setSize(const glm::vec2 &size) { setSize(size.x, size.y); }
		//! set the width and height of the content in pixels
		virtual void		setSize(int w, int h);

		//! returns the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		virtual const ofVec3f& getLuminance() const { return mLuminance; }
		//! set the luminance value for all color channels, used for edge blending (0.5 = linear)
		virtual void		setLuminance(float gamma) { mLuminance = ofVec3f(gamma); }
		//! set the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear)
		virtual void		setLuminance(float red, float green, float blue) { mLuminance.x = red; mLuminance.y = green; mLuminance.z = blue; }

		//! returns the gamma curve value for the red, green and blue channels
		virtual const ofVec3f& getGamma() const { return mGamma; }
		//! set the gamma curve value for all color channels
		virtual void		setGamma(float gamma) { mGamma = ofVec3f(gamma); }
		//! set the gamma curve value for the red, green and blue channels
		virtual void		setGamma(float red, float green, float blue) { mGamma.x = red; mGamma.y = green; mGamma.z = blue; }

		//! returns the edge blending curve exponent (1.0 = linear, 2.0 = quadratic)
		virtual float		getExponent() const { return mExponent; }
		//! set the edge blending curve exponent  (1.0 = linear, 2.0 = quadratic)
		virtual void		setExponent(float e) { mExponent = ofClamp(e, 1.0f, 100.0f); }

		//! returns the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		virtual ofVec3f	getEdges() const { return 2.0f * mEdges; }
		//! set the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		virtual void		setEdges(float left, float top, float right, float bottom)
		{
			mEdges.x = ofClamp(left * 0.5f, 0.0f, 1.0f);
			mEdges.y = ofClamp(top * 0.5f, 0.0f, 1.0f);
			mEdges.z = ofClamp(right * 0.5f, 0.0f, 1.0f);
			mEdges.w = ofClamp(bottom * 0.5f, 0.0f, 1.0f);
		}
		//! set the edge blending area for the left, top, right and bottom edges (values between 0 and 1)
		virtual void		setEdges(const ofVec4f &edges)
		{
			mEdges.x = ofClamp(edges.x * 0.5f, 0.0f, 1.0f);
			mEdges.y = ofClamp(edges.y * 0.5f, 0.0f, 1.0f);
			mEdges.z = ofClamp(edges.z * 0.5f, 0.0f, 1.0f);
			mEdges.w = ofClamp(edges.w * 0.5f, 0.0f, 1.0f);
		}
        //! set the brighntess (values between 0 and 1)
        virtual void        setBrightness(float b){ mBrightness = b; }
        //! set the contrast (values between 0 and 1)
        virtual void        setContrast(float b){ mContrast = b; }
        //! set the saturation (values between 0 and 1)
        virtual void        setSaturation(float b){ mSaturation = b; }
        //! set the color lut texture reference
        virtual void        setLutTexture(ofTexture& b){ lutTexture = b; sUseColorLut = true; }
		//! set the blend lut texture reference
		virtual void        setBlendTexture(ofTexture& b) { blendTexture = b; }
		//! toggle mapping linear or curve (default)
		virtual void				toggleMappingMode() { }


		//! reset control points to undistorted image
		virtual void		reset() = 0;
		//! setup the warp before drawing its contents
		virtual void		begin() = 0;
		//! restore the warp after drawing
		virtual void		end() = 0;
    
    
        virtual void		bind() = 0;
        virtual void		unbind() = 0;


		//! draws a warped texture
        void                draws();
    

    
		void				draw(const ofTexture &texture);
		//! draws a specific area of a warped texture
		void				draw(const ofTexture &texture, const ofRectangle &srcArea);
		//! draws a specific area of a warped texture to a specific region
		virtual void		draw(const ofTexture &texture, const ofRectangle &srcArea, const ofRectangle &destRect) = 0;

		//! adjusts both the source area and destination rectangle so that they are clipped against the warp's content
		bool				clip(ofRectangle &srcArea, ofRectangle &destRect) const;

		//! returns the coordinates of the specified control point
		virtual glm::vec2		getControlPoint(unsigned index) const;
		//! sets the coordinates of the specified control point
		virtual void		setControlPoint(unsigned index, const glm::vec2 &pos);
		//! moves the specified control point 
		virtual void		moveControlPoint(unsigned index, const glm::vec2 &shift);
		//! select one of the control points
		virtual void		selectControlPoint(unsigned index);
		//! deselect the selected control point
		virtual void		deselectControlPoint();
		//! returns the index of the closest control point, as well as the distance in pixels
		virtual unsigned	findControlPoint(const glm::vec2 &pos, float *distance) const;

		//! set the width and height in pixels of the content of all warps
		static void			setSize(const WarpList &warps, int w, int h);
		//! set the width and height in pixels of the content of all warps
		static void			setSize(const WarpList &warps, const glm::vec2 &size) { setSize(warps, size.x, size.y); }

		//! checks all warps and selects the closest control point
		static void			selectClosestControlPoint(const WarpList &warps, const glm::vec2 &position);

		//! draw a control point in the correct preset color
		void				queueControlPoint(const glm::vec2 &pt, bool selected = false, bool attached = false);
		//! draw a control point in the specified color
		void				queueControlPoint(const glm::vec2 &pt, const ofColor &clr, float scale = 1.0f);

		//! read a settings xml file and pass back a vector of Warps
		static WarpList		readSettings(const string source);
		//! write a settings xml file
		static void			writeSettings(const WarpList &warps, string &target);

		//! handles mouseMove events for multiple warps
		static bool			handleMouseMove(WarpList &warps, ofMouseEventArgs &event);
		//! handles mouseDown events for multiple warps
		static bool			handleMouseDown(WarpList &warps, ofMouseEventArgs &event);
		//! handles mouseDrag events for multiple warps
		static bool			handleMouseDrag(WarpList &warps, ofMouseEventArgs &event);
		//! handles mouseUp events for multiple warps
		static bool			handleMouseUp(WarpList &warps, ofMouseEventArgs &event);

		//! handles keyDown events for multiple warps
		static bool			handleKeyDown(WarpList &warps, ofKeyEventArgs &event);
		//! handles keyUp events for multiple warps
		static bool			handleKeyUp(WarpList &warps, ofKeyEventArgs &event);

		//! handles resize events for multiple warps
		static bool			handleResize(WarpList &warps);

		virtual void		mouseMove(ofMouseEventArgs &event);
		virtual void		mouseDown(ofMouseEventArgs &event);
		virtual void		mouseDrag(ofMouseEventArgs &event);
		virtual void		mouseUp(ofMouseEventArgs &event);

		virtual void		keyDown(ofKeyEventArgs &event);
		virtual void		keyUp(ofKeyEventArgs &event);

		virtual void		resize();
    
    
        inline ofMatrix4x4 toOf(const glm::mat4 &_m) {
            return ofMatrix4x4(&_m[0][0]);
        }
    
	protected:
		//! draw the warp and its editing interface
		virtual void		draw(bool controls = true) = 0;
		//! draw the control points
		void				drawControlPoints();

	protected:
		WarpType		mType;

		bool			mIsDirty;

		int				mWidth;
		int				mHeight;
		glm::vec2			mWindowSize;

		float			mBrightness;
        float           mContrast;
        float           mSaturation;
        ofTexture&      lutTexture = (*new ofTexture());
		ofTexture&      blendTexture = (*new ofTexture());

		unsigned		mSelected;

		//! Determines the number of horizontal and vertical control points
		int				mControlsX;
		int				mControlsY;

		std::vector<glm::vec2>	mPoints;

		//! edge blending parameters
		ofVec3f			mLuminance;
		ofVec3f			mGamma;
		ofVec4f			mEdges;
		float			mExponent;

		//! time of last control point selection
		double				mSelectedTime;
		//! keep track of mouse position
		mutable glm::vec2	mMouse;

		static const int	MAX_NUM_CONTROL_POINTS = 1024;

	protected:
		glm::vec2			mOffset;

		//! instanced control points
		typedef struct Data {
			glm::vec2	 position;
			float    scale;
			float    reserved;
			ofVec4f	 color;

			Data() {}
			Data(const glm::vec2 &pt, const ofVec4f &clr, float scale) : position(pt), scale(scale), color(clr) {}
		} Data;

		std::vector<Data>	mControlPoints;

		//! edit mode for all warps
		bool	sIsEditMode = false;
		bool	sUseColorLut = false;
	};

	// ----------------------------------------------------------------------------------------------------------------

	typedef std::shared_ptr<class WarpBilinear>	WarpBilinearRef;

	class WarpBilinear
		: public Warp {
	public:
		//
		static WarpBilinearRef create() { return std::make_shared<WarpBilinear>(); }

	public:
		WarpBilinear();
		virtual ~WarpBilinear(void);

            vector<glm::vec2> storedPositions;

            
		//! returns a shared pointer to this warp
		WarpBilinearRef	getPtr(const ofFbo::Settings &format) { return std::static_pointer_cast<WarpBilinear>(shared_from_this()); }

		//!
		virtual ofXml		toXml() const override;
		//!
		virtual void		fromXml(ofXml &xml) override;

		//! set the width and height of the content in pixels
		void				setSize(int w, int h) override { Warp::setSize(w, h); if (mFbo) mFbo->clear(); }
		//! set the frame buffer format, so you have control over its quality settings
		void				setFormat(const ofFbo::Settings &format) { mFboFormat = format; mFbo->clear();  }
		//!
		void				setLinear(bool enabled = true) { mIsLinear = enabled; mIsDirty = true; };
		void				setCurved(bool enabled = true) { mIsLinear = !enabled; mIsDirty = true; };
		void				toggleMappingMode() { mIsLinear = !mIsLinear; mIsDirty = true; }

		//! reset control points to undistorted image
		virtual void		reset() override;
		//! setup the warp before drawing its contents
		virtual void		begin() override;
		//! restore the warp after drawing
		virtual void		end() override;
            
            virtual void bind() override;
            virtual void unbind() override;

		//! draws a warped texture
		virtual void		draw(const ofTexture &texture, const ofRectangle &srcArea, const ofRectangle &destRect) override;

		//! set the number of horizontal control points for this warp 
		void				setNumControlX(int n);
		//! set the number of vertical control points for this warp
		void				setNumControlY(int n);

		void				setTexCoords(float x1, float y1, float x2, float y2);

		virtual void		keyDown(ofKeyEventArgs &event) override;
	protected:
		//! draws the warp as a mesh, allowing you to use your own texture instead of the FBO
		virtual void		draw(bool controls = true) override;
		//! Creates the shader that renders the content with a wireframe overlay
		void				createShader();
		//! Creates the frame buffer object and updates the vertex buffer object if necessary
		void				createBuffers();
		//! Creates the vertex buffer object
		void				createMesh(int resolutionX = 36, int resolutionY = 36);
		//! Updates the vertex buffer object based on the control points
		void				updateMesh();
		//!	Returns the specified control point. Values for col and row are clamped to prevent errors.
		glm::vec2				getPoint(int col, int row) const;
		//! Performs fast Catmull-Rom interpolation, returns the interpolated value at t
		glm::vec2				cubicInterpolate(const std::vector<glm::vec2> &knots, float t) const;
		//!
		ofRectangle			getMeshBounds() const;
	private:
		//! Greatest common divisor using Euclidian algorithm (from: http://en.wikipedia.org/wiki/Greatest_common_divisor)
		inline int			gcd(int a, int b) const { if (b == 0) return a; else return gcd(b, a%b); };
        inline ofVec3f      normalFrom3Points(ofVec3f p0, ofVec3f p1, ofVec3f p2);
    
	public:
		shared_ptr<ofFbo>					mFbo;
		ofFbo::Settings						mFboFormat;
		shared_ptr<ofVboMesh>				mVboMesh;
		shared_ptr<ofShader>				mShader;

		//! linear or curved interpolation
		bool					mIsLinear;
		//!
		bool					mIsAdaptive;

		//! texture coordinates of corners
		float					mX1, mY1, mX2, mY2;

		//! Determines the detail of the generated mesh. Multiples of 5 seem to work best.
		int						mResolution;

		//! Determines the number of horizontal and vertical quads 
		int						mResolutionX;
		int						mResolutionY;
	};

	// ----------------------------------------------------------------------------------------------------------------

	typedef std::shared_ptr<class WarpPerspective> WarpPerspectiveRef;

	class WarpPerspective
		: public Warp {
	public:
		//
		static WarpPerspectiveRef create() { return std::make_shared<WarpPerspective>(); }

	public:
		WarpPerspective(void);
		virtual ~WarpPerspective(void);

		//! returns a shared pointer to this warp
		WarpPerspectiveRef	getPtr() { return std::static_pointer_cast<WarpPerspective>(shared_from_this()); }

		//! get the transformation matrix
		glm::mat4x4		getTransform();
		//! get the inverted transformation matrix
		glm::mat4x4		getInvertedTransform() { return mInverted; }

		//! reset control points to undistorted image
		void			reset() override;
		//! setup the warp before drawing its contents
		void			begin() override;
		//! restore the warp after drawing
		void			end() override;
            
            void			bind() override;
            void			unbind() override;


		//! draws a warped texture
		void			draw(const ofTexture &texture, const ofRectangle &srcArea, const ofRectangle &destRect) override;

		//! override keyDown method to add additional key handling
		void			keyDown(ofKeyEventArgs &event) override;

		//! allow WarpPerspectiveBilinear to access the protected class members
		friend class WarpPerspectiveBilinear;
	protected:
		//!
		void	draw(bool controls = true) override;

		//! find homography based on source and destination quad
		glm::mat4x4 getPerspectiveTransform(const glm::vec2 src[4], const glm::vec2 dst[4]) const;
		//! helper function
		void gaussianElimination(float * input, int n) const;

		//!
		void createShader();

	protected:
		glm::vec2		mSource[4];
		glm::vec2		mDestination[4];

		glm::mat4x4	mTransform;
		glm::mat4x4	mInverted;

		shared_ptr<ofShader>	mShader;
	};

	// ----------------------------------------------------------------------------------------------------------------

	typedef std::shared_ptr<class WarpPerspectiveBilinear>	WarpPerspectiveBilinearRef;

	class WarpPerspectiveBilinear
		: public WarpBilinear {
	public:
		//
		static WarpPerspectiveBilinearRef create() {  return std::make_shared<WarpPerspectiveBilinear>();  }

	public:
		WarpPerspectiveBilinear();
		virtual ~WarpPerspectiveBilinear(void);

		//! returns a shared pointer to this warp
		WarpPerspectiveBilinearRef	getPtr() { return std::static_pointer_cast<WarpPerspectiveBilinear>(shared_from_this()); }

		//!
		ofXml	toXml() const override;
		//!
		void		fromXml(ofXml &xml) override;

		void		mouseMove(ofMouseEventArgs &event) override;
		void		mouseDown(ofMouseEventArgs &event) override;
		void		mouseDrag(ofMouseEventArgs &event) override;

		void		keyDown(ofKeyEventArgs &event) override;

		void		resize() override;

		//! set the width and height of the content in pixels
		void		setSize(int w, int h) override;

		//! returns the coordinates of the specified control point
		glm::vec2	getControlPoint(unsigned index) const override;
		//! sets the coordinates of the specified control point
		void		setControlPoint(unsigned index, const glm::vec2 &pos) override;
		//! moves the specified control point 
		void		moveControlPoint(unsigned index, const glm::vec2 &shift) override;
		//! select one of the control points
		void		selectControlPoint(unsigned index) override;
		//! deselect the selected control point
		void		deselectControlPoint() override;
	protected:
		//! 
		void		draw(bool controls = true) override;

		//! returns whether or not the control point is one of the 4 corners and should be treated as a perspective control point
		bool		isCorner(unsigned index) const;
		//! converts the control point index to the appropriate perspective warp index
		unsigned	convertIndex(unsigned index) const;

	protected:
		WarpPerspectiveRef	mWarp;
	};