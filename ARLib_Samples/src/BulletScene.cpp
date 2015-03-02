#include "BulletScene.h"
#include "LaserBulletManager.h"
#include "GlowMaterialListener.h"
#include "ARLib/Sound/SoundListener.h"
#include "ARLib/Sound/SoundManager.h"
#include "NPRWatercolorRenderTarget.h"

BulletScene::BulletScene(ARLib::Rift *rift, ARLib::TrackingManager *tracker,
    Ogre::Root *root, Ogre::RenderWindow *window, Ogre::RenderWindow *smallWindow, Ogre::SceneManager *sceneMgr,
    OgreBulletDynamics::DynamicsWorld *dyWorld, 
    OIS::Mouse *mouse, OIS::Keyboard *keyboard,
	ARLib::VideoPlayer *leftVideoPlayer, ARLib::VideoPlayer *rightVideoPlayer)
    : mRenderTarget(nullptr)
    , mSmallRenderTarget(nullptr)
    , mToggle(true)
	, mVideoPlayerLeft(leftVideoPlayer), mVideoPlayerRight(rightVideoPlayer)
	, mRiftVideoScreens(nullptr)
	, additionalLatency(0.048)
	, enabledNPRRenderer(false)
	, mWatercolorRenderTarget(nullptr)
	, mSmallWatercolorRenderTarget(nullptr)
	, mSmallGlowRenderTarget(nullptr)
	, mGlowRenderTarget(nullptr)
{
    LaserBulletManager::getSingleton().setDynamicsWorld(dyWorld);

    mGlow[0] = nullptr;
    mGlow[1] = nullptr;
	mRoot = root;
	mMouse = mouse;
	mKeyboard = keyboard;
	mSceneMgr = sceneMgr;
    mDynamicsWorld = dyWorld;

	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.1f,0.1f,0.1f));
    mSceneMgr->setShadowTechnique(Ogre::SHADOWDETAILTYPE_TEXTURE);
	mSceneMgr->setShadowFarDistance(30);

	//rift node
	mRiftNode = new ARLib::RiftSceneNode(rift, mSceneMgr, 0.001f, 50.0f, 1); // TODO: set correct rigid body id!
	//mRiftNode->getBodyNode()->setPosition(0.f,0.f,0.f);
	if (tracker)
		tracker->addRigidBodyEventListener(mRiftNode);
    
    //create viewports
    if(window && rift){
        mRenderTarget = new ARLib::RiftRenderTarget(rift, root, window);
		mGlowRenderTarget = new GlowRenderTarget(mRenderTarget);
		mWatercolorRenderTarget = new NPRWatercolorRenderTarget(root, mGlowRenderTarget, 1461, 1182, 1461 / 10, 1182 / 8, 0.1f);
    }

    if(smallWindow){
        mSmallRenderTarget = new ARLib::DebugRenderTarget(smallWindow);
		mSmallGlowRenderTarget = new GlowRenderTarget(mSmallRenderTarget);
		mSmallWatercolorRenderTarget = new NPRWatercolorRenderTarget(root, mSmallGlowRenderTarget, 1461/2, 1182/2, 1461 / 10, 1182 / 8, 0.1f);
    }

	
	mRiftNode->removeAllRenderTargets();

	if (mRenderTarget && mRenderTarget != mSmallRenderTarget)
			mRiftNode->addRenderTarget(mGlowRenderTarget);
	if (mSmallRenderTarget)
			mRiftNode->addRenderTarget(mSmallGlowRenderTarget);

	GlowMaterialListener *gml = new GlowMaterialListener();
	Ogre::MaterialManager::getSingleton().addListener(gml);
	mRiftVideoScreens = new ARLib::RiftVideoScreens(mSceneMgr, mRiftNode, leftVideoPlayer, rightVideoPlayer, tracker);

	/*Ogre::SceneNode *cubeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Ogre::Entity *cubeEntity = mSceneMgr->createEntity("cube.mesh");
	cubeEntity->setMaterialName("CubeMaterialRed");
	cubeNode->attachObject(cubeEntity);
	cubeNode->setPosition(0,0,-10);*/

    //roomLight
	Ogre::Light* roomLight = mSceneMgr->createLight();
	roomLight->setType(Ogre::Light::LT_POINT);
	roomLight->setCastShadows( true );
	roomLight->setShadowFarDistance( 30.f );
	roomLight->setAttenuation( 65.0f, 1.0f, 0.07f, 0.017f );
	roomLight->setSpecularColour( .25f, .25f, .25f );
	roomLight->setDiffuseColour( 1.0f, 1.0f, 1.0f );
	roomLight->setPosition( 3.f, 0.5f, 1.f );
	mSceneMgr->getRootSceneNode()->attachObject( roomLight );

    //ground-plane
    OgreBulletCollisions::CollisionShape *shape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3(0.15f,0.9f,0.0f), -5.0f);
    mShapes.push_back(shape);
    OgreBulletDynamics::RigidBody *planeBody = new OgreBulletDynamics::RigidBody("GroundPlane", mDynamicsWorld);
    planeBody->setStaticShape(shape, 0.1f, 0.8f);
    mRigidBodies.push_back(planeBody);

	Ogre::Light* light = mSceneMgr->createLight();
	light->setType(Ogre::Light::LT_POINT);
	light->setCastShadows( false );
	light->setAttenuation( 65.0f, 1.0f, 0.07f, 0.017f );
	light->setSpecularColour( .4f, .4f, .4f );
	light->setDiffuseColour( 0.7f, 0.54f, 0.46f );
	mRiftNode->getBodyNode()->attachObject(light); 

	Ogre::SceneNode *swordOffsetNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	swordOffsetNode->setPosition(0,0,-0.2f);
    mSwordParentNode = new RigidListenerNode(swordOffsetNode, mSceneMgr, 2);
    mSword = new StarWarsLightSaber(mSwordParentNode->getSceneNode(), mSceneMgr, mDynamicsWorld);
    if(tracker){
        tracker->addRigidBodyEventListener(mSwordParentNode);
    }

	mRemote = new StarWarsRemote(mSceneMgr->getRootSceneNode(), mSceneMgr, mDynamicsWorld, mRiftNode->getHeadNode(),2.0f);
    mRemotePuppet = new StarWarsRemotePuppet(mRemote, mRiftNode->getBodyNode(), mSceneMgr->getRootSceneNode(), mSceneMgr, mDynamicsWorld, 2.0f);
    mRemotePuppet->init(mRiftNode->getHeadNode()->_getDerivedOrientation() * Ogre::Vector3(0,0,-1));
	
	mVideoOffset[0] = Ogre::Vector2(-0.060f, 0.016f);
	mVideoOffset[1] = Ogre::Vector2(-0.004f, 0.016f);
	mVideoScale = Ogre::Vector2(0.98f, 0.90f);
	mRiftVideoScreens->setOffsets(mVideoOffset[0], mVideoOffset[1]);
	mRiftVideoScreens->setScalings(mVideoScale, mVideoScale);

	setAdditionalLatency(additionalLatency);
}

BulletScene::~BulletScene()
{
    delete mRenderTarget;
    delete mSmallRenderTarget;
	delete mWatercolorRenderTarget;
	delete mSmallWatercolorRenderTarget;

	mRoot->destroySceneManager(mSceneMgr);

    std::deque<OgreBulletDynamics::RigidBody*>::iterator itBody = mRigidBodies.begin();
    while(itBody != mRigidBodies.end()){
        delete *itBody;
        ++itBody;
    }
    mRigidBodies.clear();
    std::deque<OgreBulletCollisions::CollisionShape*>::iterator itShapes = mShapes.begin();
    while(itShapes != mShapes.end()){
        delete *itShapes;
        ++itShapes;
    }
    mShapes.clear();
	delete mRiftNode;
}

void BulletScene::toggleGlow()
{
    mToggle = !mToggle;
    mGlow[0]->setEnabled(mToggle);
    mGlow[1]->setEnabled(mToggle);
}

void BulletScene::toggleNPRRenderer()
{
	mRiftNode->removeAllRenderTargets();

	if (enabledNPRRenderer)
	{
		if (mGlowRenderTarget && mGlowRenderTarget != mSmallGlowRenderTarget)
			mRiftNode->addRenderTarget(mGlowRenderTarget);
		if (mSmallGlowRenderTarget)
			mRiftNode->addRenderTarget(mSmallGlowRenderTarget);
		enabledNPRRenderer = false;
	}
	else
	{
		if (mGlowRenderTarget && mGlowRenderTarget != mSmallGlowRenderTarget)
			mRiftNode->addRenderTarget(mWatercolorRenderTarget);
		if (mSmallGlowRenderTarget)
			mRiftNode->addRenderTarget(mSmallWatercolorRenderTarget);
		enabledNPRRenderer = true;
	}
}

void BulletScene::update(float dt)
{
    mRemotePuppet->update(dt);
	mRemote->update(dt);
    mSword->update(dt);
    LaserBulletManager::getSingleton().update(dt);
	
	mRiftVideoScreens->update();
}

void BulletScene::setAdditionalLatency(double seconds)
{
	LARGE_INTEGER frequency, additionalLatency;
	QueryPerformanceFrequency(&frequency);
	additionalLatency.QuadPart = (LONGLONG)(seconds * frequency.QuadPart);
	mVideoPlayerLeft->setAdditionalLatency(additionalLatency);
	mVideoPlayerRight->setAdditionalLatency(additionalLatency);
	printf("additional latency: %03lfs\n", seconds);
}

//////////////////////////////////////////////////////////////
// Handle Input:
//////////////////////////////////////////////////////////////

bool BulletScene::keyPressed( const OIS::KeyEvent& e )
{
    if(e.key == OIS::KC_C){
		mRiftNode->calibrate();
		mSwordParentNode->calibrate();
		mRemotePuppet->init(mRiftNode->getHeadNode()->_getDerivedOrientation() * Ogre::Vector3(0,0,-1));
    }if(e.key == OIS::KC_V){
        mSword->draw();
    }if(e.key == OIS::KC_N){
        toggleNPRRenderer(); //toggleGlow();
    }if(e.key == OIS::KC_D){
        mDynamicsWorld->setShowDebugShapes(!mDynamicsWorld->getShowDebugShapes());
    }
	
	const float offsetStep = 0.004f;
	bool setOffsets = false;
	// left
	if (e.key == OIS::KC_D) { mVideoOffset[0].x -= offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_A) { mVideoOffset[0].x += offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_W) { mVideoOffset[0].y += offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_S) { mVideoOffset[0].y -= offsetStep; setOffsets = true; }
	// right
	if (e.key == OIS::KC_L) { mVideoOffset[1].x -= offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_J) { mVideoOffset[1].x += offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_I) { mVideoOffset[1].y += offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_K) { mVideoOffset[1].y -= offsetStep; setOffsets = true; }
	// IPD adjustment
	if (e.key == OIS::KC_M) { mVideoOffset[0].x += 0.5f * offsetStep; mVideoOffset[1].x -= 0.5f * offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_B) { mVideoOffset[0].x -= 0.5f * offsetStep; mVideoOffset[1].x += 0.5f * offsetStep; setOffsets = true; }

	if (setOffsets)
	{
		mRiftVideoScreens->setOffsets(mVideoOffset[0], mVideoOffset[1]);
		printf("offset L: %02f x %02f\tR: %02f x %02f\n", mVideoOffset[0].x, mVideoOffset[0].y, mVideoOffset[1].x, mVideoOffset[1].y);
	}

	// video scalings
	const float scaleStep = 0.01f;
	bool setScalings = false;
	// same for both for now...?
	if (e.key == OIS::KC_RIGHT) { mVideoScale.x -= scaleStep; setScalings = true; }
	if (e.key == OIS::KC_LEFT ) { mVideoScale.x += scaleStep; setScalings = true; }
	if (e.key == OIS::KC_UP   ) { mVideoScale.y -= scaleStep; setScalings = true; }
	if (e.key == OIS::KC_DOWN ) { mVideoScale.y += scaleStep; setScalings = true; }

	if (setScalings)
	{
		mRiftVideoScreens->setScalings(mVideoScale, mVideoScale);
		printf("scale: %02f x %02f\n", mVideoScale.x, mVideoScale.y);
	}

	// video latency
	const double latencyStep = 0.002;
	bool setLatency = false;
	if (e.key == OIS::KC_0) { additionalLatency -= latencyStep; setLatency = true; }
	if (e.key == OIS::KC_9) { additionalLatency += latencyStep; setLatency = true; }
	if (setLatency) setAdditionalLatency(additionalLatency);

	return true;
}
bool BulletScene::keyReleased( const OIS::KeyEvent& e )
{
	return true;
}

bool BulletScene::mouseMoved( const OIS::MouseEvent& e )
{
	if( mMouse->getMouseState().buttonDown( OIS::MB_Left ) )
	{
		mRiftNode->setYaw(Ogre::Degree(-0.3f*e.state.X.rel));
		mRiftNode->setPitch(Ogre::Degree(-0.3f*e.state.Y.rel));
	}
	return true;
}

bool BulletScene::mousePressed( const OIS::MouseEvent& e, OIS::MouseButtonID id )
{
	return true;
}

bool BulletScene::mouseReleased( const OIS::MouseEvent& e, OIS::MouseButtonID id )
{
	return true;
}