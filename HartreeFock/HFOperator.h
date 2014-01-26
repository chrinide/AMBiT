#ifndef HF_OPERATOR_H
#define HF_OPERATOR_H

#include "Operator.h"
#include "SpinorODE.h"
#include "Core.h"
#include "CoulombOperator.h"

/** The relativistic Hartree-Fock (Dirac-Fock) operator:
    t = (     -V                 (-d/dr + Kappa/r)/alpha )
        ( (d/dr + Kappa/r)/alpha   -2/alpha^2 - V        )
 */
class HFOperator : public OneBodyOperator, public SpinorODE
{
public:
    HFOperator(double Z, const Core* hf_core, pOPIntegrator integration_strategy, pCoulombOperator coulomb);
    HFOperator(const HFOperator& other);
    virtual ~HFOperator();

    /** Set/reset the Hartree-Fock core, from which the potential is derived. */
    virtual void SetCore(const Core* hf_core);
    virtual const Core* GetCore() const;

    /** Set exchange (nonlocal) potential and energy for ODE routines. */
    virtual void SetODEParameters(int kappa, double energy, SpinorFunction* exchange = NULL);
    
    /** Set exchange (nonlocal) potential and energy for ODE routines. */
    virtual void SetODEParameters(const SingleParticleWavefunction& approximation);

    /** Get exchange (nonlocal) potential. */
    virtual SpinorFunction GetExchange(pSingleParticleWavefunctionConst approximation = pSingleParticleWavefunctionConst()) const;
    
    /** Get df/dr = w[0] and dg/dr = w[1] given point r, (f, g).
        PRE: w should be an allocated 2 dimensional array.
     */
    virtual void GetODEFunction(unsigned int latticepoint, const SpinorFunction& fg, double* w) const;

    /** Get numerical coefficients of the ODE at the point r, (f,g).
        PRE: w_f, w_g, and w_const should be allocated 2 dimensional arrays.
     */
    virtual void GetODECoefficients(unsigned int latticepoint, const SpinorFunction& fg, double* w_f, double* w_g, double* w_const) const;

    /** Get Jacobian (dw[i]/df and dw[i]/dg), and dw[i]/dr at a point r, (f, g).
        PRE: jacobian should be an allocated 2x2 matrix,
        dwdr should be an allocated 2 dimensional array.
     */
    virtual void GetODEJacobian(unsigned int latticepoint, const SpinorFunction& fg, double** jacobian, double* dwdr) const;

    /** Get approximation to eigenfunction for first numpoints near the origin. */
    virtual void EstimateOrbitalNearOrigin(unsigned int numpoints, SpinorFunction& s) const;

    /** Get approximation to eigenfunction for last numpoints far from the origin.
        This routine can change the size of the orbital.
     */
    virtual void EstimateOrbitalNearInfinity(unsigned int numpoints, Orbital& s) const;

public:
    /** Potential = t | a > for an operator t. */
    virtual SpinorFunction ApplyTo(const SpinorFunction& a) const;

protected:
    virtual SpinorFunction CalculateExchange(const SpinorFunction& s) const;

protected:
    double Z;
    const Core* core;
    pCoulombOperator coulombSolver;

    RadialFunction directPotential;
    SpinorFunction currentExchangePotential;
    double currentEnergy;
    int currentKappa;
};

typedef boost::shared_ptr<HFOperator> pHFOperator;
typedef boost::shared_ptr<const HFOperator> pHFOperatorConst;

class HFOperatorDecorator : public HFOperator
{
public:
    HFOperatorDecorator(pHFOperator decorated_object): HFOperator(*decorated_object), wrapped(decorated_object) {}
    virtual ~HFOperatorDecorator() {}

    /** Set/reset the Hartree-Fock core, from which the potential is derived. */
    virtual void SetCore(const Core* hf_core)
    {   wrapped->SetCore(hf_core);
        core = hf_core;
        directPotential.Clear();
    }

    /** Set exchange (nonlocal) potential and energy for ODE routines. */
    virtual void SetODEParameters(int kappa, double energy, SpinorFunction* exchange = NULL)
    {   wrapped->SetODEParameters(kappa, energy, exchange);
        currentEnergy = energy;
        currentKappa = kappa;
        currentExchangePotential.Clear();
    }

    /** Set exchange (nonlocal) potential and energy for ODE routines. */
    virtual void SetODEParameters(const SingleParticleWavefunction& approximation)
    {   wrapped->SetODEParameters(approximation);
        currentEnergy = approximation.GetEnergy();
        currentKappa = approximation.Kappa();
        currentExchangePotential.Clear();
    }

    /** Get exchange (nonlocal) potential. */
    virtual SpinorFunction GetExchange(pSingleParticleWavefunctionConst approximation = pSingleParticleWavefunctionConst()) const
    {   return wrapped->GetExchange(approximation);
    }

    /** Get df/dr = w[0] and dg/dr = w[1] given point r, (f, g).
     PRE: w should be an allocated 2 dimensional array.
     */
    virtual void GetODEFunction(unsigned int latticepoint, const SpinorFunction& fg, double* w) const
    {   wrapped->GetODEFunction(latticepoint, fg, w);
    }

    /** Get numerical coefficients of the ODE at the point r, (f,g).
     PRE: w_f, w_g, and w_const should be allocated 2 dimensional arrays.
     */
    virtual void GetODECoefficients(unsigned int latticepoint, const SpinorFunction& fg, double* w_f, double* w_g, double* w_const) const
    {   wrapped->GetODECoefficients(latticepoint, fg, w_f, w_g, w_const);
    }

    /** Get Jacobian (dw[i]/df and dw[i]/dg), and dw[i]/dr at a point r, (f, g).
     PRE: jacobian should be an allocated 2x2 matrix,
     dwdr should be an allocated 2 dimensional array.
     */
    virtual void GetODEJacobian(unsigned int latticepoint, const SpinorFunction& fg, double** jacobian, double* dwdr) const
    {   wrapped->GetODEJacobian(latticepoint, fg, jacobian, dwdr);
    }

    /** Get approximation to eigenfunction for first numpoints near the origin. */
    virtual void EstimateOrbitalNearOrigin(unsigned int numpoints, SpinorFunction& s) const
    {   wrapped->EstimateOrbitalNearOrigin(numpoints, s);
    }

    /** Get approximation to eigenfunction for last numpoints far from the origin.
     This routine can change the size of the orbital.
     */
    virtual void EstimateOrbitalNearInfinity(unsigned int numpoints, Orbital& s) const
    {   wrapped->EstimateOrbitalNearInfinity(numpoints, s);
    }

public:
    /** Potential = t | a > for an operator t. */
    virtual SpinorFunction ApplyTo(const SpinorFunction& a) const
    {   return wrapped->ApplyTo(a);
    }

protected:
    pHFOperator wrapped;
};

#endif
