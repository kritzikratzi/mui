/*
 *  Root.cpp
 *  ofxMightyUI
 *
 *  Created by hansi on 28.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Root.h"
#include "ofEventUtils.h"
#include "ofEvents.h"
#include "Container.h"
#include "ScrollPane.h"
#include "ofxMightyUI.h"
#include <GLFW/glfw3.h>

using namespace mui;

// TODO: the handleXX functions might return null, even if touchMovedOutside and touchUpOutside 
//       delegated to containers. this shouldn't be the case. 


mui::Root * mui::Root::INSTANCE = NULL;

//--------------------------------------------------------------
mui::Root::Root() : Container( 0, 0, -1, -1 ){
	INSTANCE = this;
	ignoreEvents = true;
	keyboardResponder = nullptr;
	popupMenu = nullptr;
	init();
};

//--------------------------------------------------------------
void mui::Root::init(){
	#if TARGET_OS_IPHONE
	NativeIOS::init();
//	#elif TARGET_OS_MAC
//	NativeOSX::init();
	#endif
	
	name = "Root"; 
	width = ofGetWidth()/mui::MuiConfig::scaleFactor;
	height = ofGetHeight()/mui::MuiConfig::scaleFactor;
	setProperty<bool>("mui_is_root", true);

	ofAddListener( ofEvents().setup, this, &mui::Root::of_setup, OF_EVENT_ORDER_AFTER_APP );
	ofAddListener( ofEvents().update, this, &mui::Root::of_update, OF_EVENT_ORDER_AFTER_APP );
	ofAddListener( ofEvents().draw, this, &mui::Root::of_draw, OF_EVENT_ORDER_AFTER_APP );
	//ofAddListener( ofEvents().exit, this, &mui::Root::of_exit );
	ofAddListener( ofEvents().windowResized, this, &mui::Root::of_windowResized, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().keyPressed, this, &mui::Root::of_keyPressed, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().keyReleased, this, &mui::Root::of_keyReleased, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().mouseMoved, this, &mui::Root::of_mouseMoved, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().mouseDragged, this, &mui::Root::of_mouseDragged, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().mousePressed, this, &mui::Root::of_mousePressed, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().mouseReleased, this, &mui::Root::of_mouseReleased, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().mouseScrolled, this, &mui::Root::of_mouseScrolled, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().touchDown, this, &mui::Root::of_touchDown, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().touchUp, this, &mui::Root::of_touchUp, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().touchMoved, this, &mui::Root::of_touchMoved, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().touchDoubleTap, this, &mui::Root::of_touchDoubleTap, OF_EVENT_ORDER_BEFORE_APP );
	ofAddListener( ofEvents().touchCancelled, this, &mui::Root::of_touchCancelled, OF_EVENT_ORDER_BEFORE_APP );
	//ofAddListener( ofEvents().messageEvent, this, &mui::Root::of_messageEvent );
	ofAddListener( ofEvents().fileDragEvent, this, &mui::Root::of_fileDragEvent, OF_EVENT_ORDER_BEFORE_APP );
	
	
	// this seems unclear ... let's better put this in place!
	for( int i = 0; i < OF_MAX_TOUCHES; i++ ){
		touchResponder[i] = NULL;
	}
}

// resend hover/drag commands, so that we immediately see the effects of changed
// cursor etc. without having to handle all code everywhere.
void mui::Root::retriggerMouse(){
	// now simulate hover/touch/drag...
	bool pressed = ofGetMousePressed(OF_MOUSE_BUTTON_1) || ofGetMousePressed(OF_MOUSE_BUTTON_2) || ofGetMousePressed(OF_MOUSE_BUTTON_3) || ofGetMousePressed(OF_MOUSE_BUTTON_4) || ofGetMousePressed(OF_MOUSE_BUTTON_5) || ofGetMousePressed(OF_MOUSE_BUTTON_6);
	if (pressed) {
		// dragging...
		ofTouchEventArgs args;
		args.x = ofGetMouseX();
		args.y = ofGetMouseY();
		handleTouchMoved(args);
	}
	else {
		// moving
		ofMouseEventArgs args;
		args.x = ofGetMouseX();
		args.y = ofGetMouseY();
		handleMouseMoved(args.x, args.y);
	}
};

void mui::Root::handleUpdate(){
	int _width = ofGetWidth()/mui::MuiConfig::scaleFactor;
	int _height = ofGetHeight()/mui::MuiConfig::scaleFactor;

	if( width != _width || height != _height ){
		width = _width;
		height = _height;
		handleLayout();
		cout << "updating width width = " << width << ", height = " << height << endl;
	}
	else if( numLayoutFrames > 0 ){
		numLayoutFrames--; 
		handleLayout();
		cout << "updating width width = " << width << ", height = " << height << endl;
	}
	
	tweener.step( ofGetSystemTimeMillis() );
	
	// figure out where we are hovering
	
	std::set<Container*> newHoverResponder;
	Container * c = findChildAt(ofGetMouseX()/mui::MuiConfig::scaleFactor, ofGetMouseY()/mui::MuiConfig::scaleFactor,true,true);
	Container * top = c;
	if(c){
		ofMouseEventArgs args; //TODO: fix up coords
		while(c!=nullptr){
			if(!c->ignoreEvents){
				newHoverResponder.insert(c);
				if(hoverResponder.find(c) == hoverResponder.end() ){
					c->mouseEnter(args);
					c->onMouseEnter.notify(args);
				}
			}
			
			c = c->parent;
		}
	}
	
	if(manageCursor){
		mui::Container * src = touchResponder[0] ? touchResponder[0] : top;
		auto cursor = src? src->cursor : MUI_ROOT->cursor;
		if( cursor != lastCursor){
			muiSetCursor(cursor);
			lastCursor = cursor;
		}
	}

	for( Container * c : hoverResponder ){
		ofMouseEventArgs args; //TODO: fix up coords
		if(newHoverResponder.find(c) == newHoverResponder.end()){
			c->mouseExit(args);
			c->onMouseExit.notify(args);
		}
	}
	
	hoverResponder = std::move(newHoverResponder); 
	
	Container::handleUpdate();
    handleRemovals();
	
}

//--------------------------------------------------------------
void mui::Root::handleDraw(){
	ofSetupScreenOrtho(); 
	ofPushStyle();
	ofScale( mui::MuiConfig::scaleFactor, mui::MuiConfig::scaleFactor, mui::MuiConfig::scaleFactor );
	ofFill(); 
	ofSetLineWidth( 1 ); 
	ofSetColor( 255, 255, 255 ); 
	ofEnableAlphaBlending();
	
	Container::handleDraw();
	
    handleRemovals();
	
	if( mui::MuiConfig::debugDraw ){
		mui::Container * active = this->findChildAt( ofGetMouseX()/mui::MuiConfig::scaleFactor - this->x, ofGetMouseY()/mui::MuiConfig::scaleFactor-this->y, true );
		if( active != NULL ){
			ofPoint p = active->getGlobalPosition();
			ofPushMatrix();
			ofFill();
			string name;
			mui::Container * c = active;
			string size;
			while( c != NULL  ){
				bool empty = c->name == ""; 
				auto n = empty ? (typeid(*c).name()) : c->name;
				name = (empty?"::":"") + n + (name==""?"":" > ") + name;
				c = c->parent;
			}

			name = name + " [" + typeid(*active).name() + "]"; 
			
			ofRectangle b = active->getGlobalBounds();
			stringstream info; 
			info << "Pos:" << b.x << ", " << b.y << "  " << b.width << " x " << b.height << " / ";
			info << "Rel:" << active->x << "," << active->y;
			size = info.str();
			
			ofxFontStashStyle style = mui::Helpers::getStyle(10);
			ofNoFill();
			ofSetColor( 255,255,0 );
			ofDrawRectangle( p.x, p.y, active->width, active->height );

			if (p.y > 30) p.y -= 30;
			else p.y = std::min(muiGetHeight() - 30, p.y + active->height);

			ofSetColor(255);
			ofFill();
			style.color = ofColor(0);
			style.blur = 5;
			for(int i = 0; i < 10; i++){
				mui::Helpers::getFontStash().draw(name, style, p.x, p.y+10);
				mui::Helpers::getFontStash().draw(size, style, p.x, p.y+20);
			}
			style.color = ofColor(255);
			style.blur = 0;
			mui::Helpers::getFontStash().draw(name, style, p.x, p.y+10);
			mui::Helpers::getFontStash().draw(size, style, p.x, p.y+20);
			ofPopMatrix();
			ofSetColor(255);
		}
	}
	
	ofPopStyle();
}



//--------------------------------------------------------------
mui::Container * mui::Root::handleTouchDown( ofTouchEventArgs &touch ){
	#if TARGET_OS_IPHONE
	NativeIOS::hide();
//	#elif TARGET_OS_MAC
//	NativeOSX::hide();
	#endif
	
	ofTouchEventArgs copy = touch; 
	fixTouchPosition( touch, copy, NULL ); 
	
	uint64_t now = ofGetSystemTimeMicros();
	double dt = (now-lastTouchInteraction[touch.id])/1000000.0;
	if(dt<mui::MuiConfig::doubleTapTime){
		tapCounter[touch.id]++;
		copy.type = ofTouchEventArgs::doubleTap;
	}
	else{
		tapCounter[touch.id] = 1;
	}
	
	lastTouchInteraction[touch.id] = now;
	
	//return ( touchResponder[touch.id] = Container::handleTouchDown( copy ) );
	Container * lastPopup = popupMenu;
	touchResponder[touch.id] = nullptr;
	touchResponder[touch.id] = Container::handleTouchDown( copy );
	if( touchResponder[touch.id] != keyboardResponder ) keyboardResponder = NULL;
	if( popupMenu == lastPopup ) removePopupIfNecessary(touchResponder[touch.id]);
	
	return touchResponder[touch.id]; 
}


//--------------------------------------------------------------
mui::Container * mui::Root::handleTouchMoved( ofTouchEventArgs &touch ){
	ofTouchEventArgs copy = touch;
	fixTouchPosition( touch, copy, NULL );
	Container * touched = Container::handleTouchMoved( copy );
	
	if( touched != touchResponder[touch.id] && touchResponder[touch.id] != NULL ){
		copy = touch;
		fixTouchPosition( touch, copy, NULL );
		copy = Helpers::translateTouch( copy, this, touchResponder[touch.id] );
		touchResponder[touch.id]->touchMovedOutside( copy );
		touchResponder[touch.id]->onTouchMovedOutside.notify(copy);
		
		return touchResponder[touch.id];
	}
	
	return touched;
}



//--------------------------------------------------------------
mui::Container * mui::Root::handleTouchHover( ofTouchEventArgs &touch ){
	ofTouchEventArgs copy = touch;
	fixTouchPosition( touch, copy, NULL );
	Container * touched = Container::handleTouchHover( copy );
	
	if( touched != touchResponder[touch.id] && touchResponder[touch.id] != NULL ){
		copy = touch;
		fixTouchPosition( touch, copy, NULL );
		copy = Helpers::translateTouch( copy, this, touchResponder[touch.id] );
		touchResponder[touch.id]->touchMovedOutside( copy );
		touchResponder[touch.id]->onTouchMovedOutside.notify(copy);
		return touchResponder[touch.id];
	}
	
	return touched;
}


//--------------------------------------------------------------
mui::Container * mui::Root::handleTouchUp( ofTouchEventArgs &touch ){
	ofTouchEventArgs copy = touch; 
	fixTouchPosition( touch, copy, NULL ); 
	Container * touched = Container::handleTouchUp( copy ); 
	
	if( touched != touchResponder[touch.id] && touchResponder[touch.id] != NULL ){
		fixTouchPosition( touch, copy, touchResponder[touch.id] );
		Container *c = touchResponder[touch.id];
		touchResponder[touch.id]->touchUpOutside( copy );
		touchResponder[touch.id]->onTouchUpOutside.notify( copy );
		c->singleTouchId = -1;
	}
	
    touchResponder[touch.id] = NULL; 
    
	return touched; 	
}


//--------------------------------------------------------------
mui::Container * mui::Root::handleTouchDoubleTap( ofTouchEventArgs &touch ){
	ofTouchEventArgs copy = touch; 
	fixTouchPosition( touch, copy, NULL ); 

	return Container::handleTouchDoubleTap( copy ); 
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleTouchCancelled( ofTouchEventArgs &touch ){
	if( touchResponder[touch.id] != NULL ){
		touchResponder[touch.id]->touchCanceled( touch );
		touchResponder[touch.id]->singleTouchId = -1;
		mui::Container * c = touchResponder[touch.id];
		touchResponder[touch.id] = NULL;
		return c;
	}
	else{
		return NULL;
	}
}



//--------------------------------------------------------------
void mui::Root::fixTouchPosition( glm::vec2 &touch, glm::vec2 &copy, Container * container ){
	copy.x = touch.x/mui::MuiConfig::scaleFactor;
	copy.y = touch.y/mui::MuiConfig::scaleFactor;
	
	if( container != NULL ){
		ofPoint pos = container->getGlobalPosition();
		copy.x -= pos.x;
		copy.y -= pos.y;
	}
}



//--------------------------------------------------------------
ofRectangle mui::Root::convertNativeToMui( const ofRectangle rect ){
	const float s = mui::MuiConfig::scaleFactor;
	return ofRectangle(rect.x/s, rect.y/s, rect.width/s, rect.height/s);
}
//--------------------------------------------------------------
ofPoint mui::Root::convertNativeToMui( const ofPoint pt ){
	const float s = mui::MuiConfig::scaleFactor;
	return ofPoint(pt.x/s, pt.y/s, pt.z/s);
}
//--------------------------------------------------------------
ofRectangle mui::Root::convertMuiToNative( const ofRectangle rect ){
	const float s = mui::MuiConfig::scaleFactor;
	return ofRectangle(rect.x*s, rect.y*s, rect.width*s, rect.height*s);
}

//--------------------------------------------------------------
ofPoint mui::Root::convertMuiToNative( const ofPoint pt ){
	const float s = mui::MuiConfig::scaleFactor;
	return ofPoint(pt.x*s, pt.y*s, pt.z*s);
}



//--------------------------------------------------------------
bool mui::Root::becomeTouchResponder( Container * c, ofTouchEventArgs &touch ){
	// the trivial case ...
	if( c != NULL && c == touchResponder[touch.id] )
		return true;
	
	// notify previous owner,
	// cancel if it doesn't allow transfering focus
	if( touchResponder[touch.id] != NULL ){
        if( touchResponder[touch.id]->focusTransferable == false )
            return false; 
        
		touchResponder[touch.id]->onBlur.notify(touchResponder[touch.id]);
		touchResponder[touch.id]->handleTouchCanceled( touch );
		touchResponder[touch.id]->singleTouchId = -1; 
	}
	
	// alright, install new owner
	touchResponder[touch.id] = c;
	if( touchResponder[touch.id] != NULL ){
		touchResponder[touch.id]->onFocus.notify(touchResponder[touch.id]);
		touchResponder[touch.id]->singleTouchId = touch.id;
	}

	
	return true; 
}

bool mui::Root::becomeKeyboardResponder( Container * c ){
	this->keyboardResponder = c;
	return true;
}


//--------------------------------------------------------------
void mui::Root::safeRemove( Container * c ){
	if(c) safeRemoveList.push_back( c );
	removeFromResponders(c);
}

//--------------------------------------------------------------
void mui::Root::safeDelete( Container * c ){
	if(c) safeDeleteList.push_back( c );
	removeFromResponders(c);
}

//--------------------------------------------------------------
void mui::Root::safeRemoveAndDelete( mui::Container *c ){
    if(c) safeRemoveAndDeleteList.push_back( c ); 
	removeFromResponders(c);
}

//--------------------------------------------------------------
void mui::Root::removeFromResponders( Container * c ){
	if(c == nullptr ) return;
	
	for( int i = 0; i < OF_MAX_TOUCHES; i++ ){
		if( touchResponder[i] == c ){
			touchResponder[i] = NULL;
		}
	}
	
	if( keyboardResponder == c ){
		keyboardResponder = NULL;
	}
	
	hoverResponder.erase(c);
	
	if(c == popupMenu ) popupMenu = nullptr;
	// recurse
	//for(const auto & child : c->children){
	//	removeFromResponders(child);
	//}
}

void mui::Root::reloadTextures(){
	mui::Helpers::clearCaches();
}



//--------------------------------------------------------------
void mui::Root::prepareAnimation( int milliseconds, int type, int direction ){
	param = tween::TweenerParam( milliseconds, (short)type, (short)direction );
}

//--------------------------------------------------------------
void mui::Root::runOnUiThread(std::function<void()> func){
	//TBD
}

void mui::Root::setDisplayScaling(float reqScale){
	float scale = reqScale == 0? muiGetDefaultDisplayScaling() : reqScale;
	mui::MuiConfig::detectRetina = reqScale == 0;
	mui::MuiConfig::scaleFactor = scale;
	mui::Helpers::getFontStash().pixelDensity = scale;
	ofEvents().notifyWindowResized(ofGetWidth(), ofGetHeight());
}


//--------------------------------------------------------------
bool mui::Root::getKeyPressed( int key ){
	return ofGetKeyPressed(key);
}

//--------------------------------------------------------------
int mui::Root::getTapCount( int touchId ){
	return touchId<0 || touchId>=OF_MAX_TOUCHES? 0 : tapCounter[touchId];
}


//--------------------------------------------------------------
void mui::Root::animate( float &variable, float targetValue ){
    param.addProperty( &variable, targetValue ); 
}


//--------------------------------------------------------------
void mui::Root::commitAnimation(std::function<void()> onComplete){
	param.onComplete = [onComplete](tween::TweenerParam & param){onComplete();};
    tweener.addTween( param );
}

void mui::Root::commitAnimation(){
    tweener.addTween( param );
}

void mui::Root::handleRemovals(){
	vector<Container*> cp;
	swap(cp,safeRemoveList);
	for(auto c : cp){
		c->remove();
	}

	cp.clear();
	swap(cp,safeDeleteList);
	for(auto c : cp){
		c->remove();
	}
	
	cp.clear();
	swap(cp,safeRemoveAndDeleteList);
	for(auto c : cp){
		c->remove();
		delete c;
	}
	
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleKeyPressed( ofKeyEventArgs &event ){
	// this is a bit awkward, there seems to be a bug in
	// glfw that re-sends modifier key when they are released.
	// for now, i'm not fixing this. i really shouldn't be, not here.
	
	if( getKeyPressed(OF_KEY_ESC) && popupMenu != nullptr){
		if (keyboardResponder == nullptr || !keyboardResponder->onKeyPressed.notify(event) || !keyboardResponder->keyPressed(event)) {
			safeRemove(popupMenu);
			popupMenu = nullptr; 
		}
		retriggerMouse(); 
		return this;
	}
	
	if( mui::MuiConfig::enableDebuggingShortcuts && getKeyPressed(MUI_KEY_ACTION) && event.keycode == GLFW_KEY_D ){
		mui::MuiConfig::debugDraw ^= true;
		retriggerMouse();
		return this;
	}
	
	if( mui::MuiConfig::enableDebuggingShortcuts && getKeyPressed(OF_KEY_ALT) && event.keycode == OF_KEY_RETURN){
		// dump the view hierachy!
		mui::Container * active = this->findChildAt( ofGetMouseX()/mui::MuiConfig::scaleFactor - this->x, ofGetMouseY()/mui::MuiConfig::scaleFactor-this->y, true );
		
		cout << "------------------------------------";
		cout << "DUMPING VIEW HIERACHY" << endl;
		while( active != NULL ){
			ofRectangle b = active->getBounds();
			stringstream info; 
			info << "Pos:" << b.x << ", " << b.y << "  " << b.width << " x " << b.height;
			string size = info.str();
			
			cout << "> " << active->name << size << endl;
			active = active->parent;
		}
		cout << "------------------------------------";
		
		retriggerMouse();
		return this;
	}

	if (mui::MuiConfig::debugDraw && getKeyPressed(OF_KEY_ALT) && event.keycode == 'F') {
		mui::Container * active = this->findChildAt(muiGetMouseX() - this->x, muiGetMouseY() - this->y, true);
        ofTouchEventArgs temp;
		if (active) active->requestFocus(temp);
		return this; 
	}

	if( mui::MuiConfig::debugDraw && getKeyPressed(OF_KEY_ALT) && event.keycode == 'I' ){
		cout << "------------------------------------" << endl;
		mui::Container * active = this->findChildAt( muiGetMouseX() - this->x, muiGetMouseY()-this->y, true );
		cout << "Set a debug point in " << __FILE__ << ":" << __LINE__ << " to inspect this element" << endl;
		cout << "------------------------------------" << endl;
		
		return this;
	}
	
	if( mui::MuiConfig::debugDraw && getKeyPressed(OF_KEY_ALT) && event.keycode == 'L' ){
		handleLayout();
		retriggerMouse();
		return this;
	}

	mui::Container * temp = keyboardResponder; 

	if (temp == nullptr) {
		auto pos = muiGetMousePos();
		temp = findChildAt(pos.x, pos.y, true, true);
		// sorry that this is a property ... :/
		while(temp != nullptr && temp->getPropertyOr("hoverKeyEvent",false)==false){
			temp = temp->parent;
		}
	}

	if( temp != nullptr ){
		if( !temp->isVisibleOnScreen()){
			temp = nullptr;
			retriggerMouse(); 
			return nullptr; 
		}
		else{
			while(temp != nullptr && !temp->onKeyPressed.notify(event) && !temp->keyPressed(event)){
				temp = temp->parent;
			}

			retriggerMouse(); 
			return temp; 
		}
	}

	// still here? a bit sad... 
	retriggerMouse();
	return nullptr;
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleKeyReleased( ofKeyEventArgs &event ){
	mui::Container * temp = keyboardResponder;

	if (temp == nullptr) {
		auto pos = muiGetMousePos();
		temp = findChildAt(pos.x, pos.y, true, true);
		// sorry that this is a property ... :/
		while(temp != nullptr && temp->getPropertyOr("hoverKeyEvent",false)==false){
			temp = temp->parent;
		}
	}

	if( temp != nullptr ){
		if( !temp->isVisibleOnScreen()){
			temp = nullptr;
			retriggerMouse();
			return nullptr;
		}
		else{
			while(temp != nullptr && !temp->onKeyReleased.notify(event) && !temp->keyReleased(event)){
				temp = temp->parent;
			}

			retriggerMouse();
			return temp;
		}
	}

	// still here? a bit sad... 
	retriggerMouse();
	return nullptr;
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleMouseMoved( float x, float y ){
	ofTouchEventArgs args;
	args.x = x;
	args.y = y;
	args.id = 0;
	return handleTouchHover(args);
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleMouseDragged( float x, float y, int button ){
	ofTouchEventArgs args;
	args.x = x;
	args.y = y;
	args.id = 0;
	return handleTouchMoved(args);
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleMousePressed( float x, float y, int button ){
	ofTouchEventArgs args;
	args.x = x;
	args.y = y;
	args.id = 0;
	uint64_t now = ofGetSystemTimeMicros();
	if(now>lastMouseDown && (now-lastMouseDown)/1000<230){
		args.type = ofTouchEventArgs::doubleTap;
	}
	lastMouseDown = now;
	return handleTouchDown(args);
}

//--------------------------------------------------------------
mui::Container * mui::Root::handleMouseReleased( float x, float y, int button ){
	ofTouchEventArgs args;
	args.x = x;
	args.y = y;
	args.id = 0;
	return handleTouchUp(args);
}

void mui::Root::showPopupMenu( mui::Container * c, mui::Container * source, ofVec2f pos, mui::HorizontalAlign horizontalAlign, mui::VerticalAlign verticalAlign ){
	mui::Root::showPopupMenu(c, source, pos.x, pos.y, horizontalAlign, verticalAlign );
}

void mui::Root::showPopupMenu( mui::Container * c, mui::Container * source, float x, float y, mui::HorizontalAlign horizontalAlign, mui::VerticalAlign verticalAlign ){
	if(popupMenu != nullptr){
		popupMenu->visible = false;
		popupMenu->remove(); 
		popupMenu = nullptr;
	}
	
	if(c == nullptr) return;
	
	c->visible = true; 
	add(c);
	c->handleLayout();
	popupMenu = c;
	
	if( source == nullptr ){
		popupMenu->x = x;
		popupMenu->y = y;
	}
	else{
		ofPoint p = source->getGlobalPosition();
		popupMenu->x = p.x + x;
		popupMenu->y = p.y + y;
	}
	
	switch(horizontalAlign){
		case mui::Left: break;
		case mui::Right: popupMenu->x -= popupMenu->width; break;
		case mui::Center: popupMenu->x -= popupMenu->width/2; break;
	}
	switch(verticalAlign){
		case mui::Top: break;
		case mui::Bottom: popupMenu->y -= popupMenu->height; break;
		case mui::Middle: popupMenu->y -= popupMenu->height/2; break;
	}
	
	popupMenu->x = ofClamp(popupMenu->x, 1, width - popupMenu->width );
	popupMenu->y = ofClamp(popupMenu->y, 1, height - popupMenu->height );
}

void mui::Root::removePopup(mui::Container * popup) {
	if (this->popupMenu == popup) {
		popup->visible = false; 
		safeRemove(popup); 
		popup = nullptr;
	}
}


void mui::Root::of_setup( ofEventArgs &args ){
	//handleSetup();
	handleLayout(); 
}
void mui::Root::of_update( ofEventArgs &args ){
	if(mui::MuiConfig::detectRetina){
		auto ptr = ofGetWindowPtr();
		auto glfw = dynamic_cast<ofAppGLFWWindow*>(ptr);
		if(glfw && mui::MuiConfig::scaleFactor != muiGetDefaultDisplayScaling() ){
			mui::MuiConfig::scaleFactor = muiGetDefaultDisplayScaling();
			mui::Helpers::getFontStash().pixelDensity = mui::MuiConfig::scaleFactor;
			cout << "[ofxMightyUI] updated pixel scaling factor to " << mui::MuiConfig::scaleFactor << endl;
		}
	}
	handleUpdate();
}
void mui::Root::of_draw( ofEventArgs &args ){
	handleDraw();
}
void mui::Root::of_exit( ofEventArgs &args ){
	//handleExit(args);
}
void mui::Root::of_windowResized( ofResizeEventArgs &args ){
	//handleWindowResized(args);
	if (args.width <= 0 || args.height <= 0) return; 
	width = args.width/mui::MuiConfig::scaleFactor;
	height = args.height/mui::MuiConfig::scaleFactor;
	numLayoutFrames = 1;
	handleLayout();
}
bool mui::Root::of_keyPressed( ofKeyEventArgs &args ){
	return handleKeyPressed(args) != NULL;
}
bool mui::Root::of_keyReleased( ofKeyEventArgs &args ){
	return handleKeyReleased(args) != NULL;
}
bool mui::Root::of_mouseMoved( ofMouseEventArgs &args ){
	return handleMouseMoved(args.x, args.y) != NULL;
}
bool mui::Root::of_mouseDragged( ofMouseEventArgs &args ){
	return handleMouseDragged(args.x, args.y, args.button) != NULL;
}
bool mui::Root::of_mousePressed( ofMouseEventArgs &args ){
	return handleMousePressed(args.x, args.y, args.button) != NULL;
}
bool mui::Root::of_mouseReleased( ofMouseEventArgs &args ){
	return handleMouseReleased(args.x, args.y, args.button) != NULL;
}
bool mui::Root::of_mouseScrolled( ofMouseEventArgs &args ){
    glm::vec2 pos;
	fixTouchPosition(args, pos, NULL);
	mui::Container * container = findChildAt(pos.x, pos.y, true, true );
	while( container != NULL ){
		if( container->onMouseScroll.notify(args) ) return true;
		if( container->mouseScroll(args) ) return true;
		container = container->parent;
	}

	return false;
}
bool mui::Root::of_touchDown( ofTouchEventArgs &args ){
	return handleTouchDown(args) != NULL;
}
bool mui::Root::of_touchUp( ofTouchEventArgs &args ){
	return handleTouchUp(args) != NULL;
}
bool mui::Root::of_touchMoved( ofTouchEventArgs &args ){
	return handleTouchMoved(args) != NULL;
}
bool mui::Root::of_touchDoubleTap( ofTouchEventArgs &args ){
	return handleTouchDoubleTap(args) != NULL;
}
bool mui::Root::of_touchCancelled( ofTouchEventArgs &args ){
	return handleTouchCancelled(args) != NULL;
}
void mui::Root::of_messageEvent( ofMessage &args ){
	//handleMessageEvent(args);
}
bool mui::Root::of_fileDragEvent( ofDragInfo &args ){
	ofDragInfo copy = args;
	copy.position.x = args.position.x/mui::MuiConfig::scaleFactor;
	copy.position.y = args.position.y/mui::MuiConfig::scaleFactor;
	
	return handleFileDragged(copy);
}

void mui::Root::removePopupIfNecessary( mui::Container * target ){
	if(popupMenu != nullptr ){
		if(target != nullptr ){
			// is the popup somehow a parent of what was clicked?
			while(target != nullptr){
				if( target == popupMenu ){
					return;
				}
				target = target->parent;
			}
		}
		
		popupMenu->visible = false;
		safeRemove(popupMenu);
		popupMenu = nullptr;
	}
}
