/*
-----------------------------------------------------------------------------
Filename:    AZBullet.cpp
Reza Adhitya Saputra
-----------------------------------------------------------------------------
*/

#include "Stdafx.h"
#include "AZBullet.h"
#include "RayFlashInterface.h"

#include "OgreBulletDynamicsWorld.h"
#include "Hydrax/Noise/Perlin/Perlin.h"
#include "Hydrax/Modules/ProjectedGrid/ProjectedGrid.h"

//-------------------------------------------------------------------------------------
AZBullet::AZBullet(void):OgreBulletListener()
{
	_def_SkyBoxNum = 3;

	mSkyBoxes[0] = "Sky/ClubTropicana"; mSkyBoxes[1] = "Sky/EarlyMorning"; mSkyBoxes[2] = "Sky/Clouds";
	mSunPosition[0] = Ogre::Vector3(0,10000,0); mSunPosition[1] = Ogre::Vector3(0,10000,90000); mSunPosition[2] = Ogre::Vector3(0,10000,0);	
	mSunColor[0] =  Ogre::Vector3(1, 0.9, 0.6); mSunColor[1] =  Ogre::Vector3(1,0.6,0.4); mSunColor[2] =  Ogre::Vector3(0.45,0.45,0.45);

	this->mRotateSpeed = 0.1f;			// camera rotation speed
	this->mCurrentSkyBox = 0;
	this->isHydraxEnabled = true;

	vehicle = 0;
	tManager = 0;
	obs = 0;
	//rayTerrain = 0;
	mHydrax = 0;
	menu = 0;
	rocket = 0;
	robot = 0;
	ship = 0;
	fancyTerrain = 0;
	switchLever = 0;
	flag = 0;
	soundManager = 0;
}

//-------------------------------------------------------------------------------------
AZBullet::~AZBullet(void)
{	
	this->bulletShutdown();
	mRoot->removeFrameListener(this);

	if (vehicle)		delete vehicle;
	if (tManager)		delete tManager;
	if (obs)			delete obs;
	//if (rayTerrain)		delete rayTerrain;
	if (mHydrax)		delete mHydrax;
	if (menu)			delete menu;
	if (rocket)			delete rocket;
	if (robot)			delete robot;
	if (ship)			delete ship;
	if (fancyTerrain)	delete fancyTerrain;
	if (switchLever)	delete switchLever;
	if (flag)			delete flag;
	if (soundManager)	delete soundManager;
}

//-------------------------------------------------------------------------------------
void AZBullet::createScene(void)
{		
	hViewPort = mCamera->getViewport();
	this->bulletInit();

	menu = new RayFlashInterface(this);
	menu->setupHikari();

	compSample = new CompositorSample();
	compSample->mCamera = this->mCamera;
	compSample->mViewport = this->hViewPort;
	
	compSample->setupCompositorContent();

	// create sounds
	soundManager = new SoundManager();
	soundManager->createSound();
	switchLever->soundManager = soundManager;

	//mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
	//mSceneMgr->setShadowColour(ColourValue(0, 0, 0));
	
/*#ifndef _DEBUG
	compSample->setCompositorEnabled("HDR", true);
#endif*/	
}

//------------------------------------------------------------------------------------
void AZBullet::changeSkyBox()
{
	// Change skybox
	mSceneMgr->setSkyBox(true, mSkyBoxes[mCurrentSkyBox], 99999*3, true);

	// Update Hydrax sun position and colour
	mHydrax->setSunPosition(mSunPosition[mCurrentSkyBox]);
	mHydrax->setSunColor(mSunColor[mCurrentSkyBox]);

	// Update light 0 light position and colour
	mSceneMgr->getLight("Light0")->setPosition(mSunPosition[mCurrentSkyBox]);
	mSceneMgr->getLight("Light0")->setSpecularColour(mSunColor[mCurrentSkyBox].x,
													 mSunColor[mCurrentSkyBox].y,
													 mSunColor[mCurrentSkyBox].z);
}

// -------------------------------------------------------------------------
void AZBullet::bulletInit()
{
	mBulletSceneMgr = mSceneMgr;	// OgreBulletListener's scene manager

	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setSkyBox(true, mSkyBoxes[mCurrentSkyBox], 99999*3, true);
	mCamera->setNearClipDistance(5);
	mCamera->setFarClipDistance(99999*6);
	mCamera->pitch(Degree(0));
	mCamera->yaw(Degree(0));

	initWorld();	// Start Bullet

	// Light
	Ogre::Light *mLight = mSceneMgr->createLight("Light0");
	mLight->setPosition(mSunPosition[mCurrentSkyBox]);
	mLight->setDiffuseColour(1, 1, 1);
	mLight->setSpecularColour(mSunColor[mCurrentSkyBox].x,
							  mSunColor[mCurrentSkyBox].y,
							  mSunColor[mCurrentSkyBox].z);

	// create a vehicle
	vehicle = new Vehicle();
	vehicle->createObject(this->mSceneMgr, this->mBulletWorld, this->mNumEntitiesInstanced, this->mCamera);

	exCamera = new ThirdPersonCamera("Third Person Camera", mSceneMgr, mCamera);
	mCameraListener = new CameraListener(mWindow, mCamera);
	static_cast<CameraListener*> (mCameraListener)->setExtendedCamera(exCamera);
	changeCameraPosition(0);

	// create the obstacle
	//obs = new ObstacleForFun();
	//obs->createObstacle(this);
	
	RigidBody* terrainRigidBody = this->addStaticTrimesh("GroundsceneMesh",
		"terrain_ground.mesh",
		Ogre::Vector3(0, 0, 0), 
		Quaternion::IDENTITY,
		0.1f, 
		0.8f, false);

	rocket = new Rocket();
	rocket->createObject(mSceneMgr);

	robot = new Robot();
	robot->createObject(mSceneMgr, this->mBulletWorld, this->mNumEntitiesInstanced);

	fancyTerrain = new FancyTerrain();
	fancyTerrain->createObject(this, this->mNumEntitiesInstanced);
	fancyTerrain->terrainRigidBody = terrainRigidBody;
	
	createSimpleWater();
	createHydraxSimulation();
	
#ifdef _DEBUG
	this->toggleOceanSimulation();
#endif

	ship = new Ship();
	ship->createObject(mSceneMgr, this->mBulletWorld, this->mNumEntitiesInstanced);
	robot->ship = ship;
	ship->mHydrax = mHydrax;

	switchLever = new SwitchLever();
	switchLever->createObject(this, this->mNumEntitiesInstanced);

	switchLever->robot = robot;
	switchLever->vehicle = vehicle;
	switchLever->rocket = rocket;

	flag = new Flag();
	flag->createObject(this);

	obs = new ObstacleForFun();
	obs->createObject(this, this->mNumEntitiesInstanced);
	
#ifdef _DEBUG
	mBulletWorld->getDebugDrawer()->setDrawWireframe(true);
	mBulletWorld->setShowDebugShapes(true);
#endif

	chars.push_back(vehicle);		// 00
	chars.push_back(robot);			// 01
	chars.push_back(ship);			// 02
	chars.push_back(rocket);		// 03 
	chars.push_back(fancyTerrain);	// 04
	//chars.push_back(switchLever);	// 06

	this->changeCameraPosition(4);	// change it to terrain
}

//-------------------------------------------------------------------------------------
void AZBullet::createHydraxSimulation()
{
	mHydrax = new Hydrax::Hydrax(mSceneMgr, mCamera, mCamera->getViewport());

	Hydrax::Module::ProjectedGrid *mModule 
		= new Hydrax::Module::ProjectedGrid(
			mHydrax,	
			new Hydrax::Noise::Perlin(),
			Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),	
			Hydrax::MaterialManager::NM_VERTEX,	
			Hydrax::Module::ProjectedGrid::Options());	
	
	mHydrax->setModule(static_cast<Hydrax::Module::Module*>(mModule));
	mHydrax->loadCfg("HydraxDemo.hdx");
	mHydrax->create();
}

//-------------------------------------------------------------------------------------
void AZBullet::createSimpleWater()
{
	Ogre::Plane plane01(Ogre::Vector3::UNIT_Y, 0);

	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane01, 3000, 3000, 20, 20, true, 1, 20, 20, Ogre::Vector3::UNIT_Z);

	Ogre::Entity* entWater01 = mSceneMgr->createEntity("WaterPlane01", "ground");
	waterNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Water", Ogre::Vector3(0, 45, 0));
	waterNode->attachObject(entWater01);

	entWater01->setMaterialName("Examples/Water2");
	entWater01->setCastShadows(false);	

	waterNode->setVisible(false);
}

//-------------------------------------------------------------------------------------
bool AZBullet::frameRenderingQueued(const Ogre::FrameEvent& arg)
{
	//----------------------------------------------------------------------------------
	Vector3 vPos = vehicle->getObjectPosition();
	Vector3 rPos = robot->getObjectPosition();

	//std::cout << vPos.y << "\n";
	//std::cout << rPos.y << "\n";

	if(vPos.y < 10.0f || rPos.y < 10.0f) 
	{
		menu->setGameOver();
	}

	//----------------------------------------------------------------------------------
	if(!BaseApplication::frameRenderingQueued(arg)) { return false; }
	Ogre::Real elapsedTime = arg.timeSinceLastFrame;

	if(this->isHydraxEnabled) mHydrax->update(elapsedTime);

	// step simulation -----------------------------------------------------------------
	Ogre::Real fps = mWindow->getAverageFPS();

	int step = 1;
	if(fps < 45.0f) step = 2;

	mBulletWorld->stepSimulation(elapsedTime, step);	
	
	//this->repositionCamera();	

	for(int a = 0; a < chars.size(); a++) { chars[a]->updatePerFrame(elapsedTime); }
	switchLever->updatePerFrame(elapsedTime);
	flag->updatePerFrame(elapsedTime);

	if(this->vehicle->speed >= 125.0f) { this->compSample->SetMotionBlur(true); }
	else { this->compSample->SetMotionBlur(false); }

	soundManager->update(elapsedTime);

	menu->update(this->mWindow);	// update Hikari
	return true;
}

// -------------------------------------------------------------------------
bool AZBullet::frameStarted(const FrameEvent& evt)
{
	Ogre::Real elapsedTime = evt.timeSinceLastFrame;	
	mCameraListener->frameStarted(evt);
	return true;
}

// -------------------------------------------------------------------------
bool AZBullet::frameEnded(const FrameEvent& evt)
{
	Ogre::Real elapsedTime = evt.timeSinceLastFrame;
	mCameraListener->frameEnded(evt);
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

	bool val = menu->hikariMgr->injectMouseMove(arg.state.X.abs, arg.state.Y.abs) ||  menu->hikariMgr->injectMouseWheel(arg.state.Z.rel);

	//if the left mouse button is held down
	if(bLMouseDown)
	{		
	}
	else if(bRMouseDown)	//if the right mouse button is held down, be rotate the camera with the mouse
	{		
		mCamera->yaw(Ogre::Degree(-arg.state.X.rel * mRotateSpeed));
		mCamera->pitch(Ogre::Degree(-arg.state.Y.rel * mRotateSpeed));
	}

	return val;
}

//-------------------------------------------------------------------------------------
// when a mouse button is pressed
bool AZBullet::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{	
	if(id == OIS::MB_Left) { bLMouseDown = true; }
	else if(id == OIS::MB_Right) { bRMouseDown = true; }
		
	return  menu->hikariMgr->injectMouseDown(id); 
}

//-------------------------------------------------------------------------------------
// when a mouse button is released
bool AZBullet::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	if(id  == OIS::MB_Left) { bLMouseDown = false; }
	else if(id == OIS::MB_Right) { bRMouseDown = false; }

	return  menu->hikariMgr->injectMouseUp(id);
}

//------------------------------------------------------------------------------------- 
// when keyboard pressed
bool AZBullet::keyPressed(const OIS::KeyEvent& arg)
{
	if(!BaseApplication::keyPressed(arg)) { return false; }

	if(arg.key == OIS::KC_UP     || 
		arg.key == OIS::KC_RIGHT || 
		arg.key == OIS::KC_LEFT  || 
		arg.key == OIS::KC_DOWN
		)
	{
	}
	else if(arg.key == OIS::KC_1) { changeCameraPosition(0); }
	else if(arg.key == OIS::KC_2) { changeCameraPosition(1); }
	else if(arg.key == OIS::KC_3) { changeCameraPosition(2); }
	else if(arg.key == OIS::KC_4) { changeCameraPosition(3); }
	else if(arg.key == OIS::KC_5) { changeCameraPosition(4); }
	else { mCameraMan->injectKeyDown(arg); }

	for(int a = 0; a < chars.size(); a++)
	{
		chars[a]->keyPressed(arg);
	}

	return true;
}

//------------------------------------------------------------------------------------- 
// when keyboard pressed
bool AZBullet::keyReleased(const OIS::KeyEvent& arg)
{
	if(!BaseApplication::keyReleased(arg)) { return false; }

	if(arg.key == OIS::KC_UP     || 
		arg.key == OIS::KC_RIGHT || 
		arg.key == OIS::KC_LEFT  || 
		arg.key == OIS::KC_DOWN )
	{
	}
	else { mCameraMan->injectKeyUp(arg); }

	for(int a = 0; a < chars.size(); a++)
	{
		chars[a]->keyReleased(arg);
	}

	return true;
}

//-------------------------------------------------------------------------------------
void AZBullet::repositionCamera()
{
	/*
	//Ogre::Vector3 camPos = mCamera->getRealPosition();
	//Ogre::Vector3 camPos = exCamera->getCameraPosition();
	Ogre::Vector3 camPos = Ogre::Vector3::ZERO;

	Ogre::Ray cameraRay(Ogre::Vector3(camPos.x, 5000.0f, camPos.z), Ogre::Vector3::NEGATIVE_UNIT_Y);
	mRayScnQuery->setRay(cameraRay);

	Ogre::RaySceneQueryResult &result = mRayScnQuery->execute();
	Ogre::RaySceneQueryResult::iterator itr = result.begin();

	if (itr != result.end() && itr->worldFragment)
	{
		Ogre::Real terrainHeight = itr->worldFragment->singleIntersection.y;
		if( Ogre::Math::Abs(terrainHeight - camPos.y) > 1.0f )
		{
			//std::cout << itr->worldFragment->singleIntersection.y << "-";
			//std::cout << camPos.y << "\n";
			//std::cout << "whoops\n";
			//exCamera->instantUpdate(Ogre::Vector3(camPos.x, terrainHeight + 1.0f, camPos.z));
			//mCamera->setPosition( camPos.x, terrainHeight + 0.5f, camPos.z );
		}
	}
	*/
}

//-------------------------------------------------------------------------------------
void AZBullet::toggleOceanSimulation()
{
	this->isHydraxEnabled = !this->isHydraxEnabled;
	
	if(this->isHydraxEnabled)	// hydrax activated
	{
		//mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
		this->mHydrax->setVisible(true);
		waterNode->setVisible(false);
	}
	else					   // hydrax deactivated 
	{
		//mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);		
		this->mHydrax->setVisible(false);
		waterNode->setVisible(true);
	}
}

//-------------------------------------------------------------------------------------
void AZBullet::setWeather(int val)
{
	this->mCurrentSkyBox = val;
	this->changeSkyBox();
}

//-------------------------------------------------------------------------------------
void AZBullet::changeCompositor(int val)
{
	compSample->setCompositorEnabled("HDR", false);
	compSample->setCompositorEnabled("Radial Blur", false);
	compSample->setCompositorEnabled("Bloom", false);

	if(val == 1)
	{
		compSample->setCompositorEnabled("HDR", true);
	}
	else if(val == 2)
	{
		compSample->setCompositorEnabled("Bloom", true);
	}
	else if(val == 3)
	{
		compSample->setCompositorEnabled("Radial Blur", true);
	}
}

//-------------------------------------------------------------------------------------
void AZBullet::changeCameraPosition(int val)
{
	mCamera->setPosition(Ogre::Vector3::ZERO);
	mCamera->setOrientation(Ogre::Quaternion(1, 0, 0));

	for(int a = 0; a < chars.size(); a++) { chars[a]->isFocus = false; }
	
	if(val == 0) 
	{ 
		static_cast<CameraListener*> (mCameraListener)->setCharacter(vehicle); 
		vehicle->isFocus = true;
	}
	else if(val == 1) 
	{ 
		static_cast<CameraListener*> (mCameraListener)->setCharacter(robot); 
		robot->isFocus = true;
	}
	else if(val == 2) 
	{ 
		static_cast<CameraListener*> (mCameraListener)->setCharacter(ship); 
		ship->isFocus = true;
	}
	else if(val == 3) 
	{ 
		static_cast<CameraListener*> (mCameraListener)->setCharacter(rocket); 
		rocket->isFocus = true;
	}
	else if(val == 4) 
	{ 
		static_cast<CameraListener*> (mCameraListener)->setCharacter(fancyTerrain); 
		fancyTerrain->isFocus = true;
	}

	mCameraListener->instantUpdate();
}

//-------------------------------------------------------------------------------------
// End the application
void AZBullet::shutdownApp(void)
{
	this->mShutDown = true;
}

//-------------------------------------------------------------------------------------
void AZBullet::createFrameListener(void)
{
	BaseApplication::createFrameListener();

	this->bLMouseDown = false;
	this->bRMouseDown = false;
	this->mRotateSpeed = 0.1f;

	//mRayScnQuery = mSceneMgr->createRayQuery(Ogre::Ray());	
}


