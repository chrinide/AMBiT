#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "State.h"
#include "StatePointer.h"
#include "StateInfo.h"
#include <set>
#include <map>

typedef std::map<StateInfo, StatePointer> StateSet;

class StateIterator;
class ConstStateIterator;

class StateManager
{
public:
    friend class StateIterator;
    friend class ConstStateIterator;

public:
    StateManager(Lattice* lat, unsigned int atomic_number, int ion_charge);
    virtual ~StateManager(void);

    virtual bool Empty() const { return AllStates.empty(); }
    virtual unsigned int NumStates() const { return static_cast<unsigned int>(AllStates.size()); }

    /** Get pointer to discrete state.
        Return null if no such state exists.
     */
    virtual const State* GetState(const StateInfo& info) const;
    virtual State* GetState(const StateInfo& info);

    virtual StateIterator GetStateIterator();
    virtual ConstStateIterator GetConstStateIterator() const;

    /** Write all electron states to a file. */
    virtual void Write(FILE* fp) const = 0;

    /** Read previously stored electron states.
        Manager needs to know whether they're discrete or continuum states.
      */
    virtual void Read(FILE* fp) = 0;

    Lattice* GetLattice() const { return lattice; }

    /** Test for orthogonality of states.
        Return largest overlap.
      */
    double TestOrthogonality() const;

protected:
    /** Delete all currently stored states. */
    virtual void Clear();

    virtual void AddState(State* s);

    double Z, Charge;
    StateSet AllStates;

    Lattice* lattice;
};

#endif