#ifndef LATTICE_H
#define LATTICE_H

#include <string>
#include <vector>

/**
 * The lattice class provides a conversion between a lattice with even spacing (x),
 * and a "real" space which may not (r).
 *   x = r + beta*ln(r/rmin)
 */
class Lattice
{
public:
    Lattice(unsigned int numpoints, double r_min, double r_max);
    Lattice(const std::string& filename);
    virtual ~Lattice(void);

    unsigned int Size() const { return NumPoints; }
    double MaxRealDistance() const { return r[NumPoints-1]; }

    double R(unsigned int i);
    double dR(unsigned int i);

    /** Return all points of R, from 0 to Size()-1 */
    const double* R() const { return r; }
    const double* dR() const { return dr; }

    /** Return all points of R^k, from 0 to Size()-1.
        PRE: k > 0
      */
    const double* Rpower(unsigned int k);

    double H() const { return h; }

    /** Return the first lattice point greater than or equal to "r_point".
        If no such point exists, create it.
     */
    unsigned int real_to_lattice(double r_point);

protected:
    Lattice() {}

    /** Calculate the value that r[i] should be. */
    double lattice_to_real(unsigned int i) const;

    /** Calculate the lattice spacing at a point. */
    double calculate_dr(double r_point) const;

    /** Calculate R^power and store, for all powers up to k.
        PRE: k > 0
     */
    const double* Calculate_Rpower(unsigned int k);
    
    /** Resizes the lattice such that NumPoints > min_size. */
    void ReSize(unsigned int min_size);
protected:
    unsigned int NumPoints;
    double* r;
    double* dr;

    std::vector<double*> r_power;

    double beta, h, rmin;
};

inline double Lattice::R(unsigned int i)
{   if(i >= NumPoints)
        ReSize(i);
    return r[i];
}

inline double Lattice::dR(unsigned int i)
{   if(i >= NumPoints)
        ReSize(i);
    return dr[i];
}

inline const double* Lattice::Rpower(unsigned int k)
{
    if(k == 0)
        return NULL;
    if(k == 1)
        return r;
    if(k <= r_power.size()+1)
        return r_power[k - 2];
    else
        return Calculate_Rpower(k);
}

#endif