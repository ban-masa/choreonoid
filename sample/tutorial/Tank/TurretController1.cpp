#include <cnoid/SimpleController>

using namespace cnoid;

class TurretController1 : public SimpleController
{
    Link* joint;
    double q_ref;
    double q_prev;
    double dt;

public:
    virtual bool initialize(SimpleControllerIO* io)
    {
        joint = io->body()->link("TURRET_P");

        io->setLinkInput (joint, JOINT_ANGLE);
        io->setLinkOutput(joint, JOINT_TORQUE);

        q_ref = q_prev = joint->q();

        dt = io->timeStep();

        return true;
    }

    virtual bool control()
    {
        // PD gains
        static const double P = 200.0;
        static const double D = 50.0;

        double q = joint->q(); // input
        double dq = (q - q_prev) / dt;
        double dq_ref = 0.0;
        joint->u() = P * (q_ref - q) + D * (dq_ref - dq); // output
        q_prev = q;
        
        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(TurretController1)
