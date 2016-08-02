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

Warp::Warp(WarpType type)
	: mType(type)
	, mIsDirty(true)
	, mWidth(640)
	, mHeight(480)
	, mSaturation(1)
	, mContrast(1)
	, mBrightness(1)
	, mSelected(-1) // since this is an unsigned int, actual value will be 'MAX_INTEGER'
	, mControlsX(2)
	, mControlsY(2)
	, mLuminance(0.5f)
	, mGamma(1.0f)
	, mEdges(0.0f)
	, mExponent(2.0f)
	, mSelectedTime(0)
{
	mWindowSize = glm::vec2(float(mWidth), float(mHeight));
}

Warp::~Warp(void)
{
}

void Warp::draws()
{
    draw(true);
}

void Warp::draw(const ofTexture &texture)
{
	draw(texture, ofRectangle(0,0,texture.getWidth(), texture.getHeight()), ofRectangle(getBounds()));
}

void Warp::draw(const ofTexture &texture, const ofRectangle &srcArea)
{
	draw(texture, srcArea, ofRectangle(getBounds()));
}

bool Warp::clip(ofRectangle &srcArea, ofRectangle &destRect) const
{
	bool clipped = false;

	float x1 = destRect.x / mWidth;
	float x2 = destRect.x + destRect.width / mWidth;
	float y1 = destRect.y / mHeight;
	float y2 = destRect.y + destRect.height / mHeight;

	//FIX
	if (x1 < 0.0f) {
		destRect.x = 0.0f;
		srcArea.x -= static_cast<int32_t>(x1 * srcArea.getWidth());
		clipped = true;
	}
	else if (x1 > 1.0f) {
		destRect.x = static_cast<float>(mWidth);
		srcArea.x -= static_cast<int32_t>((1.0f / x1) * srcArea.getWidth());
		clipped = true;
	}

	if (x2 < 0.0f) {
		destRect.width = 0.0f;
		srcArea.width -= static_cast<int32_t>(x2 * srcArea.getWidth());
		clipped = true;
	}
	else if (x2 > 1.0f) {
		destRect.width = static_cast<float>(mWidth);
		srcArea.width -= static_cast<int32_t>((1.0f / x2) * srcArea.getWidth());
		clipped = true;
	}

	if (y1 < 0.0f) {
		destRect.y = 0.0f;
		srcArea.y -= static_cast<int32_t>(y1 * srcArea.getHeight());
		clipped = true;
	}
	else if (y1 > 1.0f) {
		destRect.y = static_cast<float>(mHeight);
		srcArea.y -= static_cast<int32_t>((1.0f / y1) * srcArea.getHeight());
		clipped = true;
	}

	if (y2 < 0.0f) {
		destRect.height = 0.0f;
		srcArea.height -= static_cast<int32_t>(y2 * srcArea.getHeight());
		clipped = true;
	}
	else if (y2 > 1.0f) {
		destRect.height = static_cast<float>(mHeight);
		srcArea.height -= static_cast<int32_t>((1.0f / y2) * srcArea.getHeight());
		clipped = true;
	}

	return clipped;
}

ofXml Warp::toXml() const
{
	ofXml xml;
    xml.addChild("warp");
	xml.setTo("warp");
	
    switch (mType) {
        case BILINEAR: xml.setAttribute("method", "bilinear"); break;
        case PERSPECTIVE: xml.setAttribute("method", "perspective"); break;
        case PERSPECTIVE_BILINEAR: xml.setAttribute("method", "perspectivebilinear"); break;
        default: xml.setAttribute("method", "unknown"); break;
	}
    
	xml.setAttribute("width", ofToString(mControlsX) );
	xml.setAttribute("height", ofToString(mControlsY) );
	xml.setAttribute("brightness", ofToString(mBrightness) );
    xml.setAttribute("contrast", ofToString(mContrast) );
    xml.setAttribute("saturation", ofToString(mSaturation) );

    int index = 0;
	// add <controlpoint> tags (column-major)
	std::vector<glm::vec2>::const_iterator itr;
	for (itr = mPoints.begin(); itr != mPoints.end(); ++itr) {
        string s = ofToString(index);
        xml.addChild("controlpoint");
		xml.setTo("controlpoint["+s+"]");
		xml.setAttribute("x", ofToString( (*itr).x) );
		xml.setAttribute("y", ofToString( (*itr).y) );
        xml.setToParent();
        index++;
	}
    
	// add <blend> parameters
    xml.addChild("blend");
	xml.setTo("blend");
	xml.setAttribute("exponent", ofToString(mExponent) );
    if (this->getType() == BILINEAR){
        WarpBilinear* b =  (WarpBilinear*)this;
        xml.setAttribute("resolution", ofToString(b->mResolution));
        xml.setAttribute("linear", ofToString(b->mIsLinear));
        xml.setAttribute("adaptive", ofToString(b->mIsAdaptive));
    }
    
    xml.addChild("edges");
		xml.setTo("edges");
		xml.setAttribute("left", ofToString(mEdges.x) );
		xml.setAttribute("top", ofToString(mEdges.y) );
		xml.setAttribute("right", ofToString(mEdges.z) );
		xml.setAttribute("bottom", ofToString(mEdges.w) );
        
    xml.setToParent();
    xml.addChild("gamma");
		xml.setTo("gamma");
		xml.setAttribute("red", ofToString(mGamma.x) );
		xml.setAttribute("green", ofToString(mGamma.y) );
		xml.setAttribute("blue", ofToString(mGamma.z) );

    xml.setToParent();
    xml.addChild("luminance");
		xml.setTo("luminance");
		xml.setAttribute("red", ofToString(mLuminance.x) );
		xml.setAttribute("green", ofToString(mLuminance.y) );
		xml.setAttribute("blue", ofToString(mLuminance.z) );
      
	xml.setToParent();
	return xml;
}

void Warp::fromXml(ofXml &xml)
{
	mControlsX = ofToInt( xml.getAttribute("width") );
	mControlsY = ofToInt( xml.getAttribute("height") );

	mBrightness = ofToFloat(xml.getAttribute("brightness"));
	mContrast = ofToFloat(xml.getAttribute("contrast"));
	mSaturation = ofToFloat(xml.getAttribute("saturation"));
	
	// load control points
	mPoints.clear();
    
    int controlChildren = xml.getNumChildren();
    
    for (int i=0; i<controlChildren; i++){
        xml.setToChild(i);
        
        if (xml.getName() == "controlpoint"){
            float x = ofToFloat( xml.getAttribute("x") );
            float y = ofToFloat( xml.getAttribute("y") );
            mPoints.push_back( glm::vec2(x, y) );
        }
        
        if (xml.getName() == "blend"){
            mExponent = ofToFloat( xml.getAttribute("exponent") );

            if (this->getType() == BILINEAR){
                WarpBilinear* b =  (WarpBilinear*)this;
                b->mResolution = ofToInt( xml.getAttribute("resolution") );
                b->mIsLinear = ofToBool( xml.getAttribute( "linear" ) );
                b->mIsAdaptive = ofToBool( xml.getAttribute( "adaptive" ) );
            }
            
			int blendControlChildren = xml.getNumChildren();

			for (int j = 0; j < blendControlChildren; j++) {
				xml.setToChild(j);

				if (xml.getName() == "edges") {
					mEdges.x = ofToFloat(xml.getAttribute("left"));
					mEdges.y = ofToFloat(xml.getAttribute("top"));
					mEdges.z = ofToFloat(xml.getAttribute("right"));
					mEdges.w = ofToFloat(xml.getAttribute("bottom"));
				}

				if (xml.getName() == "gamma"){
					mGamma.x = ofToFloat(xml.getAttribute("red"));
					mGamma.y = ofToFloat(xml.getAttribute("green"));
					mGamma.z = ofToFloat(xml.getAttribute("blue"));
				}	

				if (xml.getName() == "luminance") {
					mLuminance.x = ofToFloat(xml.getAttribute("red"));
					mLuminance.y = ofToFloat(xml.getAttribute("green"));
					mLuminance.z = ofToFloat(xml.getAttribute("blue"));
				}
				xml.setToParent();
			}
        }
       
        xml.setToParent();
    }
    
	// reconstruct warp
	mIsDirty = true;
}

void Warp::setSize(int w, int h)
{
	mWidth = w;
	mHeight = h;
    mWindowSize = ofVec2f(float(w), float(h));
	mIsDirty = true;
}

glm::vec2 Warp::getControlPoint(unsigned index) const
{
	if (index >= mPoints.size()) return glm::vec2(0);
	return glm::vec2(mPoints[index].x, mPoints[index].y);
}

void Warp::setControlPoint(unsigned index, const glm::vec2 &pos)
{
	if (index >= mPoints.size()) return;
    mPoints[index].x = pos.x;
    mPoints[index].y = pos.y;

	mIsDirty = true;
}

void Warp::moveControlPoint(unsigned index, const glm::vec2 &shift)
{
    glm::vec2 s = glm::vec2(shift.x, shift.y);
    
	if (index >= mPoints.size()) return;
	mPoints[index] += s;

	mIsDirty = true;
}

void Warp::selectControlPoint(unsigned index)
{
	if (index >= mPoints.size() || index == mSelected) return;

	mSelected = index;
	mSelectedTime = ofGetElapsedTimef();
}

void Warp::deselectControlPoint()
{
	mSelected = -1; // since this is an unsigned int, actual value will be 'MAX_INTEGER'
}

unsigned Warp::findControlPoint(const glm::vec2 &pos, float *distance) const
{
	unsigned index;

	// store mouse position for later use in e.g. WarpBilinear::keyDown().
	mMouse = pos;

	// find closest control point
	float dist = 10.0e6f;

	for (unsigned i = 0; i < mPoints.size(); i++) {

		ofVec2f m;
		m.set(getControlPoint(i) * mWindowSize);

		float d = ofDist(pos.x, pos.y, m.x, m.y);

		if (d < dist) {
			dist = d;
			index = i;
		}
	}

	*distance = dist;

	return index;
}

void Warp::selectClosestControlPoint(const WarpList &warps, const glm::vec2 &position)
{
	WarpRef		warp;
	unsigned	i, index;
	float		d, distance = 10.0e6f;

	// find warp and distance to closest control point
	for (WarpConstReverseIter itr = warps.rbegin(); itr != warps.rend(); ++itr) {
		i = (*itr)->findControlPoint(position, &d);

		if (d < distance) {
			distance = d;
			index = i;
			warp = *itr;
		}
	}

	// select the closest control point and deselect all others
	for (WarpConstIter itr = warps.begin(); itr != warps.end(); ++itr) {
		if (*itr == warp)
			(*itr)->selectControlPoint(index);
		else
			(*itr)->deselectControlPoint();
	}
}

void Warp::setSize(const WarpList &warps, int w, int h)
{
	for (WarpConstIter itr = warps.begin(); itr != warps.end(); ++itr)
		(*itr)->setSize(w, h);
}

WarpList Warp::readSettings(const string source) // FIX Datasource Ref
{
	ofXml		doc;
	WarpList	warps;

	// try to load the specified xml file
    try { doc.load(source); }
	catch (...) { return warps; }

	// check if this is a valid file
	//bool isWarp = doc.exists("warpconfig");
    
    
	//if (!isWarp)
    //    return warps;

	//
	//if (isWarp) {
		// get first profile
 
    
    doc.setToChild(0);
    int mapChilden = doc.getNumChildren();
    
    for(int i=0; i<mapChilden; i++){
        doc.setToChild(i); //map
        doc.setToChild(0); //warp

        string method = doc.getAttribute("method");
        
        if (method == "bilinear") {
            WarpBilinearRef warp(new WarpBilinear());
            warp->fromXml(doc);
            warps.push_back(warp);
        }
        else if (method == "perspective") {
            WarpPerspectiveRef warp(new WarpPerspective());
            warp->fromXml(doc);
            warps.push_back(warp);
        }
        else if (method == "perspectivebilinear") {
            WarpPerspectiveBilinearRef warp(new WarpPerspectiveBilinear());
            warp->fromXml(doc);
            warps.push_back(warp);
        }
        
        doc.setToParent(2);
    }
    
	return warps;
}

void Warp::writeSettings(const WarpList &warps, string &target)
{
    // create config document and root <warpconfig>
    ofXml doc;
    doc.addChild("warpconfig");
    doc.setTo("warpconfig");
    doc.setAttribute("version", "1.0");
    doc.setAttribute("profile", "default");
    
	// create default <profile> (profiles are not yet supported)
    doc.addChild("profile");
	doc.setTo("profile");
	doc.setAttribute("name", "default");

	// 
	for (unsigned i = 0; i < warps.size(); i++) {
		// create <map>
        doc.addChild("map");
        doc.setToChild(i);
		doc.setAttribute("id", ofToString(i + 1) );
		doc.setAttribute("display", ofToString(1) );	// not supported yet

		// create <warp>
        ofXml xml = warps[i]->toXml();
        doc.addXml(xml, true);
        doc.setToParent();
	}
    
	// write file
	doc.save(target);
}

bool Warp::handleMouseMove(WarpList &warps, ofMouseEventArgs &event)
{
	// find and select closest control point
	selectClosestControlPoint(warps, event);

	return false;
}

//FIX
bool Warp::handleMouseDown(WarpList &warps, ofMouseEventArgs &event)
{
	// find and select closest control point
	selectClosestControlPoint(warps, event);

	for (WarpReverseIter itr = warps.rbegin(); itr != warps.rend()&& (event.button != -1); ++itr)
		(*itr)->mouseDown(event);

    return ((event.button == -1) ? true : false);
}

bool Warp::handleMouseDrag(WarpList &warps, ofMouseEventArgs &event)
{
	for (WarpReverseIter itr = warps.rbegin(); itr != warps.rend() && (event.button != -1); ++itr)
		(*itr)->mouseDrag(event);

    return ((event.button == -1) ? true : false);
}

bool Warp::handleMouseUp(WarpList &warps, ofMouseEventArgs &event)
{
	return false;
}

bool Warp::handleKeyDown(WarpList &warps, ofKeyEventArgs &event)
{
	for (WarpReverseIter itr = warps.rbegin(); itr != warps.rend() && (event.key != -1); ++itr)
		(*itr)->keyDown(event);
    
	switch (event.keycode) {
	case OF_KEY_UP:
	case OF_KEY_DOWN:
	case OF_KEY_LEFT:
	case OF_KEY_RIGHT:
		// do not select another control point
		break;
	}

    return ((event.key == -1) ? true : false);
}

bool Warp::handleKeyUp(WarpList &warps, ofKeyEventArgs &event)
{
	return false;
}

bool Warp::handleResize(WarpList &warps)
{
    // FIX
	//for (WarpIter itr = warps.begin(); itr != warps.end(); ++itr)
	//	(*itr)->resize();

	return false;
}

void Warp::mouseMove(ofMouseEventArgs &event)
{
	float distance;
	mSelected = findControlPoint(event, &distance);
}

void Warp::mouseDown(ofMouseEventArgs &event)
{
	if (mSelected >= mPoints.size()) return;

	// calculate offset by converting control point from normalized to standard screen space
	ofVec2f p = (getControlPoint(mSelected) * mWindowSize);
	mOffset = event - p;

    event.button = -1;
}

void Warp::mouseDrag(ofMouseEventArgs &event)
{
	if (mSelected >= mPoints.size()) return;

	ofVec2f m(event);
	ofVec2f p(m.x - mOffset.x, m.y - mOffset.y);

	// set control point in normalized screen space
	setControlPoint(mSelected, p / mWindowSize);
    
	mIsDirty = true;

    event.button = -1;
}

void Warp::mouseUp(ofMouseEventArgs &event)
{
}

void Warp::keyDown(ofKeyEventArgs &event)
{
	// disable keyboard input when not in edit mode
	if (sIsEditMode) {
		if (event.keycode == OF_KEY_ESC) {
			// gracefully exit edit mode 
			sIsEditMode = false;
            event.key = -1;
			return;
		}
	}
	else return;

	// do not listen to key input if not selected
	if (mSelected >= mPoints.size()) return;

	switch (event.key	) {
	case OF_KEY_TAB:
		// select the next or previous (+SHIFT) control point
		if (ofGetKeyPressed(OF_KEY_SHIFT)) {
			if (mSelected == 0)
				mSelected = (int)mPoints.size() - 1;
			else
				--mSelected;
			selectControlPoint(mSelected);
		}
		else {
			++mSelected;
			if (mSelected >= mPoints.size()) mSelected = 0;
			selectControlPoint(mSelected);
		}
		break;
	case OF_KEY_UP: {
		if (mSelected >= mPoints.size()) return;
		float step = ofGetKeyPressed(OF_KEY_SHIFT) ? 10.0f : 0.5f;
		mPoints[mSelected].y -= step / mWindowSize.y;
		mIsDirty = true; }
		break;
	case OF_KEY_DOWN: {
		if (mSelected >= mPoints.size()) return;
		float step = ofGetKeyPressed(OF_KEY_SHIFT) ? 10.0f : 0.5f;
		mPoints[mSelected].y += step / mWindowSize.y;
		mIsDirty = true; }
		break;
	case OF_KEY_LEFT: {
		if (mSelected >= mPoints.size()) return;
		float step = ofGetKeyPressed(OF_KEY_SHIFT) ? 10.0f : 0.5f;
		mPoints[mSelected].x -= step / mWindowSize.x;
		mIsDirty = true; }
		break;
	case OF_KEY_RIGHT: {
		if (mSelected >= mPoints.size()) return;
		float step = ofGetKeyPressed(OF_KEY_SHIFT) ? 10.0f : 0.5f;
		mPoints[mSelected].x += step / mWindowSize.x;
		mIsDirty = true; }
		break;
	case 45: //-
		if (mSelected >= mPoints.size()) return;
		mBrightness = std::max(0.0f, mBrightness - 0.01f);
		break;
	case 43: //+
		if (mSelected >= mPoints.size()) return;
		mBrightness = std::min(1.0f, mBrightness + 0.01f);
		break;
	case 114: //r
		if (mSelected >= mPoints.size()) return;
		reset();
		mIsDirty = true;
		break;
	default:
		return;
	}

    event.key = -1;
}


void Warp::keyUp(ofKeyEventArgs &event)
{
}

void Warp::resize()
{
	mWindowSize = glm::vec2(ofGetWindowSize());
	mIsDirty = true;
}

void Warp::queueControlPoint(const glm::vec2 &pt, bool selected, bool attached)
{
	float scale = (0.9f + 0.2f * sin(6.0f * float(ofGetElapsedTimef() - mSelectedTime))) * 15;

	if (selected && attached) { 
		queueControlPoint(pt, ofColor(0, 32, 0), 15);
	}
	else if (selected) { 
		queueControlPoint(pt, ofColor(230, 230, 230), scale);
	}
	else if (attached) { 
		queueControlPoint(pt, ofColor(0, 102, 0)), 15;
	}
	else { 
		queueControlPoint(pt, ofColor(102, 102, 102), 15);
	}
}

void Warp::queueControlPoint(const glm::vec2 &pt, const ofColor &clr, float scale)
{
    if (mControlPoints.size() < MAX_NUM_CONTROL_POINTS){
    	mControlPoints.emplace_back(Data(pt, ofVec4f(clr.r, clr.g, clr.b, 1), scale));
        
    }
}

void Warp::drawControlPoints()
{
	if (!sIsEditMode) return;

	for (size_t i = 0; i < mPoints.size(); i++){
        ofSetColor(mControlPoints[i].color.x, mControlPoints[i].color.y, mControlPoints[i].color.z);
		ofDrawCircle(glm::vec2(mControlPoints[i].position.x, mControlPoints[i].position.y), mControlPoints[i].scale);
    }
	ofSetColor(255);
}
