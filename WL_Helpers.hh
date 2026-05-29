// File WL_Helpers.hh
#ifndef WL_HELPERS_HH
#define WL_HELPERS_HH

#include "WL_Data.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;

class WL_Change
{
  friend bool operator==(const WL_Change& m1, const WL_Change& m2);
  friend bool operator!=(const WL_Change& m1, const WL_Change& m2);
  friend bool operator<(const WL_Change& m1, const WL_Change& m2);
  friend ostream& operator<<(ostream& os, const WL_Change& c);
  friend istream& operator>>(istream& is, WL_Change& c);
 public:
  int store, old_w, new_w;
  WL_Change();
};

/***************************************************************************
 * State Manager 
 ***************************************************************************/

class WL_SolutionManager : public SolutionManager<WL_Input,WL_Output> 
{
public:
  WL_SolutionManager(const WL_Input &);
  void RandomState(WL_Output& out) override;   
  void GreedyState(WL_Output& out) override;   
  void DumpState(const WL_Output& out, ostream& os) const override { out.Dump(os); }   
  bool CheckConsistency(const WL_Output& st) const override;
protected:
}; 

class WL_Supply : public CostComponent<WL_Input,WL_Output> 
{
public:
  WL_Supply(const WL_Input & in, int w, bool hard) :    CostComponent<WL_Input,WL_Output>(in,w,hard,"WL_Supply") 
  {}
  int ComputeCost(const WL_Output& st) const override;
  void PrintViolations(const WL_Output& st, ostream& os = cout) const override;
};

class  WL_Opening: public CostComponent<WL_Input,WL_Output> 
{
public:
  WL_Opening(const WL_Input & in, int w, bool hard) : CostComponent<WL_Input,WL_Output>(in,w,hard,"WL_Opening") 
  {}
  int ComputeCost(const WL_Output& st) const override;
  void PrintViolations(const WL_Output& st, ostream& os = cout) const override;
};

class  WL_Capacity: public CostComponent<WL_Input,WL_Output> 
{
public:
  WL_Capacity(const WL_Input & in, int w, bool hard) : CostComponent<WL_Input,WL_Output>(in,w,hard,"WL_Capacity") 
  {}
  int ComputeCost(const WL_Output& st) const override;
  void PrintViolations(const WL_Output& st, ostream& os = cout) const override;
};

/***************************************************************************
 * WL_Change Neighborhood Explorer:
 ***************************************************************************/

class WL_ChangeDeltaSupply
  : public DeltaCostComponent<WL_Input,WL_Output,WL_Change>
{
public:
  WL_ChangeDeltaSupply(const WL_Input & in, WL_Supply& cc) 
    : DeltaCostComponent<WL_Input,WL_Output,WL_Change>(in,cc,"WL_ChangeDeltaSupply") 
  {}
  int ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const override;
};

class WL_ChangeDeltaOpening
  : public DeltaCostComponent<WL_Input,WL_Output,WL_Change>
{
public:
  WL_ChangeDeltaOpening(const WL_Input & in, WL_Opening& cc) 
    : DeltaCostComponent<WL_Input,WL_Output,WL_Change>(in,cc,"WL_ChangeDeltaOpening") 
  {}
  int ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const override;
};

class WL_ChangeDeltaCapacity
  : public DeltaCostComponent<WL_Input,WL_Output,WL_Change>
{
public:
  WL_ChangeDeltaCapacity(const WL_Input & in, WL_Capacity& cc) 
    : DeltaCostComponent<WL_Input,WL_Output,WL_Change>(in,cc,"WL_ChangeDeltaCapacity") 
  {}
  int ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const override;
};

class WL_ChangeNeighborhoodExplorer
  : public NeighborhoodExplorer<WL_Input,WL_Output,WL_Change> 
{
public:
  WL_ChangeNeighborhoodExplorer(const WL_Input & pin, SolutionManager<WL_Input,WL_Output>& psm)  
    : NeighborhoodExplorer<WL_Input,WL_Output,WL_Change>(pin, psm, "WL_ChangeNeighborhoodExplorer") {} 
  void RandomMove(const WL_Output&, WL_Change&) const override;          
  bool FeasibleMove(const WL_Output&, const WL_Change&) const override;  
  void MakeMove(WL_Output&, const WL_Change&) const override;             
  void FirstMove(const WL_Output&, WL_Change&) const override;  
  bool NextMove(const WL_Output&, WL_Change&) const override;   
protected:
  bool AnyNextMove(const WL_Output&, WL_Change&) const;   
};

#endif
