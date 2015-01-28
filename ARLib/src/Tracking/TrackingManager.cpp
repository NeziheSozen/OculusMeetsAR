#include "ARLib/Tracking/TrackingManager.h"
#include "ARLib/Tracking/RigidBodyEventListener.h"

namespace ARLib{
	
TrackingManager::TrackingManager(TRACKING_METHOD tracking, unsigned int frameBufferSize, Rift *oculusHMD)
	: mTracking(tracking)
	, mRiftHandle(oculusHMD)
	, mEvaluator(nullptr)
	, mNatNetHandler(nullptr)
	, mInitialized(false){
	mEvaluator = new FrameEvaluator(frameBufferSize);
}

TrackingManager::~TrackingManager(){
	if(mNatNetHandler != nullptr)
		delete mNatNetHandler;
	delete mEvaluator;
}

TRACKING_ERROR_CODE TrackingManager::initialize(){
	TRACKING_ERROR_CODE errorCode = NONE;
	if(!(mTracking & (ARLIB_NATNET | ARLIB_RIFT))){
		errorCode = errorCode | ARLIB_TRACKING_NO_DEVICE_ERROR;
	}
	if(mTracking & ARLIB_NATNET){
		mNatNetHandler = new NatNetHandler(mNatNetConnectionType);
		mNatNetHandler->registerFrameEvaluator(mEvaluator);
		mNatNetHandler->connect(mNatNetServerIP.c_str(), mNatNetClientIP.c_str());
		if(mNatNetHandler->connected() & NATNET_DISCONNECTED ||
			mNatNetHandler->connected() & NATNET_PENDING){
				errorCode = errorCode | ARLIB_TRACKING_NATNET_ERROR;
		}
	}
	if(mTracking & ARLIB_RIFT){
		if(!mRiftHandle){
			errorCode = errorCode | ARLIB_TRACKING_RIFT_ERROR;
		}
	}
	if(errorCode == NONE){
		mInitialized = true;
		return ARLIB_TRACKING_OK;	
	}else{
		mInitialized = false;
		return errorCode;
	}
}

void TrackingManager::uninitialize(){
	if(mNatNetHandler != nullptr){
		delete mNatNetHandler;
		mNatNetHandler = nullptr;
	}
	mInitialized = false;
}

TRACKING_ERROR_CODE TrackingManager::reinitialize(){
    uninitialize();
    return initialize();
}
		
void TrackingManager::update(float retroActiveQueryTime){
	if(mInitialized){
		mEvaluator->evaluate(retroActiveQueryTime);
	}
}

void TrackingManager::setNatNetConnectionType(ConnectionType cType){
	mNatNetConnectionType = cType;
}
		
void TrackingManager::setNatNetServerIP(const std::string& sIP){
	mNatNetServerIP = sIP;
}

void TrackingManager::setNatNetClientIP(const std::string& cIP){
	mNatNetClientIP = cIP;
}

void TrackingManager::setFrameEvaluationMethod(FRAME_EVALUATION_METHOD eval){
    mEvaluator->setEvaluationMethod(eval);
}

void TrackingManager::addRigidBodyEventListener(RigidBodyEventListener* listener){
	mEvaluator->addRigidBodyEventListener(listener);
}

};