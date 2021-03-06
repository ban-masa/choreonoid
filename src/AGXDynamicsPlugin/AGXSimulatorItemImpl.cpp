#include "AGXSimulatorItemImpl.h"
#include "AGXSimulatorItem.h"
#include <assert.h>
#include <cnoid/EigenUtil>
#include "gettext.h"

using namespace std;

namespace cnoid {

AGXSimulatorItemImpl::AGXSimulatorItemImpl(AGXSimulatorItemPtr self) : self(self)
{
    initialize();
    _p_gravity << 0.0, 0.0, -DEFAULT_GRAVITY_ACCELERATION;
    _p_isAutoSleep = false;
}
AGXSimulatorItemImpl::AGXSimulatorItemImpl(AGXSimulatorItemPtr self, const AGXSimulatorItemImpl& org) 
    : AGXSimulatorItemImpl(self) 
{
    initialize();    
}
AGXSimulatorItemImpl::~AGXSimulatorItemImpl(){}

void AGXSimulatorItemImpl::initialize()
{
    // Write defalut parameter here
    //agxScene = nullptr;
}

void AGXSimulatorItemImpl::doPutProperties(PutPropertyFunction & putProperty)
{
    putProperty(_("Gravity"), str(_p_gravity), [&](const string& value){ return toVector3(value, _p_gravity); });
    putProperty(_("AutoSleep"), _p_isAutoSleep, changeProperty(_p_isAutoSleep));
    //putProperty(_("All link positions"), isAllLinkPositionOutputMode,
    //        [&](bool on){ return onAllLinkPositionOutputModeChanged(on); });

//     putProperty("Friction Model Type", frictionModelType, changeProperty(frictionModelType));
//    putProperty(("Step mode"), stepMode, changeProperty(stepMode));
//    putProperty(("Step mode"), "hoge");
}

bool AGXSimulatorItemImpl::store(Archive & archive)
{
    // Write agx parameter to save
    //archive.write("friction", friction);
    return false;
}

bool AGXSimulatorItemImpl::restore(const Archive & archive)
{
    // Write agx parameter to restore
    return false;
}

SimulationBody * AGXSimulatorItemImpl::createSimulationBody(Body * orgBody)
{
    // When user click start bottom, this function will be called first.
    return new AGXBody(*orgBody);
}

#define AGX_SCENE
bool AGXSimulatorItemImpl::initializeSimulation(const std::vector<SimulationBody*>& simBodies)
{
    cout << "initializeSimulation" << endl;
    const Vector3& g = _p_gravity;
    AGXSceneDesc sd;
    sd.simdesc.timeStep = self->worldTimeStep();
    sd.simdesc.gravity = agx::Vec3(g(0), g(1), g(2));
    sd.simdesc.enableAutoSleep = _p_isAutoSleep;
    agxScene = AGXScene::create(sd);

    /* temporary code. will read material from choreonoid */
    // Create AGX material
    AGXMaterialDesc m_def;
    agxScene->createMaterial(m_def);

    // Create AGX contact material
    AGXContactMaterialDesc cm_def;
    cm_def.friction = 1.0;
    cm_def.frictionModelType = AGXFrictionModelType::DEFAULT;
    cm_def.solveType = agx::FrictionModel::SolveType::DIRECT_AND_ITERATIVE;
    agxScene->createContactMaterial(cm_def);
    /* end temporary */

    for(size_t i=0; i < simBodies.size(); ++i){
        AGXBody* body = static_cast<AGXBody*>(simBodies[i]);
        // Create rigidbody, geometry, constraints
        body->createBody();
        body->setSensor(self->worldTimeStep(), g);
        agxScene->add(body);
        for(int j = 0; j < body->numAGXLinks(); ++j){
            body->setAGXMaterial(j, agxScene->getMaterial(m_def.name));   // will replace m_def.name to choreonoid material name
        }
    }

    saveSimulationToAGXFile();

    return true;
}

bool AGXSimulatorItemImpl::stepSimulation(const std::vector<SimulationBody*>& activeSimBodies)
{
//    cout << "step" << std::endl;
    for(size_t i=0; i < activeSimBodies.size(); ++i){
        AGXBody* agxBody = static_cast<AGXBody*>(activeSimBodies[i]);
        agxBody->setControlInputToAGX();
    }

    agxScene->stepSimulation();

    for(size_t i=0; i < activeSimBodies.size(); ++i){
        AGXBody* agxBody = static_cast<AGXBody*>(activeSimBodies[i]);
        agxBody->setLinkStateToCnoid();

        if(agxBody->hasForceSensors()){
            agxBody->updateForceSensors();
        }
        if(agxBody->hasGyroOrAccelerationSensors()){
            agxBody->updateGyroAndAccelerationSensors();
        }
    }
    return true;
}

void AGXSimulatorItemImpl::stopSimulation()
{
    cout << "stopSimulation" << endl;
}

void AGXSimulatorItemImpl::pauseSimulation()
{
    cout << "pauseSimulation" << endl;
}

void AGXSimulatorItemImpl::restartSimulation()
{
    cout << "restartSimulation" << endl;
}

void AGXSimulatorItemImpl::setGravity(const Vector3& g)
{
    agxScene->setGravity(agx::Vec3(g(0), g(1), g(2)));
}

Vector3 AGXSimulatorItemImpl::getGravity() const
{
    const agx::Vec3& g = agxScene->getGravity();
    return Vector3(g.x(), g.y(), g.z());
};

    bool AGXSimulatorItemImpl::saveSimulationToAGXFile()
{
    return agxScene->saveSceneToAGXFile();
}

}