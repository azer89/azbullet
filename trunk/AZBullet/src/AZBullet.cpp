/*
-----------------------------------------------------------------------------
Filename:    AZBullet.cpp
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
#include "Stdafx.h"
#include "AZBullet.h"

/*
#include "OgreBulletDynamics.h"
#include "OgreBulletListener.h"
#include "ObstacleForFun.h"
#include "TerrainManager.h"
#include "Vehicle.h"


#if !(OGRE_VERSION <  ((1 << 16) | (3 << 8) | 0))
using namespace OIS;
#endif 

using namespace Ogre;
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;
*/
static const Ogre::Vector3 CameraStart = Ogre::Vector3(0, 25, 0);

AZBullet::AZBullet(void):OgreBulletListener()
{
	this->mRotateSpeed = 0.1f;
}

//-------------------------------------------------------------------------------------
AZBullet::~AZBullet(void)
{
	if(vehicle) delete vehicle;
	if(tManager) delete tManager;
	if(obs) delete obs;
}

//-------------------------------------------------------------------------------------

void AZBullet::createScene(void)
{	
	/*
    Ogre::Entity* mEntity = mSceneMgr->createEntity("Head", "wheel.mesh");

    Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(mEntity);
	mNode->scale(10.0f, 10.0f, 10.0f);

    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

    // Create a light
    Ogre::Light* l = mSceneMgr->createLight("MainLight");
    l->setPosition(20,80,50);
	*/

	hViewPort = mCamera->getViewport();

	this->bulletInit();
}

// -------------------------------------------------------------------------
void AZBullet::bulletInit()
{	
	mBulletCamera = mCamera;		// OgreBulletListener's camera
	mBulletWindow = mWindow;		// OgreBulletListener's window
	mBulletRoot = mRoot;			// OgreBulletListener's root
	mBulletSceneMgr = mSceneMgr;	// OgreBulletListener's scene manager

	mBulletCameraMove = 1;

	Ogre::Viewport *vp = this->mCamera->getViewport();
	vp->setBackgroundColour(ColourValue(0,0,0));

	// add lights
	setBulletBasicLight();

	// Start Bullet
	initWorld();

	// Create a terrain
	tManager = new TerrainManager();
	tManager->createTerrain(mSceneMgr, mBulletWorld, mBodies, mShapes);

	// create a vehicle
	vehicle = new Vehicle();
	vehicle->createVehicle(this->mSceneMgr, this->mBulletWorld, this->mNumEntitiesInstanced, tManager->terrain_Shift);

	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
	mCamera->setPosition(CameraStart + tManager->terrain_Shift);
	mCamera->lookAt(vehicle->CarPosition + tManager->terrain_Shift);

	// create the obstacle
	obs = new ObstacleForFun();
	obs->createObstacle(this, tManager->terrain_Shift);

	// delete all pointers
	//delete vehicle;
	//delete tManager;
	//delete obs;
}

//-------------------------------------------------------------------------------------
bool AZBullet::frameRenderingQueued(const Ogre::FrameEvent& arg)
{
	if(!BaseApplication::frameRenderingQueued(arg))
	{
		return false;
	}

	Ogre::Real elapsedTime = arg.timeSinceLastFrame;
	
	mBulletWorld->stepSimulation(elapsedTime);
	this->repositionCamera();
	this->vehicle->updatePerFrame(arg.timeSinceLastFrame);

	return true;

}

// -------------------------------------------------------------------------
bool AZBullet::frameStarted(const FrameEvent& evt)
{
	Ogre::Real elapsedTime = evt.timeSinceLastFrame;
	/*
	if(!OgreBulletListener::bulletFrameStarted(elapsedTime))
	{
		return false;
	}
	*/
	return true;
}

// -------------------------------------------------------------------------
bool AZBullet::frameEnded(const FrameEvent& evt)
{
	Ogre::Real elapsedTime = evt.timeSinceLastFrame;
	/*	
	if(!OgreBulletListener::bulletFrameEnded(elapsedTime))
	{
		return false;
	}
	*/
	return true; 
}

//-------------------------------------------------------------------------------------
// when mouse dragged
bool AZBullet::mouseMoved(const OIS::MouseEvent& arg)
{
	if(!BaseApplication::mouseMoved(arg)) { return false; }

	Ogre::Real screenWidth = hViewPort->getWidth();
	Ogre::Real screenHeight = hViewPort->getHeight();

	Ogre::Real offsetX = (float)arg.state.X.abs / (float)arg.state.width;
	Ogre::Real offsetY = (float)arg.state.Y.abs / (float)arg.state.height;

	//using namespace Hikari;
	//bool val = menu->hikariMgr->injectMouseMove(arg.state.X.abs, arg.state.Y.abs) ||  menu->hikariMgr->injectMouseWheel(arg.state.Z.rel);

	//if the left mouse button is held down
	if(bLMouseDown)
	{
		
	}
	else if(bRMouseDown)	//if the right mouse button is held down, be rotate the camera with the mouse
	{
		mCamera->yaw(Ogre::Degree(-arg.state.X.rel * mRotateSpeed));
		mCamera->pitch(Ogre::Degree(-arg.state.Y.rel * mRotateSpeed));
	}

	return true;
}

//-------------------------------------------------------------------------------------
// when a mouse button is pressed
bool AZBullet::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{	
	if(id == OIS::MB_Left)
	{
		bLMouseDown = true;		
	}
	else if(id == OIS::MB_Right)
	{
		bRMouseDown = true;

	}
	
	return true;
	//return  menu->hikariMgr->injectMouseDown(id); 
}

//-------------------------------------------------------------------------------------
// when a mouse button is released
bool AZBullet::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	if(id  == OIS::MB_Left)
	{
		bLMouseDown = false;
	}
	else if(id == OIS::MB_Right)
	{
		bRMouseDown = false;
	}

	//return  menu->hikariMgr->injectMouseUp(id);

	return true;
}

//------------------------------------------------------------------------------------- 
// when keyboard pressed
bool AZBullet::keyPressed(const OIS::KeyEvent& arg)
{
	if(!BaseApplication::keyPressed(arg))
	{
		return false;
	}

	vehicle->keyPressed(arg);

	return true;
}

//------------------------------------------------------------------------------------- 
// when keyboard pressed
bool AZBullet::keyReleased(const OIS::KeyEvent& arg)
{
	if(!BaseApplication::keyReleased(arg))
	{
		return false;
	}

	vehicle->keyReleased(arg);

	return true;
}

//-------------------------------------------------------------------------------------
void AZBullet::repositionCamera()
{
	// Setup the scene query
	Ogre::Vector3 camPos = mCamera->getPosition();
	Ogre::Ray cameraRay(Ogre::Vector3(camPos.x, 5000.0f, camPos.z), Ogre::Vector3::NEGATIVE_UNIT_Y);
	mRayScnQuery->setRay(cameraRay);

	// Perform the scene query
	Ogre::RaySceneQueryResult &result = mRayScnQuery->execute();
	Ogre::RaySceneQueryResult::iterator itr = result.begin();
	// Get the results, set the camera height
	if (itr != result.end() && itr->worldFragment)
	{
		Ogre::Real terrainHeight = itr->worldFragment->singleIntersection.y;
		if ((terrainHeight + 10.0f) > camPos.y)
			mCamera->setPosition( camPos.x, terrainHeight + 10.0f, camPos.z );
	}
}

//-------------------------------------------------------------------------------------
void AZBullet::createFrameListener(void)
{
	BaseApplication::createFrameListener();

	this->bLMouseDown = false;
	this->bRMouseDown = false;
	this->mRotateSpeed = 0.1f;
	this->mName = "AZBullet";	

	mRayScnQuery = mSceneMgr->createRayQuery(Ogre::Ray());

}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    // INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
	int main()
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        AZBullet app;

        try 
		{
            app.go();
        } 
		catch( Ogre::Exception& e ) 
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
