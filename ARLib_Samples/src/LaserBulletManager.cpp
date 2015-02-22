#include "LaserBulletManager.h"
#include "OgreBullet/Collisions/Utils/OgreBulletCollisionsMeshToShapeConverter.h"
#include "OgreBullet/Dynamics/Constraints/OgreBulletDynamicsHingeConstraint.h"

extern ContactProcessedCallback gContactProcessedCallback;

LaserBullet::LaserBullet(Ogre::SceneManager* sceneMgr, OgreBulletDynamics::DynamicsWorld *dyWorld, const Ogre::Vector3& position, const Ogre::Vector3& impulse)
    : mShape(nullptr)
    , mBody(nullptr)
    , mLifeTime(0.0f)
	, mCollided(false){
	gContactProcessedCallback = (ContactProcessedCallback)HandleCollision;

    //rotate Y axis onto impulse
    Ogre::Vector3 cross = Ogre::Vector3::UNIT_Y.crossProduct(impulse.normalisedCopy());
    Ogre::Matrix3 rotation = Ogre::Matrix3(0, cross.z, -cross.y, -cross.z, 0, cross.x, cross.y, -cross.x, 0);
    Ogre::Vector3 axis;
    Ogre::Degree angle;
    rotation.ToAngleAxis(axis, angle);

	mSceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
	mSceneNode->setPosition(position);

    Ogre::Entity *bullet = sceneMgr->createEntity("Bullet.mesh");
    bullet->setMaterialName("RedGlow");
    mSceneNode->attachObject(bullet);

	OgreBulletCollisions::StaticMeshToShapeConverter *stmc = new OgreBulletCollisions::StaticMeshToShapeConverter(bullet);

    mShape = stmc->createConvex();
	mShape->getBulletShape()->setLocalScaling(btVector3(0.1f, 0.2f, 0.1f));
    mBody = new OgreBulletDynamics::RigidBody("LaserBullet" + std::to_string(LaserBulletManager::getSingleton().getBulletCounter()), dyWorld, 1, 2);
    mBody->setShape(mSceneNode, mShape, 0.6f, 0.6f, 1.0f, Ogre::Vector3::ZERO, Ogre::Quaternion(Ogre::Radian(angle), axis));
    mBody->enableActiveState();
    mBody->applyImpulse(impulse, Ogre::Vector3::ZERO);

	mBody->getBulletRigidBody()->setCollisionFlags(mBody->getBulletRigidBody()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	mBody->getBulletRigidBody()->setAngularFactor(0.0f);
	mBody->getBulletRigidBody()->setUserIndex(42);
	mBody->getBulletRigidBody()->setUserPointer(this);
	//OgreBulletDynamics::HingeConstraint *lockedYAxis;
	//lockedYAxis = new OgreBulletDynamics::HingeConstraint(mBody, Ogre::Vector3(0,1,0), Ogre::Vector3(0,1,0));
	//dyWorld->addConstraint(lockedYAxis);
    delete stmc;
}

LaserBullet::~LaserBullet(){
    if(mBody != nullptr)
        delete mBody;
    if(mShape != nullptr)
        delete mShape;
	if(mSceneNode != nullptr)
		delete mSceneNode;
}

bool LaserBullet::update(float dt){
	//printf("%f, %f, %f \n", mBody->getLinearVelocity().x, mBody->getLinearVelocity().y, mBody->getLinearVelocity().z);
    const float MaxLifeTime = 30.0f;
    mLifeTime += dt;
    if(mLifeTime >= MaxLifeTime){
        return false;
    }

	if(mCollided == true){
		mCollided = false;

		Ogre::Vector3 impulse = mBody->getLinearVelocity();
		Ogre::Vector3 cross = Ogre::Vector3::UNIT_Y.crossProduct(Ogre::Vector3(impulse.x , impulse.y, impulse.z));
		Ogre::Matrix3 rotation = Ogre::Matrix3(0, cross.z, -cross.y, -cross.z, 0, cross.x, cross.y, -cross.x, 0);
		Ogre::Vector3 axis;
		Ogre::Degree angle;
		rotation.ToAngleAxis(axis, angle);

		OgreBulletDynamics::RigidBody *body = new OgreBulletDynamics::RigidBody("LaserBullet" + std::to_string(LaserBulletManager::getSingleton().getBulletCounter()), mBody->getDynamicsWorld(), 1, 2);
		Ogre::Vector3 wp = mBody->getWorldPosition();
		delete mBody;
		body->setShape(mSceneNode, mShape, 0.6f, 0.6f, 1.0f, wp, Ogre::Quaternion(Ogre::Radian(angle), axis));
		mBody = body;

		body->enableActiveState();
		body->applyImpulse(Ogre::Vector3(impulse.x, impulse.y, impulse.z), Ogre::Vector3::ZERO);

		body->getBulletRigidBody()->setCollisionFlags(body->getBulletRigidBody()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		body->getBulletRigidBody()->setAngularFactor(0.0f);
		body->getBulletRigidBody()->setUserIndex(42);
		body->getBulletRigidBody()->setUserPointer(this);
		mCollided = false;
	}
    return true;
}

bool LaserBullet::HandleCollision(btManifoldPoint& cp, void* body0, void* body1){
	btRigidBody* rigidbody0 = reinterpret_cast<btRigidBody*>(body0);
	btRigidBody* rigidbody1 = reinterpret_cast<btRigidBody*>(body1);
	
	btRigidBody *selectedBody;
	if(rigidbody0->getUserPointer() != nullptr){ //rigidbody0 is Laserbullet
		printf("0\n");
		selectedBody = rigidbody0;
	}else if(rigidbody1->getUserPointer() != nullptr){ //rigidbody1 is Laserbullet
		printf("1\n");
		selectedBody = rigidbody1;
	}else{
		return false;
	}
	LaserBullet *userData = reinterpret_cast<LaserBullet*>(selectedBody->getUserPointer());
	userData->mCollided = true;
	btVector3 impulse = selectedBody->getLinearVelocity();
	printf("%f, %f, %f \n", impulse.getX(), impulse.getY(), impulse.getZ());
	return true; //return value ignored anyways
}

LaserBulletManager::~LaserBulletManager(){
    for(std::deque<LaserBullet*>::iterator it = mShots.begin(); it != mShots.end(); ++it){
        //delete *it;
    }
}

void LaserBulletManager::update(float dt){
    unsigned int pop_backs = 0;
    for(std::deque<LaserBullet*>::iterator it = mShots.begin(); it != mShots.end(); ++it){
        if(!(*it)->update(dt)){
            delete (*it);
            pop_backs++;
        }
    }
    mShots.erase(mShots.begin(), mShots.begin() + pop_backs);
}

void LaserBulletManager::addBullet(LaserBullet* bullet){
    mBulletCounter ++;
    mShots.push_back(bullet);
}