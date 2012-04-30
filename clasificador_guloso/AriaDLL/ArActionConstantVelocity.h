#ifndef ARACTIONCONSTANTVELOCITY_H
#define ARACTIONCONSTANTVELOCITY_H

#include "ariaTypedefs.h"
#include "ArAction.h"

/// Action for going straight at a constant velocity
/**
   This action simply goes straight at a constant velocity.
*/
class ArActionConstantVelocity : public ArAction
{
public:
  /// Constructor
  AREXPORT ArActionConstantVelocity(const char *name = "Constant Velocity", 
			   double velocity = 400);
  /// Destructor
  AREXPORT virtual ~ArActionConstantVelocity();
  AREXPORT virtual ArActionDesired *fire(ArActionDesired currentDesired);
  AREXPORT virtual ArActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  AREXPORT virtual const ArActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  double myVelocity;
  ArActionDesired myDesired;
};

#endif // ARACTIONCONSTANTVELOCITY_H
