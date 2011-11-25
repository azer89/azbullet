/*
-----------------------------------------------------------------------------
Filename:    AZBullet.h
-----------------------------------------------------------------------------


This source file is generated by the
   ___                   _              __    __ _                  _ 
  /___\__ _ _ __ ___    /_\  _ __  _ __/ / /\ \ (_)______ _ _ __ __| |
 //  // _` | '__/ _ \  //_\\| '_ \| '_ \ \/  \/ / |_  / _` | '__/ _` |
/ \_// (_| | | |  __/ /  _  \ |_) | |_) \  /\  /| |/ / (_| | | | (_| |
\___/ \__, |_|  \___| \_/ \_/ .__/| .__/ \/  \/ |_/___\__,_|_|  \__,_|
      |___/                 |_|   |_|                                 
      Ogre 1.7.x Application Wizard for VC9 (July 2011)
      http://code.google.com/p/ogreappwizards/
-----------------------------------------------------------------------------
*/

#ifndef __AZBullet_h_
#define __AZBullet_h_

//#include "ClassDefine.h"
//#include "OgreBulletDynamics.h"
//#include "OgreBulletListener.h"

#include "BaseApplication.h"

#include "ObstacleForFun.h"
#include "TerrainManager.h"
#include "Vehicle.h"
#include "CameraListener.h"
#include "ThirdPersonCamera.h"
#include "RayTerrain.h"

#include "Hydrax/Hydrax.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "../res/resource.h"
#endif

class AZBullet : public BaseApplication,  public OgreBulletListener
{
public:
	AZBullet::AZBullet(void);
    virtual ~AZBullet(void);	

	void bulletInit();
	virtual void createFrameListener(void);

	// void bulletKeyPressed(BULLET_KEY_CODE key);
	// void bulletKeyReleased(BULLET_KEY_CODE key);
	// bool bulletFrameStarted(Ogre::Real elapsedTime);

	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& arg);

public:
	Ogre::Viewport* hViewPort;

protected:
     virtual void createScene(void);
	 void repositionCamera(void);
	 void createSimpleWater();
	 void createSimpleSky();
	 void createHydraxSimulation();
	 void changeSkyBox();

	 // OIS::MouseListener
	 virtual bool mouseMoved(const OIS::MouseEvent& arg);
	 virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
	 virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

	 // OIS::KeyListener
	 virtual bool keyPressed(const OIS::KeyEvent& arg);
	 virtual bool keyReleased( const OIS::KeyEvent &arg);

protected:
	Ogre::SceneNode *mCurrentObject;	// pointer to our currently selected object
	Ogre::RaySceneQuery* mRayScnQuery;	// pointer to our ray scene query	

	bool bLMouseDown, bRMouseDown;		// true if mouse buttons are held down
	float mRotateSpeed;					// the rotation speed for the camera
	int mCurrentSkyBox;

	TerrainManager* tManager;
	Vehicle* vehicle;
	ObstacleForFun* obs;
	RayTerrain* rayTerrain;
	Hydrax::Hydrax *mHydrax;

	OgreBites::Label* mInfoLabel;	// message info

	CameraListener* mCameraListener;
	ThirdPersonCamera* exCamera;
};

#endif // #ifndef __AZBullet_h_
