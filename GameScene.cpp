#include "GameScene.hpp"
#include <bullet/BulletDynamics/btBulletDynamicsCommon.h>
#include "LevelGenerator.hpp"

#include <iostream>
// used in the camera initialization: wtf is the meaning of this?
// used also in collision detection for the start and end
// TODO: CODE DUPLICATION! This is also in main.cpp

bool MainGameScene::initialize(GameLoop& gameloop) {

	scene::ISceneManager * smgr = gameloop.smgr; // TODO: initialize
	scene::ISceneNode * parent = nullptr;


    core::vector3d<int> level_size = core::vector3d<int>(50,100,50);
    LevelGenerator level = LevelGenerator(level_size,1000);

    core::list<Structure> * structures = level.getStructures();

    // create the structures
    for (Structure& current: *structures) {
		StaticStructure* structure = new StaticStructure();
		current.size_x *= world_scale;
		current.size_y *= world_scale;
		current.size_z *= world_scale;
		current.pos_x *= world_scale;
		current.pos_y *= world_scale;
		current.pos_z *= world_scale;
		btRigidBody* rigidb = structure->initialize(gameloop, current);
		gameloop.dynamicsWorld->addRigidBody(rigidb);

    }

    // create start/end point
    vector3d<int> startcell = level.getStart();
    vector3df sp = core::vector3df(1.0/2) + vector3df(startcell.X, startcell.Y, startcell.Z);
	sp *= world_scale;
	
    scene::IMeshSceneNode * start_node = smgr -> addSphereSceneNode(0.5);
    start_node -> getMaterial(0).DiffuseColor = video::SColor(100,0,255,0);
    start_node -> getMaterial(0).AmbientColor = video::SColor(100,0,255,0);
    start_node -> getMaterial(0).EmissiveColor = video::SColor(100,0,255,0);
    start_node -> getMaterial(0).ColorMaterial = video::ECM_NONE;
    int ID_startnode = 0;
    start_node -> setID(ID_startnode);
    start_node -> setPosition(sp);

    vector3d<int> endcell = level.getEnd();
    vector3df ep = core::vector3df(1.0/2) + vector3df(endcell.X, endcell.Y, endcell.Z);
	ep *= world_scale;

    scene::IMeshSceneNode * end_node = smgr -> addSphereSceneNode(0.5);
    end_node -> getMaterial(0).DiffuseColor = video::SColor(100,255,100,255);
    end_node -> getMaterial(0).AmbientColor = video::SColor(100,255,100,255);
    end_node -> getMaterial(0).EmissiveColor = video::SColor(100,255,100,255);
    end_node -> getMaterial(0).ColorMaterial = video::ECM_NONE;

    int ID_endnode = 0;
    end_node -> setID(ID_endnode);
    end_node -> setPosition(ep);

    int ID_endlight = 0;
    scene::ILightSceneNode * end_light = smgr -> addLightSceneNode( end_node, core::vector3df(0,0,0), video::SColor(255,255,50,255), 10.0f, ID_endlight );

    // "God-ray" volumetric light
    int ID_godray = 0;
    scene::IVolumeLightSceneNode * godray = smgr->addVolumeLightSceneNode(0, ID_godray,
																		  32,                                // Subdivisions on U axis
																		  32,                                // Subdivisions on V axis
																		  video::SColor(0, 150, 20, 150),  // foot color
																		  video::SColor(0, 0, 0, 0)); // tail color

    if (godray){
		godray -> setScale(core::vector3df(3.0f, 1000.0f, 3.0f));
		godray -> setPosition(ep);

        // load textures for animation
        core::array<video::ITexture*> textures;
        for (s32 g=1; g < 8; g++){
            core::stringc tmp;
            tmp = "./media/portal";
            tmp += g;
            tmp += ".bmp";
            video::ITexture* t = smgr -> getVideoDriver() -> getTexture( tmp.c_str() );
            textures.push_back(t);
        }

        // create texture animator
        scene::ISceneNodeAnimator* glow = smgr->createTextureAnimator(textures, 100);

        // add the animator
        godray->addAnimator(glow);

        // drop the animator because it was created with a create() function
        glow->drop();
    }


    int ID_light = 0;
    scene::ILightSceneNode* light2 = smgr -> addLightSceneNode( 0, core::vector3df(1000,1000,1000), video::SColor(255,255,0,0), 50.0f, ID_light );


    // scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
    //     "./media/terrain-heightmap.png",
    //     0,                  // parent node
    //     -1,                 // node id
    //     core::vector3df(0.f, 0.f, 0.f),     // position
    //     core::vector3df(0.f, 0.f, 0.f),     // rotation
    //     core::vector3df(level_size.X/4, 20.4f, level_size.Z/4),  // scale
    //     video::SColor ( 255, 255, 255, 255 ),   // vertexColor
    //     5,                  // maxLOD
    //     scene::ETPS_17,             // patchSize
    //     4                   // smoothFactor
    //     );

    // terrain->setMaterialFlag(video::EMF_LIGHTING, false);

    // terrain->setMaterialTexture(0,
    //         smgr -> getVideoDriver() -> getTexture("./media/terrain-texture.png"));
    // terrain->setMaterialTexture(1,smgr -> getVideoDriver() -> getTexture("./media/terrain-texture.png"));

    // terrain->setMaterialType(video::EMT_DETAIL_MAP);

    // terrain->scaleTexture(1.0f, 20.0f);


    //   // create triangle selector for the terrain
    // scene::ITriangleSelector* selector2
    //     = smgr->createTerrainTriangleSelector(terrain, 0);
    // terrain->setTriangleSelector(selector2);
	// metaselector->addTriangleSelector(selector2);


	{ // player obj test
		playerObj = new PlayerGameObj();
		playerObj->initialize(gameloop);
		// TODO: All of the following should be moved into playergameobj.initialize 
		
		std::cout<<"START PLAYER POS:"<<sp.X<<","<<sp.Y<<","<<sp.Z<<","<<std::endl;
		// from euler angles, other constructors should be preferred (I
		// think)
		btQuaternion init_rotation(btScalar(0.01),btScalar(0.01),btScalar(0.01));
		// initial position of the rigid body.
		btVector3 init_position(btScalar(sp.X), btScalar(sp.Y), btScalar(sp.Z));
		// initialize the rigid body transform.
		btTransform transform(init_rotation, init_position);
		// set motion state. TODO: Find better way to do this, such as passing the position to the constructor.
		playerObj->setWorldTransform(transform);

		// model of the player in the physicial world: A BIG ROUND SPHERE: you fat!
		btCollisionShape* sphere = new btSphereShape(0.2);

		// inertia vector.
		btVector3 inertiavector(0.1,0.1,0.1);

		// add the rigid body
		// mass of 80 kg.

		btRigidBody* test = new btRigidBody(80, playerObj, sphere, inertiavector);
		gameloop.dynamicsWorld->addRigidBody(test);

		// just as a test, start with some initial velocity
		test->applyCentralImpulse(btVector3(30, -200, 30));
	}

    return true;
}

void MainGameScene::render(GameLoop&, float) {
}

void MainGameScene::logic_tick(GameLoop&) {
}




void StaticStructure::logic_tick(GameLoop&) { }

void StaticStructure::render(GameLoop&, float) { }

btRigidBody* StaticStructure::initialize(GameLoop& loop, const Structure& current) {

	core::vector3df position = core::vector3df(current.pos_x, current.pos_y, current.pos_z);
	core::vector3df size = core::vector3df(current.size_x, current.size_y, current.size_z);

	int ID_mapnode;
	scene::IAnimatedMeshSceneNode* map_node = loop.smgr -> addAnimatedMeshSceneNode(loop.smgr -> getMesh("./media/cube.3ds"), nullptr, ID_mapnode);
	map_node -> setScale(size);
	map_node -> setPosition(position);
	map_node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);


	switch (rand() % 3) {
	case 0 :
		map_node -> getMaterial(0).AmbientColor = video::SColor(255,255,0,0);
		map_node -> getMaterial(0).DiffuseColor = video::SColor(100,10,10,255);
		map_node -> setMaterialTexture( 0, loop.smgr -> getVideoDriver() -> getTexture("./media/blue.png") );
		break;
	case 1 :
		map_node -> getMaterial(0).AmbientColor = video::SColor(255,0,255,0);
		map_node -> getMaterial(0).DiffuseColor = video::SColor(100,255,10,10);
		map_node -> setMaterialTexture( 0, loop.smgr -> getVideoDriver() -> getTexture("./media/red.png") );
		break;
	case 2 :
		map_node -> getMaterial(0).AmbientColor = video::SColor(255,0,0,255);
		map_node -> getMaterial(0).DiffuseColor = video::SColor(100,10,255,10);
		map_node -> setMaterialTexture( 0, loop.smgr -> getVideoDriver() -> getTexture("./media/green.png") );
		break;
	}
	map_node -> setMaterialFlag(video::EMF_LIGHTING, true);
	//    map_node -> getMaterial(0).ColorMaterial = video::ECM_NONE;

	//size /=2;
	position += size/2;
	size /=2;
	btQuaternion init_rotation(btScalar(0),btScalar(0),btScalar(0));
	// initial position of the rigid body.
	btVector3 init_position(btScalar(position.X), btScalar(position.Y), btScalar(position.Z));
	// initialize the rigid body transform.
	btTransform transform(init_rotation, init_position);
	// set motion state. 
	this->setWorldTransform(transform);

	// model of the building is a box
	btCollisionShape* sphere = new btBoxShape(btVector3(size.X, size.Y, size.Z));

	// add a static rigid body (mass = 0).
	return new btRigidBody(0, this, sphere);
}