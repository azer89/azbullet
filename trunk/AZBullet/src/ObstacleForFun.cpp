
#include "Stdafx.h"
#include "ObstacleForFun.h"

ObstacleForFun::ObstacleForFun(void)
{
}

ObstacleForFun::~ObstacleForFun(void)
{
}

void ObstacleForFun::createObject(OgreBulletListener *bulletListener, size_t &mNumEntitiesInstanced)
{

	for(int a = 0; a < 10; a++)	// vertical
	{
		for(int b = 0; b < 5; b++)	// horizontal
		{
			int idx = b - 3;
			
			this->addRigidBody(
				bulletListener,											// bullet listener
				"crate.mesh",											// mesh name
				Ogre::Vector3(-442 + (idx * 6.1), 62 + (a * 6.2), 30),	// position
				Ogre::Vector3(3, 3, 3),									// size
				Ogre::Vector3::ZERO,									// node translation
				Ogre::Vector3(2),										// scale
				Ogre::Quaternion::IDENTITY,								// rotation
				mNumEntitiesInstanced);									// number of instance
		}
	}

	for(int a = 0; a < 5; a++)
	{
		this->addRigidBody(
			bulletListener,	
			"crate.mesh",											
			Ogre::Vector3(-105, 112 + (a * 4.2), 280),	
			Ogre::Vector3(2, 2, 2),									
			Ogre::Vector3::ZERO,									
			Ogre::Vector3(1.3),										
			Ogre::Quaternion::IDENTITY,								
			mNumEntitiesInstanced);		

		this->addRigidBody(
			bulletListener,	
			"crate.mesh",											
			Ogre::Vector3(-145, 112 + (a * 4.2), 245),	
			Ogre::Vector3(2, 2, 2),									
			Ogre::Vector3::ZERO,									
			Ogre::Vector3(1.3),										
			Ogre::Quaternion::IDENTITY,								
			mNumEntitiesInstanced);									
	}

#ifndef _DEBUG

	for(int a = 0; a < 10; a++)	// vertical
	{
		for(int b = 0; b < 5; b++)	// horizontal
		{
			int idx = b - 3;

			this->addRigidBody(
				bulletListener,	
				"crate.mesh",											
				Ogre::Vector3(-442 + (idx * 4.1), 62 + (a * 4.2), 60),	
				Ogre::Vector3(2, 2, 2),									
				Ogre::Vector3::ZERO,									
				Ogre::Vector3(1.3),										
				Ogre::Quaternion::IDENTITY,								
				mNumEntitiesInstanced);									
		}
	}

#endif
}



void ObstacleForFun::updatePerFrame(Real elapsedTime)
{
}

//-------------------------------------------------------------------------------------
void ObstacleForFun::addRigidBody(
							   OgreBulletListener *bulletListener,
							   const Ogre::String meshFile,
							   const Ogre::Vector3 &pos, 
							   const Ogre::Vector3 &size,
							   const Ogre::Vector3 &translation,
							   const Ogre::Vector3 &scale,
							   const Ogre::Quaternion &q, 
							   size_t &mNumEntitiesInstanced)
{
	Ogre::Real bodyRestitution = 0.5; 
	Ogre::Real bodyFriction = 0.5;
	Ogre::Real bodyMass = 2.5;

	Entity *mEntity = bulletListener->mBulletSceneMgr->createEntity( "SwitchLever" + StringConverter::toString(mNumEntitiesInstanced), meshFile);
	mEntity->setQueryFlags (GEOMETRY_QUERY_MASK);

	BoxCollisionShape* sceneCubeShape = new BoxCollisionShape(size);

	RigidBody* mRigidBody = new RigidBody(
		"SwitchRigidBody" + StringConverter::toString(mNumEntitiesInstanced), 
		bulletListener->mBulletWorld);	

	Ogre::SceneNode* mMainNode = bulletListener->mBulletSceneMgr->getRootSceneNode()->createChildSceneNode();
	Ogre::SceneNode* switchLvrNode = mMainNode->createChildSceneNode();
	switchLvrNode->attachObject(mEntity);		
	switchLvrNode->translate(translation);
	switchLvrNode->setScale(scale);
	mRigidBody->setShape(mMainNode,  sceneCubeShape, bodyRestitution, bodyFriction, bodyMass, pos, q);
	mRigidBody->setDeactivationTime(0.01f);
	//mRigidBody->setDamping(0.9, 0.9);	

	bulletListener->mEntities.push_back(mEntity);
	bulletListener->mShapes.push_back(sceneCubeShape);
	bulletListener->mBodies.push_back(mRigidBody);

	mNumEntitiesInstanced++;
}