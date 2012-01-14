

#ifndef __Robot_h_
#define __Robot_h_

#include "Ogre.h"
#include "Character.h"
#include "Ship.h"

#include "Ogre.h"
#include "Character.h"
#include "LinearMath/btVector3.h"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "Shapes/OgreBulletCollisionsBoxShape.h"
#include "Shapes/OgreBulletCollisionsCompoundShape.h"
#include "Constraints/OgreBulletDynamicsRaycastVehicle.h"
#include "OgreBulletDynamicsRigidBody.h"

using namespace Ogre;
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;

//enum RobotState
//{
//	MOVE_FORWARD,
//	MOVE_BACKWARD,
//	NOT_MOVE
//};

class Robot : public Character
{
private:	
	OgreBulletDynamics::WheeledRigidBody        *mRobotChassis;
	OgreBulletDynamics::VehicleTuning	        *mTuning;
	OgreBulletDynamics::VehicleRayCaster	    *mVehicleRayCaster;
	//OgreBulletDynamics::RaycastVehicle	    *mVehicle;

	Ogre::Entity    *robotEntity;
	Ogre::Entity    *mWheels[4];
	Ogre::SceneNode *mWheelNodes[4];	

	int mWheelsEngine[4];
	int mWheelsEngineCount;
	int mWheelsSteerable[4];
	int mWheelsSteerableCount;

	float mEngineForce;
	float mSteering;

	int mWheelEngineStyle;
	int mWheelSteeringStyle;

	bool mSteeringLeft;
	bool mSteeringRight;

protected:
	//RobotState robotState;
	Ogre::AnimationState* ani;
	
public:
	Ship* ship;
	SceneNode *robotNode;
	Vector3 robotPosition;
	Quaternion robotRotation;
	SceneNode *vehicleNode;
	OgreBulletDynamics::RaycastVehicle	        *mVehicle;

public:
	Robot(void);
	virtual ~Robot(void);

	void createObject(SceneManager* mSceneMgr, 
		OgreBulletDynamics::DynamicsWorld *mBulletWorld,
		size_t &mNumEntitiesInstanced);

	void updatePerFrame(Real elapsedTime);
	void keyPressed(const OIS::KeyEvent& arg);
	void keyReleased(const OIS::KeyEvent& arg);
};

#endif