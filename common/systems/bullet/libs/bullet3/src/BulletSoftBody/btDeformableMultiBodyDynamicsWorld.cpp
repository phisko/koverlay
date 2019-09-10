/*
 Written by Xuchen Han <xuchenhan2015@u.northwestern.edu>
 
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

/* ====== Overview of the Deformable Algorithm ====== */

/*
A single step of the deformable body simulation contains the following main components:
1. Update velocity to a temporary state v_{n+1}^* = v_n + explicit_force * dt / mass, where explicit forces include gravity and elastic forces.
2. Detect collisions between rigid and deformable bodies at position x_{n+1}^* = x_n + dt * v_{n+1}^*.
3. Then velocities of deformable bodies v_{n+1} are solved in
        M(v_{n+1} - v_{n+1}^*) = damping_force * dt / mass,
   by a conjugate gradient solver, where the damping force is implicit and depends on v_{n+1}.
4. Contact constraints are solved as projections as in the paper by Baraff and Witkin https://www.cs.cmu.edu/~baraff/papers/sig98.pdf. Dynamic frictions are treated as a force and added to the rhs of the CG solve, whereas static frictions are treated as constraints similar to contact.
5. Position is updated via x_{n+1} = x_n + dt * v_{n+1}.
6. Apply position correction to prevent numerical drift.

The algorithm also closely resembles the one in http://physbam.stanford.edu/~fedkiw/papers/stanford2008-03.pdf
 */

#include <stdio.h>
#include "btDeformableMultiBodyDynamicsWorld.h"
#include "btDeformableBodySolver.h"
#include "LinearMath/btQuickprof.h"

void btDeformableMultiBodyDynamicsWorld::internalSingleStepSimulation(btScalar timeStep)
{
    BT_PROFILE("internalSingleStepSimulation");
    reinitialize(timeStep);
    // add gravity to velocity of rigid and multi bodys
    applyRigidBodyGravity(timeStep);
    
    ///apply gravity and explicit force to velocity, predict motion
    predictUnconstraintMotion(timeStep);
    
    ///perform collision detection
    btMultiBodyDynamicsWorld::performDiscreteCollisionDetection();
    
    btMultiBodyDynamicsWorld::calculateSimulationIslands();
    
    beforeSolverCallbacks(timeStep);
    
    ///solve deformable bodies constraints
    solveConstraints(timeStep);
    
    afterSolverCallbacks(timeStep);
    
    integrateTransforms(timeStep);
    
    ///update vehicle simulation
    btMultiBodyDynamicsWorld::updateActions(timeStep);
    
    btMultiBodyDynamicsWorld::updateActivationState(timeStep);
    // End solver-wise simulation step
    // ///////////////////////////////
}

void btDeformableMultiBodyDynamicsWorld::positionCorrection(btScalar timeStep)
{
    // perform position correction for all constraints 
    BT_PROFILE("positionCorrection");
    for (int index = 0; index < m_deformableBodySolver->m_objective->projection.m_constraints.size(); ++index)
    {
        DeformableContactConstraint& constraint = *m_deformableBodySolver->m_objective->projection.m_constraints.getAtIndex(index);
        for (int j = 0; j < constraint.m_contact.size(); ++j)
        {
            const btSoftBody::RContact* c = constraint.m_contact[j];
            // skip anchor points
            if (c == NULL || c->m_node->m_im == 0)
                continue;
            const btSoftBody::sCti& cti = c->m_cti;
            btVector3 va(0, 0, 0);
            
            // grab the velocity of the rigid body
            if (cti.m_colObj->getInternalType() == btCollisionObject::CO_RIGID_BODY)
            {
                btRigidBody* rigidCol = (btRigidBody*)btRigidBody::upcast(cti.m_colObj);
                va = rigidCol ? (rigidCol->getVelocityInLocalPoint(c->m_c1)): btVector3(0, 0, 0);
            }
            else if (cti.m_colObj->getInternalType() == btCollisionObject::CO_FEATHERSTONE_LINK)
            {
                btMultiBodyLinkCollider* multibodyLinkCol = (btMultiBodyLinkCollider*)btMultiBodyLinkCollider::upcast(cti.m_colObj);
                if (multibodyLinkCol)
                {
                    const int ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
                    const btScalar* J_n = &c->jacobianData_normal.m_jacobians[0];
                    const btScalar* J_t1 = &c->jacobianData_t1.m_jacobians[0];
                    const btScalar* J_t2 = &c->jacobianData_t2.m_jacobians[0];
                    const btScalar* local_v = multibodyLinkCol->m_multiBody->getVelocityVector();
                    // add in the normal component of the va
                    btScalar vel = 0.0;
                    for (int k = 0; k < ndof; ++k)
                    {
                        vel += local_v[k] * J_n[k];
                    }
                    va = cti.m_normal * vel;
                    
                    vel = 0.0;
                    for (int k = 0; k < ndof; ++k)
                    {
                        vel += local_v[k] * J_t1[k];
                    }
                    va += c->t1 * vel;
                    vel = 0.0;
                    for (int k = 0; k < ndof; ++k)
                    {
                        vel += local_v[k] * J_t2[k];
                    }
                    va += c->t2 * vel;
                }
            }
            else
            {
                // The object interacting with deformable node is not supported for position correction
                btAssert(false);
            }
            
            if (cti.m_colObj->hasContactResponse())
            {
                btScalar dp = cti.m_offset;
                
                // only perform position correction when penetrating
                if (dp < 0)
                {
                    c->m_node->m_v -= dp * cti.m_normal / timeStep;
                }
            }
        }
    }
}


void btDeformableMultiBodyDynamicsWorld::integrateTransforms(btScalar timeStep)
{
    BT_PROFILE("integrateTransforms");
    m_deformableBodySolver->backupVelocity();
    positionCorrection(timeStep);
    btMultiBodyDynamicsWorld::integrateTransforms(timeStep);
    for (int i = 0; i < m_softBodies.size(); ++i)
    {
        btSoftBody* psb = m_softBodies[i];
        for (int j = 0; j < psb->m_nodes.size(); ++j)
        {
            btSoftBody::Node& node = psb->m_nodes[j];
            btScalar maxDisplacement = psb->getWorldInfo()->m_maxDisplacement;
            btScalar clampDeltaV = maxDisplacement / timeStep;
            for (int c = 0; c < 3; c++)
            {
                if (node.m_v[c] > clampDeltaV)
                {
                    node.m_v[c] = clampDeltaV;
                }
                if (node.m_v[c] < -clampDeltaV)
                {
                    node.m_v[c] = -clampDeltaV;
                }
            }
            node.m_x  =  node.m_x + timeStep * node.m_v;
            node.m_q = node.m_x;
            node.m_vn = node.m_v;
        }
    }
    m_deformableBodySolver->revertVelocity();
}

void btDeformableMultiBodyDynamicsWorld::solveConstraints(btScalar timeStep)
{
    // save v_{n+1}^* velocity after explicit forces
    m_deformableBodySolver->backupVelocity();
    
    // set up constraints among multibodies and between multibodies and deformable bodies
    setupConstraints();
    solveMultiBodyRelatedConstraints();
    
    if (m_implicit)
    {
        // at this point dv = v_{n+1} - v_{n+1}^*
        // modify dv such that dv = v_{n+1} - v_n
        // modify m_backupVelocity so that it stores v_n instead of v_{n+1}^* as needed by Newton
        m_deformableBodySolver->backupVn();
    }
    
    // At this point, dv should be golden for nodes in contact
    m_deformableBodySolver->solveDeformableConstraints(timeStep);
}

void btDeformableMultiBodyDynamicsWorld::setupConstraints()
{
    // set up constraints between multibody and deformable bodies
    m_deformableBodySolver->setConstraints();
    
    // set up constraints among multibodies
    {
        sortConstraints();
        // setup the solver callback
        btMultiBodyConstraint** sortedMultiBodyConstraints = m_sortedMultiBodyConstraints.size() ? &m_sortedMultiBodyConstraints[0] : 0;
        btTypedConstraint** constraintsPtr = getNumConstraints() ? &m_sortedConstraints[0] : 0;
        m_solverMultiBodyIslandCallback->setup(&m_solverInfo, constraintsPtr, m_sortedConstraints.size(), sortedMultiBodyConstraints, m_sortedMultiBodyConstraints.size(), getDebugDrawer());
        m_constraintSolver->prepareSolve(getCollisionWorld()->getNumCollisionObjects(), getCollisionWorld()->getDispatcher()->getNumManifolds());
        
        // build islands
        m_islandManager->buildIslands(getCollisionWorld()->getDispatcher(), getCollisionWorld());
    }
}

void btDeformableMultiBodyDynamicsWorld::sortConstraints()
{
    m_sortedConstraints.resize(m_constraints.size());
    int i;
    for (i = 0; i < getNumConstraints(); i++)
    {
        m_sortedConstraints[i] = m_constraints[i];
    }
    m_sortedConstraints.quickSort(btSortConstraintOnIslandPredicate2());
    
    m_sortedMultiBodyConstraints.resize(m_multiBodyConstraints.size());
    for (i = 0; i < m_multiBodyConstraints.size(); i++)
    {
        m_sortedMultiBodyConstraints[i] = m_multiBodyConstraints[i];
    }
    m_sortedMultiBodyConstraints.quickSort(btSortMultiBodyConstraintOnIslandPredicate());
}
    
    
void btDeformableMultiBodyDynamicsWorld::solveMultiBodyRelatedConstraints()
{
    // process constraints on each island
    m_islandManager->processIslands(getCollisionWorld()->getDispatcher(), getCollisionWorld(), m_solverMultiBodyIslandCallback);
    
    // process deferred
    m_solverMultiBodyIslandCallback->processConstraints();
    m_constraintSolver->allSolved(m_solverInfo, m_debugDrawer);
    
    // write joint feedback
    {
        for (int i = 0; i < this->m_multiBodies.size(); i++)
        {
            btMultiBody* bod = m_multiBodies[i];
            
            bool isSleeping = false;
            
            if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
            {
                isSleeping = true;
            }
            for (int b = 0; b < bod->getNumLinks(); b++)
            {
                if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
                    isSleeping = true;
            }
            
            if (!isSleeping)
            {
                //useless? they get resized in stepVelocities once again (AND DIFFERENTLY)
                m_scratch_r.resize(bod->getNumLinks() + 1);  //multidof? ("Y"s use it and it is used to store qdd)
                m_scratch_v.resize(bod->getNumLinks() + 1);
                m_scratch_m.resize(bod->getNumLinks() + 1);
                
                if (bod->internalNeedsJointFeedback())
                {
                    if (!bod->isUsingRK4Integration())
                    {
                        if (bod->internalNeedsJointFeedback())
                        {
                            bool isConstraintPass = true;
                            bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(m_solverInfo.m_timeStep, m_scratch_r, m_scratch_v, m_scratch_m, isConstraintPass,
                                                                                      getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                      getSolverInfo().m_jointFeedbackInJointFrame);
                        }
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < this->m_multiBodies.size(); i++)
    {
        btMultiBody* bod = m_multiBodies[i];
        bod->processDeltaVeeMultiDof2();
    }
}

void btDeformableMultiBodyDynamicsWorld::addSoftBody(btSoftBody* body, int collisionFilterGroup, int collisionFilterMask)
{
    m_softBodies.push_back(body);
    
    // Set the soft body solver that will deal with this body
    // to be the world's solver
    body->setSoftBodySolver(m_deformableBodySolver);
    
    btCollisionWorld::addCollisionObject(body,
                                         collisionFilterGroup,
                                         collisionFilterMask);
}

void btDeformableMultiBodyDynamicsWorld::predictUnconstraintMotion(btScalar timeStep)
{
    BT_PROFILE("predictUnconstraintMotion");
    btMultiBodyDynamicsWorld::predictUnconstraintMotion(timeStep);
    m_deformableBodySolver->predictMotion(timeStep);
}

void btDeformableMultiBodyDynamicsWorld::reinitialize(btScalar timeStep)
{
    m_internalTime += timeStep;
    m_deformableBodySolver->setImplicit(m_implicit);
    m_deformableBodySolver->reinitialize(m_softBodies, timeStep);
//    if (m_implicit)
//    {
//        // todo: backup v_n velocity somewhere else
//        m_deformableBodySolver->backupVelocity();
//    }
    btDispatcherInfo& dispatchInfo = btMultiBodyDynamicsWorld::getDispatchInfo();
    dispatchInfo.m_timeStep = timeStep;
    dispatchInfo.m_stepCount = 0;
    dispatchInfo.m_debugDraw = btMultiBodyDynamicsWorld::getDebugDrawer();
    btMultiBodyDynamicsWorld::getSolverInfo().m_timeStep = timeStep;
}

void btDeformableMultiBodyDynamicsWorld::applyRigidBodyGravity(btScalar timeStep)
{
    // Gravity is applied in stepSimulation and then cleared here and then applied here and then cleared here again
    // so that 1) gravity is applied to velocity before constraint solve and 2) gravity is applied in each substep
    // when there are multiple substeps
    clearForces();
    clearMultiBodyForces();
    btMultiBodyDynamicsWorld::applyGravity();
    // integrate rigid body gravity
    for (int i = 0; i < m_nonStaticRigidBodies.size(); ++i)
    {
        btRigidBody* rb = m_nonStaticRigidBodies[i];
        rb->integrateVelocities(timeStep);
    }
    
    // integrate multibody gravity
    {
        forwardKinematics();
        clearMultiBodyConstraintForces();
        {
            for (int i = 0; i < this->m_multiBodies.size(); i++)
            {
                btMultiBody* bod = m_multiBodies[i];
                
                bool isSleeping = false;
                
                if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
                {
                    isSleeping = true;
                }
                for (int b = 0; b < bod->getNumLinks(); b++)
                {
                    if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
                        isSleeping = true;
                }
                
                if (!isSleeping)
                {
                    m_scratch_r.resize(bod->getNumLinks() + 1);
                    m_scratch_v.resize(bod->getNumLinks() + 1);
                    m_scratch_m.resize(bod->getNumLinks() + 1);
                    bool isConstraintPass = false;
                    {
                        if (!bod->isUsingRK4Integration())
                        {
                            bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(m_solverInfo.m_timeStep,
                                                                                      m_scratch_r, m_scratch_v, m_scratch_m,isConstraintPass,
                                                                                      getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                      getSolverInfo().m_jointFeedbackInJointFrame);
                        }
                        else
                        {
                            btAssert(" RK4Integration is not supported" );
                        }
                    }
                }
            }
        }
    }
    clearForces();
    clearMultiBodyForces();
}

void btDeformableMultiBodyDynamicsWorld::beforeSolverCallbacks(btScalar timeStep)
{
    if (0 != m_internalTickCallback)
    {
        (*m_internalTickCallback)(this, timeStep);
    }
    
    if (0 != m_solverCallback)
    {
        (*m_solverCallback)(m_internalTime, this);
    }
}

void btDeformableMultiBodyDynamicsWorld::afterSolverCallbacks(btScalar timeStep)
{
    if (0 != m_solverCallback)
    {
        (*m_solverCallback)(m_internalTime, this);
    }
}

void btDeformableMultiBodyDynamicsWorld::addForce(btSoftBody* psb, btDeformableLagrangianForce* force)
{
    btAlignedObjectArray<btDeformableLagrangianForce*>& forces = m_deformableBodySolver->m_objective->m_lf;
    bool added = false;
    for (int i = 0; i < forces.size(); ++i)
    {
        if (forces[i]->getForceType() == force->getForceType())
        {
            forces[i]->addSoftBody(psb);
            added = true;
            break;
        }
    }
    if (!added)
    {
        force->addSoftBody(psb);
        force->setIndices(m_deformableBodySolver->m_objective->getIndices());
        forces.push_back(force);
    }
}

void btDeformableMultiBodyDynamicsWorld::removeSoftBody(btSoftBody* body)
{
    m_softBodies.remove(body);
    btCollisionWorld::removeCollisionObject(body);
    // force a reinitialize so that node indices get updated.
    m_deformableBodySolver->reinitialize(m_softBodies, btScalar(-1));
}
