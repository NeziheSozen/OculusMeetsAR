#ifndef REMOTE_H
#define REMOTE_H

#include "OGRE/Ogre.h"
#include "Cannon.h"
#include "OgreBullet/Collisions/Shapes/OgreBulletCollisionsSphereShape.h"
#include "OgreBullet/Dynamics/OgreBulletDynamicsRigidBody.h"
#include <random>

class StarWarsRemote{
public:
    StarWarsRemote(Ogre::SceneNode *parentNode, Ogre::SceneManager *sceneMgr, OgreBulletDynamics::DynamicsWorld *dynamicsWorld, Ogre::SceneNode* player, float radius);
    ~StarWarsRemote();
		
    void update(float dt);
    void changePos(const Ogre::Vector3& newPos);
    void changeMaterial(float interp);

private:
    Ogre::SceneNode *mSceneNode;
    Ogre::SceneNode *mSpinNode;
    OgreBulletCollisions::SphereCollisionShape *mRemoteSphere;
    OgreBulletDynamics::RigidBody *mRemoteBody;
	
    std::default_random_engine mGenerator;
    std::uniform_real_distribution<float> mDistribution;

    Ogre::SceneNode *mThrusterNode;
    Ogre::Pass *mThrusterHighPass;
    Ogre::Pass *mThrusterLowPass;
    Ogre::Pass *mThrusterIntermediatePass;

    StarWarsLaserCannon *mCannons;

    Ogre::SceneNode *mPlayer;
    float mRadius;
    float mAccumTime;
    float mAccumRot;
	float mTimeSinceShotsFired;
	float mTimeBetweenShots;
};

#endif
