// File WL_Helpers.cc
#include "WL_Helpers.hh"

WL_SolutionManager::WL_SolutionManager(const WL_Input & pin) 
  : SolutionManager<WL_Input,WL_Output>(pin, "WLSolutionManager")  {} 

void WL_SolutionManager::RandomState(WL_Output& out) 
{
  unsigned s, w;
  out.Reset();
  for (s = 0; s < in.Stores(); s++)
    {
      w = Random::Uniform<int>(0, in.Warehouses() - 1);
      out.Assign(s,w);
    }
} 

void WL_SolutionManager::GreedyState(WL_Output& out) 
{
  bool found_first;
  unsigned i, s, w, best_s, best_i, best_w;
  double cost, best_cost, amortized_fixed_cost;
  vector<unsigned> unserved_stores(in.Stores());

  out.Reset();
  iota(unserved_stores.begin(), unserved_stores.end(),0);

  while(unserved_stores.size() > 0)
    {
      found_first = false;
      for (i = 0; i < unserved_stores.size(); i++)
        {
          s = unserved_stores[i];
          for (w = 0; w < in.Warehouses(); w++)
            if (out.ResidualCapacity(w) > in.AmountOfGoods(s))
              {
                if (out.Load(w) == 0) 
                  amortized_fixed_cost = in.FixedCost(w) * in.AmountOfGoods(s)/static_cast<double>(in.Capacity(w));
                else
                  amortized_fixed_cost = 0.0;
                if (!found_first)
                  {
                    found_first = true;
                    best_w = w;
                    best_s = s;
                    best_i = i;
                    best_cost = in.SupplyCost(s,w) + amortized_fixed_cost;
                  }
                else
                  {
                    cost = in.SupplyCost(s,w) + amortized_fixed_cost;
                    if (cost < best_cost) 
                      {
                        best_w = w;
                        best_s = s;
                        best_i = i;
                        best_cost = cost;
                      }
                  }
              }
        }
      if (!found_first)
        break; // no feasible assignment found
      out.Assign(best_s,best_w);
      unserved_stores.erase(unserved_stores.begin() + best_i);
    }
  if (unserved_stores.size() > 0) // if there are unassigned stores, assign all of them to warehouse 0
    for (i = 0; i < unserved_stores.size(); i++)
      out.Assign(unserved_stores[i], 0);
}

bool WL_SolutionManager::CheckConsistency(const WL_Output& st) const
{ // not necessary, as state modifications are managed by the single method Assign
  return true;
}

int WL_Supply::ComputeCost(const WL_Output& st) const
{
  unsigned s, cost = 0;
  for (s = 0; s < in.Stores(); s++)
    cost += in.AmountOfGoods(s) * in.SupplyCost(s,st.Assignment(s));
  return cost;
}

void WL_Supply::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned s;
  for (s = 0; s < in.Stores(); s++)
    os << "The cost of supplying " << in.AmountOfGoods(s) << " units from " << st.Assignment(s) 
       << " to " << s << " is " << in.AmountOfGoods(s)*in.SupplyCost(s,st.Assignment(s)) 
       << " (cost per unit " << in.SupplyCost(s,st.Assignment(s)) << ")" << endl;	
}

int WL_Opening::ComputeCost(const WL_Output& st) const
{ 
  unsigned w, cost = 0;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > 0)
      cost += in.FixedCost(w);
  return cost;
}

void WL_Opening::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned w;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > 0)
      os << "The cost of opening warehouse " << w << " is " << in.FixedCost(w) << endl;
}

int WL_Capacity::ComputeCost(const WL_Output& st) const
{ 
  unsigned w, cost = 0;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > in.Capacity(w))
      cost += st.Load(w) - in.Capacity(w);
  return cost;
}
 
void WL_Capacity::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned w;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > in.Capacity(w))
      os << "Warehouse " << w << " is overloaded by " << st.Load(w) - in.Capacity(w) << endl;
}

/*****************************************************************************
 * WL_Change Neighborhood Methods
 *****************************************************************************/
WL_Change::WL_Change()
{
   store = -1;
   old_w = -1;
   new_w = -1;
}

bool operator==(const WL_Change& mv1, const WL_Change& mv2)
{
  return mv1.store == mv2.store && mv1.new_w == mv2.new_w;
}

bool operator!=(const WL_Change& mv1, const WL_Change& mv2)
{
  return mv1.store != mv2.store || mv1.new_w != mv2.new_w;
}

bool operator<(const WL_Change& mv1, const WL_Change& mv2)
{
  return (mv1.store < mv2.store)
  || (mv1.store == mv2.store && mv1.new_w < mv2.new_w);
}

istream& operator>>(istream& is, WL_Change& mv)
{
  char ch;
  is >> mv.store >> ch >> mv.old_w >> ch >> ch >> mv.new_w;
  return is;
}

ostream& operator<<(ostream& os, const WL_Change& mv)
{
  os << mv.store << ':' << mv.old_w << "->" << mv.new_w;
  return os;
}

void WL_ChangeNeighborhoodExplorer::RandomMove(const WL_Output& st, WL_Change& mv) const
{
// Insert your code
} 

bool WL_ChangeNeighborhoodExplorer::FeasibleMove(const WL_Output& st, const WL_Change& mv) const
{
// Insert your code
  return true;
} 

void WL_ChangeNeighborhoodExplorer::MakeMove(WL_Output& st, const WL_Change& mv) const
{
// Insert your code
}  

void WL_ChangeNeighborhoodExplorer::FirstMove(const WL_Output& st, WL_Change& mv) const
{
// Insert your code
}

bool WL_ChangeNeighborhoodExplorer::NextMove(const WL_Output& st, WL_Change& mv) const
{
  do
    if (!AnyNextMove(st,mv))
      return false;
  while (!FeasibleMove(st,mv));
  return true;
}

bool WL_ChangeNeighborhoodExplorer::AnyNextMove(const WL_Output& st, WL_Change& mv) const
{
// Insert your code
  return true;
}

int WL_ChangeDeltaSupply::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  int cost = 0;
// Insert your code
  return cost;
}

int WL_ChangeDeltaOpening::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  int cost = 0;
// Insert your code
  return cost;
}

int WL_ChangeDeltaCapacity::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  int cost = 0;
// Insert your code
  return cost;
}

