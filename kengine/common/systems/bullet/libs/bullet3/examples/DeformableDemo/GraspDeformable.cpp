/*
 Bullet Continuous Collision Detection and Physics Library
 Copyright (c) 2019 Google Inc. http://bulletphysics.org
 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it freely,
 subject to the following restrictions:
 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */

#include "GraspDeformable.h"
///btBulletDynamicsCommon.h is the main Bullet include file, contains most common include files.
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btDeformableMultiBodyDynamicsWorld.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "BulletSoftBody/btDeformableBodySolver.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h"
#include "BulletDynamics/Featherstone/btMultiBodyJointMotor.h"
#include <stdio.h>  //printf debugging

#include "../CommonInterfaces/CommonRigidBodyBase.h"
#include "../Utils/b3ResourcePath.h"
#include "../Importers/ImportURDFDemo/BulletUrdfImporter.h"
#include "../Importers/ImportURDFDemo/MyMultiBodyCreator.h"
#include "../Importers/ImportURDFDemo/URDF2Bullet.h"
#include "../CommonInterfaces/CommonMultiBodyBase.h"
#include "../CommonInterfaces/CommonGraphicsAppInterface.h"
#include "../CommonInterfaces/CommonParameterInterface.h"
#include "../CommonInterfaces/CommonFileIOInterface.h"
#include "Bullet3Common/b3FileUtils.h"

///The GraspDeformable shows the use of rolling friction.
///Spheres will come to a rest on a sloped plane using a constraint. Damping cannot achieve the same.
///Generally it is best to leave the rolling friction coefficient zero (or close to zero).
static btScalar sGripperVerticalVelocity = 0.f;
static btScalar sGripperClosingTargetVelocity = 0.f;
static float friction = 1.;
struct TetraCube
{
#include "../SoftDemo/cube.inl"
};

struct TetraBunny
{
#include "../SoftDemo/bunny.inl"
};

static bool supportsJointMotor(btMultiBody* mb, int mbLinkIndex)
{
    bool canHaveMotor = (mb->getLink(mbLinkIndex).m_jointType == btMultibodyLink::eRevolute
                         || mb->getLink(mbLinkIndex).m_jointType == btMultibodyLink::ePrismatic);
    return canHaveMotor;
}

class GraspDeformable : public CommonRigidBodyBase
{
    btAlignedObjectArray<btDeformableLagrangianForce*> m_forces;
public:
	GraspDeformable(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~GraspDeformable()
	{
	}
	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
        float dist = 2;
        float pitch = -45;
        float yaw = 100;
        float targetPos[3] = {0, -0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
    
    btMultiBody* createFeatherstoneMultiBody(btMultiBodyDynamicsWorld* pWorld,const btVector3& basePosition, const btVector3& baseHalfExtents, const btVector3& linkHalfExtents, bool floating);
    
    void addColliders(btMultiBody* pMultiBody, btMultiBodyDynamicsWorld* pWorld, const btVector3& baseHalfExtents, const btVector3& linkHalfExtents);
    
    btMultiBody* createFeatherstoneMultiBody_testMultiDof(btMultiBodyDynamicsWorld* pWorld, int numLinks, const btVector3& basePosition, const btVector3& baseHalfExtents, const btVector3& linkHalfExtents, bool spherical, bool floating);
    
    void stepSimulation(float deltaTime)
    {
        double fingerTargetVelocities[2] = {sGripperVerticalVelocity, sGripperClosingTargetVelocity};
        int num_multiBody = getDeformableDynamicsWorld()->getNumMultibodies();
        for (int i = 0; i < num_multiBody; ++i)
        {
            btMultiBody* mb = getDeformableDynamicsWorld()->btMultiBodyDynamicsWorld::getMultiBody(i);
            mb->setBaseVel(btVector3(0,sGripperVerticalVelocity, 0));
            int dofIndex = 6;  //skip the 3 linear + 3 angular degree of freedom entries of the base
            for (int link = 0; link < mb->getNumLinks(); link++)
            {
                if (supportsJointMotor(mb, link))
                {
                    btMultiBodyJointMotor* motor = (btMultiBodyJointMotor*)mb->getLink(link).m_userPtr;
                    if (motor)
                    {
//                        if (dofIndex == 10 || dofIndex == 11)
//                        {
//                            motor->setVelocityTarget(fingerTargetVelocities[1], 1);
//                            motor->setMaxAppliedImpulse(1);
//                        }
                        if (dofIndex == 6)
                        {
                            motor->setVelocityTarget(-fingerTargetVelocities[1], 1);
                            motor->setMaxAppliedImpulse(2);
                        }
                        if (dofIndex == 7)
                        {
                            motor->setVelocityTarget(fingerTargetVelocities[1], 1);
                            motor->setMaxAppliedImpulse(2);
                        }
//                        motor->setRhsClamp(SIMD_INFINITY);
                        motor->setMaxAppliedImpulse(1);
                    }
                }
                dofIndex += mb->getLink(link).m_dofCount;
            }
        }
        
        //use a smaller internal timestep, there are stability issues
        float internalTimeStep = 1. / 250.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
    }
    
    void createGrip()
    {
        int count = 2;
        float mass = 2;
        btCollisionShape* shape[] = {
            new btBoxShape(btVector3(3, 3, 0.5)),
        };
        static const int nshapes = sizeof(shape) / sizeof(shape[0]);
        for (int i = 0; i < count; ++i)
        {
            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(btVector3(10, 0, 0));
            startTransform.setRotation(btQuaternion(btVector3(1, 0, 0), SIMD_PI * 0.));
            createRigidBody(mass, startTransform, shape[i % nshapes]);
        }
    }
    
    virtual const btDeformableMultiBodyDynamicsWorld* getDeformableDynamicsWorld() const
    {
        return (btDeformableMultiBodyDynamicsWorld*)m_dynamicsWorld;
    }
    
    virtual btDeformableMultiBodyDynamicsWorld* getDeformableDynamicsWorld()
    {
        return (btDeformableMultiBodyDynamicsWorld*)m_dynamicsWorld;
    }
    
    virtual void renderScene()
    {
        CommonRigidBodyBase::renderScene();
        btDeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();
        
        for (int i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
        {
            btSoftBody* psb = (btSoftBody*)deformableWorld->getSoftBodyArray()[i];
            {
                btSoftBodyHelpers::DrawFrame(psb, deformableWorld->getDebugDrawer());
                btSoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
            }
        }
    }
};


void GraspDeformable::initPhysics()
{
	m_guiHelper->setUpAxis(1);
    
    m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new btDbvtBroadphase();
    btDeformableBodySolver* deformableBodySolver = new btDeformableBodySolver();

	btDeformableMultiBodyConstraintSolver* sol = new btDeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(deformableBodySolver);
	m_solver = sol;

	m_dynamicsWorld = new btDeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, deformableBodySolver);
//    deformableBodySolver->setWorld(getDeformableDynamicsWorld());
    btVector3 gravity = btVector3(0, -9.81, 0);
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

//    // load a gripper
//    {
//        btTransform rootTrans;
//        rootTrans.setIdentity();
//        BulletURDFImporter u2b(m_guiHelper,0,0,50,0);
//        bool forceFixedBase = false;
//        bool loadOk = u2b.loadSDF("gripper/wsg50_one_motor_gripper_new.sdf", forceFixedBase);
//        if (loadOk)
//        {
//            for (int m = 0; m < u2b.getNumModels(); m++)
//            {
//                u2b.activateModel(m);
//
//                btMultiBody* mb = 0;
//
//                MyMultiBodyCreator creation(m_guiHelper);
//
//                u2b.getRootTransformInWorld(rootTrans);
//                ConvertURDF2Bullet(u2b, creation, rootTrans, getDeformableDynamicsWorld(), true, u2b.getPathPrefix(), CUF_USE_SDF+CUF_RESERVED);
//                mb = creation.getBulletMultiBody();
//
//                int numLinks = mb->getNumLinks();
//                for (int i = 0; i < numLinks; i++)
//                {
//                    int mbLinkIndex = i;
//                    float maxMotorImpulse = 1.f;
//
//                    if (supportsJointMotor(mb, mbLinkIndex))
//                    {
//                        int dof = 0;
//                        btScalar desiredVelocity = 0.f;
//                        btMultiBodyJointMotor* motor = new btMultiBodyJointMotor(mb, mbLinkIndex, dof, desiredVelocity, maxMotorImpulse);
//                        motor->setPositionTarget(0, 0);
//                        motor->setVelocityTarget(0, 1);
//                        mb->getLink(mbLinkIndex).m_userPtr = motor;
//                        getDeformableDynamicsWorld()->addMultiBodyConstraint(motor);
//                        motor->finalizeMultiDof();
//                    }
//                }
//            }
//        }
//    }
    // build a gripper
    {
        bool damping = true;
        bool gyro = false;
        bool canSleep = false;
        bool selfCollide = true;
        int numLinks = 2;
        btVector3 linkHalfExtents(.1, .2, .04);
        btVector3 baseHalfExtents(.1, 0.02, .2);
        btMultiBody* mbC = createFeatherstoneMultiBody(getDeformableDynamicsWorld(), btVector3(0.f, .7f,0.f), linkHalfExtents, baseHalfExtents, false);
        
        mbC->setCanSleep(canSleep);
        mbC->setHasSelfCollision(selfCollide);
        mbC->setUseGyroTerm(gyro);

        for (int i = 0; i < numLinks; i++)
        {
            int mbLinkIndex = i;
            float maxMotorImpulse = 1.f;

            if (supportsJointMotor(mbC, mbLinkIndex))
            {
                int dof = 0;
                btScalar desiredVelocity = 0.f;
                btMultiBodyJointMotor* motor = new btMultiBodyJointMotor(mbC, mbLinkIndex, dof, desiredVelocity, maxMotorImpulse);
                motor->setPositionTarget(0, 0);
                motor->setVelocityTarget(0, 1);
                mbC->getLink(mbLinkIndex).m_userPtr = motor;
                getDeformableDynamicsWorld()->addMultiBodyConstraint(motor);
                motor->finalizeMultiDof();
            }
        }
        
        
        if (!damping)
        {
            mbC->setLinearDamping(0.0f);
            mbC->setAngularDamping(0.0f);
        }
        else
        {
            mbC->setLinearDamping(0.04f);
            mbC->setAngularDamping(0.04f);
        }
        btScalar q0 = 0.f * SIMD_PI / 180.f;
        if (numLinks > 0)
            mbC->setJointPosMultiDof(0, &q0);
        ///
        addColliders(mbC, getDeformableDynamicsWorld(), baseHalfExtents, linkHalfExtents);

    }
    //create a ground
    {
        btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(150.), btScalar(25.), btScalar(150.)));

        m_collisionShapes.push_back(groundShape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, -25-.6, 0));
        groundTransform.setRotation(btQuaternion(btVector3(1, 0, 0), SIMD_PI * 0));
        //We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
        btScalar mass(0.);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            groundShape->calculateLocalInertia(mass, localInertia);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(0.1);

        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body,1,1+2);
    }

    // create a soft block
    {
        char relative_path[1024];
//        b3FileUtils::findFile("banana.vtk", relative_path, 1024);
        b3FileUtils::findFile("ball.vtk", relative_path, 1024);
//        b3FileUtils::findFile("single_tet.vtk", relative_path, 1024);
//         b3FileUtils::findFile("tube.vtk", relative_path, 1024);
//         b3FileUtils::findFile("torus.vtk", relative_path, 1024);
//         b3FileUtils::findFile("paper_roll.vtk", relative_path, 1024);
//         b3FileUtils::findFile("bread.vtk", relative_path, 1024);
//        b3FileUtils::findFile("ditto.vtk", relative_path, 1024);
//        b3FileUtils::findFile("boot.vtk", relative_path, 1024);
//        btSoftBody* psb = btSoftBodyHelpers::CreateFromTetGenData(getDeformableDynamicsWorld()->getWorldInfo(),
//                                                                  TetraCube::getElements(),
//                                                                  0,
//                                                                  TetraCube::getNodes(),
//                                                                  false, true, true);
        btSoftBody* psb = btSoftBodyHelpers::CreateFromVtkFile(getDeformableDynamicsWorld()->getWorldInfo(), relative_path);

//        psb->scale(btVector3(30, 30, 30)); // for banana
        psb->scale(btVector3(.25, .25, .25));
//        psb->scale(btVector3(.3, .3, .3));  // for tube, torus, boot
//        psb->scale(btVector3(1, 1, 1));  // for ditto
        psb->translate(btVector3(.25, 0, 0.4)); 
        psb->getCollisionShape()->setMargin(0.02);
        psb->setTotalMass(.1);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = 2;
        psb->m_cfg.collisions = btSoftBody::fCollision::SDF_RD;
        getDeformableDynamicsWorld()->addSoftBody(psb);
        
        btDeformableMassSpringForce* mass_spring = new btDeformableMassSpringForce(.0,.04, true);
        getDeformableDynamicsWorld()->addForce(psb, mass_spring);
        m_forces.push_back(mass_spring);
        
        btDeformableGravityForce* gravity_force =  new btDeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        
        btDeformableNeoHookeanForce* neohookean = new btDeformableNeoHookeanForce(5,10);
        getDeformableDynamicsWorld()->addForce(psb, neohookean);
        m_forces.push_back(neohookean);
    }
    
//    // create a piece of cloth
//    {
//        bool onGround = false;
//        const btScalar s = 4;
//        btSoftBody* psb = btSoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), btVector3(-s, 0, -s),
//                                                         btVector3(+s, 0, -s),
//                                                         btVector3(-s, 0, +s),
//                                                         btVector3(+s, 0, +s),
//                                                         20,20,
//                                                                  0, true);
//
//        if (onGround)
//            psb = btSoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), btVector3(-s, 0, -s),
//                                                 btVector3(+s, 0, -s),
//                                                 btVector3(-s, 0, +s),
//                                                 btVector3(+s, 0, +s),
//                                                 //                                                 20,20,
//                                                 2,2,
//                                                 0, true);
//
//        psb->getCollisionShape()->setMargin(0.1);
//        psb->generateBendingConstraints(2);
//        psb->setTotalMass(1);
//        psb->setSpringStiffness(2);
//        psb->setDampingCoefficient(0.03);
//        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
//        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
//        psb->m_cfg.kDF = 1;
//        psb->m_cfg.collisions = btSoftBody::fCollision::SDF_RD;
//        getDeformableDynamicsWorld()->addSoftBody(psb);
//        getDeformableDynamicsWorld()->addForce(psb, new btDeformableMassSpringForce());
//        getDeformableDynamicsWorld()->addForce(psb, new btDeformableGravityForce(gravity));
//    }
    
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
    
    {
        SliderParams slider("Moving velocity", &sGripperVerticalVelocity);
        slider.m_minVal = -.2;
        slider.m_maxVal = .2;
        m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }
    
    {
        SliderParams slider("Closing velocity", &sGripperClosingTargetVelocity);
        slider.m_minVal = -.3;
        slider.m_maxVal = .3;
        m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }
    
}

void GraspDeformable::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	int i;
	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}
    // delete forces
    for (int j = 0; j < m_forces.size(); j++)
    {
        btDeformableLagrangianForce* force = m_forces[j];
        delete force;
    }
    m_forces.clear();
    
	//delete collision shapes
	for (int j = 0; j < m_collisionShapes.size(); j++)
	{
		btCollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_dynamicsWorld;

	delete m_solver;

	delete m_broadphase;

	delete m_dispatcher;

	delete m_collisionConfiguration;
}

btMultiBody* GraspDeformable::createFeatherstoneMultiBody(btMultiBodyDynamicsWorld* pWorld, const btVector3& basePosition, const btVector3& baseHalfExtents, const btVector3& linkHalfExtents, bool floating)
{
    //init the base
    btVector3 baseInertiaDiag(0.f, 0.f, 0.f);
    float baseMass = 100.f;
    float linkMass = 100.f;
    int numLinks = 2;
    
    if (baseMass)
    {
        btCollisionShape* pTempBox = new btBoxShape(btVector3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));
        pTempBox->calculateLocalInertia(baseMass, baseInertiaDiag);
        delete pTempBox;
    }
    
    bool canSleep = false;
    btMultiBody* pMultiBody = new btMultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);
    
    btQuaternion baseOriQuat(0.f, 0.f, 0.f, 1.f);
    pMultiBody->setBasePos(basePosition);
    pMultiBody->setWorldToBaseRot(baseOriQuat);
    
    //init the links
    btVector3 hingeJointAxis(1, 0, 0);
    
    btVector3 linkInertiaDiag(0.f, 0.f, 0.f);
    
    btCollisionShape* pTempBox = new btBoxShape(btVector3(linkHalfExtents[0], linkHalfExtents[1], linkHalfExtents[2]));
    pTempBox->calculateLocalInertia(linkMass, linkInertiaDiag);
    delete pTempBox;
    
    //y-axis assumed up
    btAlignedObjectArray<btVector3> parentComToCurrentCom;
    parentComToCurrentCom.push_back(btVector3(0, -linkHalfExtents[1] * 8.f, -baseHalfExtents[2] * 4.f));
    parentComToCurrentCom.push_back(btVector3(0, -linkHalfExtents[1] * 8.f, +baseHalfExtents[2] * 4.f));//par body's COM to cur body's COM offset
    
    btVector3 currentPivotToCurrentCom(0, -linkHalfExtents[1]*8.f, 0);                         //cur body's COM to cur body's PIV offset
    
    btAlignedObjectArray<btVector3> parentComToCurrentPivot;
    parentComToCurrentPivot.push_back(btVector3(parentComToCurrentCom[0] - currentPivotToCurrentCom));
    parentComToCurrentPivot.push_back(btVector3(parentComToCurrentCom[1] - currentPivotToCurrentCom));//par body's COM to cur body's PIV offset
    
    //////
    btScalar q0 = 0.f * SIMD_PI / 180.f;
    btQuaternion quat0(btVector3(0, 1, 0).normalized(), q0);
    quat0.normalize();
    /////
    
    for (int i = 0; i < numLinks; ++i)
    {
        pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, - 1, btQuaternion(0.f, 0.f, 0.f, 1.f), hingeJointAxis, parentComToCurrentPivot[i], currentPivotToCurrentCom, true);
    }
    pMultiBody->finalizeMultiDof();
    ///
    pWorld->addMultiBody(pMultiBody);
    ///
    return pMultiBody;
}

void GraspDeformable::addColliders(btMultiBody* pMultiBody, btMultiBodyDynamicsWorld* pWorld, const btVector3& baseHalfExtents, const btVector3& linkHalfExtents)
{
    btAlignedObjectArray<btQuaternion> world_to_local;
    world_to_local.resize(pMultiBody->getNumLinks() + 1);
    
    btAlignedObjectArray<btVector3> local_origin;
    local_origin.resize(pMultiBody->getNumLinks() + 1);
    world_to_local[0] = pMultiBody->getWorldToBaseRot();
    local_origin[0] = pMultiBody->getBasePos();
    
    {
        btScalar quat[4] = {-world_to_local[0].x(), -world_to_local[0].y(), -world_to_local[0].z(), world_to_local[0].w()};
        
        if (1)
        {
            btCollisionShape* box = new btBoxShape(baseHalfExtents);
            btMultiBodyLinkCollider* col = new btMultiBodyLinkCollider(pMultiBody, -1);
            col->setCollisionShape(box);
            
            btTransform tr;
            tr.setIdentity();
            tr.setOrigin(local_origin[0]);
            tr.setRotation(btQuaternion(quat[0], quat[1], quat[2], quat[3]));
            col->setWorldTransform(tr);
            
            pWorld->addCollisionObject(col, 2, 1 + 2);
            
            col->setFriction(friction);
            pMultiBody->setBaseCollider(col);
        }
    }
    
    for (int i = 0; i < pMultiBody->getNumLinks(); ++i)
    {
        const int parent = pMultiBody->getParent(i);
        world_to_local[i + 1] = pMultiBody->getParentToLocalRot(i) * world_to_local[parent + 1];
        local_origin[i + 1] = local_origin[parent + 1] + (quatRotate(world_to_local[i + 1].inverse(), pMultiBody->getRVector(i)));
    }
    
    for (int i = 0; i < pMultiBody->getNumLinks(); ++i)
    {
        btVector3 posr = local_origin[i + 1];
        
        btScalar quat[4] = {-world_to_local[i + 1].x(), -world_to_local[i + 1].y(), -world_to_local[i + 1].z(), world_to_local[i + 1].w()};
        
        btCollisionShape* box = new btBoxShape(linkHalfExtents);
        btMultiBodyLinkCollider* col = new btMultiBodyLinkCollider(pMultiBody, i);
        
        col->setCollisionShape(box);
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(posr);
        tr.setRotation(btQuaternion(quat[0], quat[1], quat[2], quat[3]));
        col->setWorldTransform(tr);
        col->setFriction(friction);
        pWorld->addCollisionObject(col, 2, 1 + 2);
        
        pMultiBody->getLink(i).m_collider = col;
    }
}

class CommonExampleInterface* GraspDeformableCreateFunc(struct CommonExampleOptions& options)
{
	return new GraspDeformable(options.m_guiHelper);
}


