#include "BulletScene.h"
#include "LaserBulletManager.h"
#include "GlowMaterialListener.h"
#include "ARLib/Sound/SoundListener.h"
#include "ARLib/Sound/SoundManager.h"

#define VISIBILITY_FLAG_LEFT  (1 << 0)
#define VISIBILITY_FLAG_RIGHT (1 << 1)

BulletScene::BulletScene(ARLib::Rift *rift, ARLib::TrackingManager *tracker,
    Ogre::Root *root, Ogre::RenderWindow *window, Ogre::RenderWindow *smallWindow, Ogre::SceneManager *sceneMgr,
    OgreBulletDynamics::DynamicsWorld *dyWorld, 
    OIS::Mouse *mouse, OIS::Keyboard *keyboard,
	ARLib::VideoPlayer *leftVideoPlayer, ARLib::VideoPlayer *rightVideoPlayer)
    : mRenderTarget(nullptr)
    , mSmallRenderTarget(nullptr)
    , mToggle(true)
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
	mRiftNode->getBodyNode()->setPosition(3.0f, 1.5f, 3.0f);
	if (tracker)
		tracker->addRigidBodyEventListener(mRiftNode);
    
    //create viewports
    if(window && rift){
        mRenderTarget = new ARLib::RiftRenderTarget(rift, root, window);
        setRenderTarget(mRenderTarget);
    }

    if(smallWindow){
        mSmallRenderTarget = new ARLib::DebugRenderTarget(smallWindow);
        setRenderTarget(mSmallRenderTarget);
    }

    //add Glow compositor
    mGlow[0] = Ogre::CompositorManager::getSingleton().addCompositor(mRiftNode->getLeftCamera()->getViewport(), "GlowBig");
    mGlow[0]->setEnabled(mToggle);
    mGlow[1] = Ogre::CompositorManager::getSingleton().addCompositor(mRiftNode->getRightCamera()->getViewport(), "GlowBig");
    mGlow[1]->setEnabled(mToggle);
	Ogre::CompositorManager::getSingleton().addCompositor(mRiftNode->getLeftCamera()->getViewport(), "GlowSmall");
	Ogre::CompositorManager::getSingleton().addCompositor(mRiftNode->getRightCamera()->getViewport(), "GlowSmall");
    GlowMaterialListener *gml = new GlowMaterialListener();
    Ogre::MaterialManager::getSingleton().addListener(gml);


	//mRiftVideoScreens = new ARLib::RiftVideoScreens(mSceneMgr, mRiftNode, leftVideoPlayer, rightVideoPlayer, tracker);

    //roomLight
	Ogre::Light* roomLight = mSceneMgr->createLight();
	roomLight->setType(Ogre::Light::LT_POINT);
	roomLight->setCastShadows( true );
	roomLight->setShadowFarDistance( 30.f );
	roomLight->setAttenuation( 65.0f, 1.0f, 0.07f, 0.017f );
	roomLight->setSpecularColour( .25f, .25f, .25f );
	roomLight->setDiffuseColour( 1.0f, 1.0f, 1.0f );
	roomLight->setPosition( 5.f, 5.f, 5.f );
	mSceneMgr->getRootSceneNode()->attachObject( roomLight );

    //ground-plane
    OgreBulletCollisions::CollisionShape *shape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3(0.15f,0.9f,0.0f), -5.0f);
    mShapes.push_back(shape);
    OgreBulletDynamics::RigidBody *planeBody = new OgreBulletDynamics::RigidBody("GroundPlane", mDynamicsWorld);
    planeBody->setStaticShape(shape, 0.1f, 0.8f);
    mRigidBodies.push_back(planeBody);


	mRemote = new StarWarsRemote(mSceneMgr->getRootSceneNode(), mSceneMgr, mDynamicsWorld, mRiftNode->getHeadNode(),5.0f);
    mRemotePuppet = new StarWarsRemotePuppet(mRemote, mRiftNode->getBodyNode(), mSceneMgr->getRootSceneNode(), mSceneMgr, mDynamicsWorld, 10.0f);
    mRemotePuppet->init(mRiftNode->getHeadNode()->_getDerivedOrientation() * Ogre::Vector3(0,0,-1));

    mSwordParentNode = new RigidListenerNode(mRiftNode->getBodyNode(), mSceneMgr, 2);
    mSword = new StarWarsLightSaber(mSwordParentNode->getSceneNode(), mSceneMgr, mDynamicsWorld);
    if(tracker){
        tracker->addRigidBodyEventListener(mSwordParentNode);
    }

    //rift light
	Ogre::Light* light = mSceneMgr->createLight();
	light->setType(Ogre::Light::LT_POINT);
	light->setCastShadows( false );
	light->setAttenuation( 65.0f, 1.0f, 0.07f, 0.017f );
	light->setSpecularColour( .25f, .25f, .25f );
	light->setDiffuseColour( 0.35f, 0.27f, 0.23f );
	mRiftNode->getBodyNode()->attachObject(light); 


    //Add Screenspace Ambient Occlusion
    //mDebugLeftSSAO = new PFXSSAO(smallWindow, mRiftNode->getLeftCamera());
    //mDebugRightSSAO = new PFXSSAO(smallWindow, mRiftNode->getRightCamera());
    //mLeftSSAO = new PFXSSAO(window, mRiftNode->getLeftCamera());
    //mRightSSAO = new PFXSSAO(window, mRiftNode->getRightCamera());
	
	//mVideoOffset[0] = Ogre::Vector2(-0.060f, 0.016f);
	//mVideoOffset[1] = Ogre::Vector2(-0.004f, 0.016f);
	//mVideoScale = Ogre::Vector2(0.98f, 0.90f);
	//mRiftVideoScreens->setOffsets(mVideoOffset[0], mVideoOffset[1]);
	//mRiftVideoScreens->setScalings(mVideoScale, mVideoScale);
}

BulletScene::~BulletScene()
{
    if(mRenderTarget) delete mRenderTarget;
    if(mSmallRenderTarget) delete mSmallRenderTarget;

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

void BulletScene::setRenderTarget(ARLib::RenderTarget *renderTarget)
{
	mRiftNode->removeAllRenderTargets();
	mRiftNode->addRenderTarget(renderTarget);
	mRiftNode->getLeftCamera()->getViewport()->setVisibilityMask(VISIBILITY_FLAG_LEFT);
	mRiftNode->getRightCamera()->getViewport()->setVisibilityMask(VISIBILITY_FLAG_RIGHT);
}

void BulletScene::toggleGlow()
{
    mToggle = !mToggle;
    mGlow[0]->setEnabled(mToggle);
    mGlow[1]->setEnabled(mToggle);
}

void BulletScene::update(float dt)
{
	ARLib::Rift *rift = mRiftNode->getRift();
    mRemotePuppet->update(dt);
	mRemote->update(dt);
    mSword->update(dt);
    LaserBulletManager::getSingleton().update(dt);

	//mRiftVideoScreens->update();
}

//////////////////////////////////////////////////////////////
// Handle Input:
//////////////////////////////////////////////////////////////

bool BulletScene::keyPressed( const OIS::KeyEvent& e )
{
    if(e.key == OIS::KC_C){
		mRiftNode->calibrate();
		mSwordParentNode->calibrate();
    }if(e.key == OIS::KC_V){
        mSword->draw();
    }if(e.key == OIS::KC_N){
        toggleGlow();
    }if(e.key == OIS::KC_D){
        mDynamicsWorld->setShowDebugShapes(!mDynamicsWorld->getShowDebugShapes());
    }
	
	/*const float offsetStep = 0.004f;
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
	if (e.key == OIS::KC_B) { mVideoOffset[0].x += 0.5f * offsetStep; mVideoOffset[1].x -= 0.5f * offsetStep; setOffsets = true; }
	if (e.key == OIS::KC_V) { mVideoOffset[0].x -= 0.5f * offsetStep; mVideoOffset[1].x += 0.5f * offsetStep; setOffsets = true; }

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
	}*/
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