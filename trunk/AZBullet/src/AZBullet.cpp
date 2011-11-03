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

#include "OgreBulletDynamicsWorld.h"

static const Ogre::Vector3 CameraStart = Ogre::Vector3(0, 25, 0);

AZBullet::AZBullet(void):OgreBulletListener()
{
	this->mRotateSpeed = 0.1f;

	vehicle = NULL;
	tManager = NULL;
	obs = NULL;
	rayTerrain = NULL;
}

//-------------------------------------------------------------------------------------
AZBullet::~AZBullet(void)
{	
	if(vehicle != NULL) delete vehicle;
	if(tManager != NULL) delete tManager;
	if(obs != NULL) delete obs;
	if (rayTerrain != NULL) delete rayTerrain;
}

//-------------------------------------------------------------------------------------

void AZBullet::createScene(void)
{		
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

	mCamera->setNearClipDistance(0.1);
	mCamera->setFarClipDistance(50000);
	if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE)) { mCamera->setFarClipDistance(0);}   // enable infinite far clip distance if we can

	mBulletCameraMove = 1;

	Ogre::Viewport *vp = this->mCamera->getViewport();
	vp->setBackgroundColour(ColourValue(0, 0, 0.2f));
	
	// Start Bullet
	initWorld();

	Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
    lightdir.normalise();

	// add lights
	setBulletBasicLight();

	Ogre::Light* light = mSceneMgr->createLight("tstLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.4, 0.4, 0.4));
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

	// create terrain group
	//rayTerrain = new RayTerrain();
	//rayTerrain->createTerrain(this->mSceneMgr, light);
	//rayTerrain->integrateBullet(mSceneMgr, mBulletWorld, mBodies, mShapes);
	
	// Create a terrain
	tManager = new TerrainManager();
	tManager->createTerrain(mSceneMgr, mBulletWorld, mBodies, mShapes);

	// create a vehicle
	vehicle = new Vehicle();
	vehicle->createVehicle(this->mSceneMgr, 
		this->mBulletWorld, 
		this->mNumEntitiesInstanced,
		this->mCamera);

	mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));

	// extended camera
	exCamera = new ThirdPersonCamera("Third Person Camera", mSceneMgr, mCamera);

	// frame listener to manage camera
	mCameraListener = new CameraListener(mWindow, mCamera);
	static_cast<CameraListener*> (mCameraListener)->setCharacter(vehicle);
	static_cast<CameraListener*> (mCameraListener)->setExtendedCamera(exCamera);
	mCameraListener->instantUpdate();	

	// create the obstacle
	obs = new ObstacleForFun();
	obs->createObstacle(this);
	
	Ogre::Plane skyPlane;
	skyPlane.d = 100;
	skyPlane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;

	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8, 1500);
	//mSceneMgr->setSkyPlane(true, skyPlane, "Examples/CloudySky", 50, 10 , true, 0.7, 10, 10);

	//mBulletWorld->getDebugDrawer()->setDrawWireframe(true);
	//mBulletWorld->setShowDebugShapes(true);

	createSimpleWater();
}

//-------------------------------------------------------------------------------------
void AZBullet::createSimpleWater()
{	
	// this is to add water pond effect
	Ogre::Plane plane01(Ogre::Vector3::UNIT_Y, 0);

	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane01, 1000, 1000, 20, 20, true, 1, 20, 20, Ogre::Vector3::UNIT_Z);

	Ogre::Entity* entWater01 = mSceneMgr->createEntity("WaterPlane01", "ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode("Water", Ogre::Vector3(500, 30, 500))->attachObject(entWater01);

	entWater01->setMaterialName("Examples/WaterStream3");
	entWater01->setCastShadows(false);	
	
	// this is to add water pond effect
	Ogre::Plane plane02(Ogre::Vector3::UNIT_Y, 0);

	/*	
	Ogre::MeshManager::getSingleton().createPlane("ground", 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane02, 
		1000, 
		1000, 
		1, 
		1, 
		true, 
		1, 
		5, 
		5, 
		Ogre::Vector3::UNIT_Z);

	Ogre::Entity* entWater02 = mSceneMgr->createEntity("WaterPlane02", "ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode("Water02", Ogre::Vector3(500, 5, 500))->attachObject(entWater02);

	entWater02->setMaterialName("Examples/WaterStream2");
	entWater02->setCastShadows(false);	
	*/
}


bool AZBullet::frameRenderingQueued(const Ogre::FrameEvent& arg)
{
	if(!BaseApplication::frameRenderingQueued(arg))
	{
		return false;
	}

	Ogre::Real elapsedTime = arg.timeSinceLastFrame;
	
	this->repositionCamera();
	mBulletWorld->stepSimulation(elapsedTime);	
	this->vehicle->updatePerFrame(arg.timeSinceLastFrame);

	/*
	if (rayTerrain->getTerrainGroup()->isDerivedDataUpdateInProgress())
	{

		mTrayMgr->moveWidgetToTray(mInfoLabel, OgreBites::TL_TOP, 0);
		mInfoLabel->show();
		if (rayTerrain->getTerrainsImported())
		{
			mInfoLabel->setCaption("Building terrain, please wait...");
		}
		else
		{
			mInfoLabel->setCaption("Updating textures, patience...");
		}

	}
	else
	{

		mTrayMgr->removeWidgetFromTray(mInfoLabel);
		mInfoLabel->hide();
		if (rayTerrain->getTerrainsImported())
		{
			rayTerrain->getTerrainGroup()->saveAllTerrains(true);
			//mTerrainsImported = false;
			rayTerrain->setTerrainsImported(false);
		}		
	}
	*/
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
	mCameraListener->frameStarted(evt);

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

	if(arg.key == OIS::KC_UP     || 
		arg.key == OIS::KC_RIGHT || 
		arg.key == OIS::KC_LEFT  || 
		arg.key == OIS::KC_DOWN
		)
	{
	}
	/*else if(arg.key == OIS::KC_W)
	{
	}
	else if(arg.key == OIS::KC_A)
	{
	}
	else if(arg.key == OIS::KC_S)
	{
	}
	else if(arg.key == OIS::KC_D)
	{
	}*/
	else
	{
		mCameraMan->injectKeyDown(arg);
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

	if(arg.key == OIS::KC_UP     || 
		arg.key == OIS::KC_RIGHT || 
		arg.key == OIS::KC_LEFT  || 
		arg.key == OIS::KC_DOWN )
	{
	}
	/*else if(arg.key == OIS::KC_W)
	{
	}
	else if(arg.key == OIS::KC_A)
	{
	}
	else if(arg.key == OIS::KC_S)
	{
	}
	else if(arg.key == OIS::KC_D)
	{
	}*/
	else
	{
		mCameraMan->injectKeyUp(arg);
	}

	vehicle->keyReleased(arg);
	return true;
}

//-------------------------------------------------------------------------------------
void AZBullet::repositionCamera()
{
	//Ogre::Vector3 camPos = mCamera->getRealPosition();
	Ogre::Vector3 camPos = exCamera->getCameraPosition();
	
	//std::cout << camPos << "\n";
	
	Ogre::Ray cameraRay(Ogre::Vector3(camPos.x, 5000.0f, camPos.z), Ogre::Vector3::NEGATIVE_UNIT_Y);
	mRayScnQuery->setRay(cameraRay);

	// Perform the scene query
	Ogre::RaySceneQueryResult &result = mRayScnQuery->execute();
	Ogre::RaySceneQueryResult::iterator itr = result.begin();
	// Get the results, set the camera height
	if (itr != result.end() && itr->worldFragment)
	{
		Ogre::Real terrainHeight = itr->worldFragment->singleIntersection.y;

		//if ((terrainHeight + 0.5f) > camPos.y)
		if( Ogre::Math::Abs(terrainHeight - camPos.y) > 1.0f )
		{
			//std::cout << itr->worldFragment->singleIntersection.y << "-";
			//std::cout << camPos.y << "\n";
			//std::cout << "whoops\n";
			//exCamera->instantUpdate(Ogre::Vector3(camPos.x, terrainHeight + 1.0f, camPos.z));
			//mCamera->setPosition( camPos.x, terrainHeight + 0.5f, camPos.z );
		}
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

	// create text inf on top of window
	//mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TInfo", "", 350);
	//mInfoLabel->hide();

	//mRoot->addFrameListener(mCameraListener);
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
