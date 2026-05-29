// File WL_Data.hh
#ifndef WL_DATA_HH
#define WL_DATA_HH
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace std;

class WL_Input 
{
  friend ostream& operator<<(ostream& os, const WL_Input& in);
public:
  WL_Input(string file_name);
  unsigned Stores() const { return stores; }
  unsigned Warehouses() const { return warehouses; }
  unsigned Capacity(unsigned w) const { return capacity[w]; }
  unsigned FixedCost(unsigned w) const { return fixed_cost[w]; }
  unsigned AmountOfGoods(unsigned s) const { return amount_of_goods[s]; }
  unsigned SupplyCost(unsigned s, unsigned w) const { return supply_cost[s][w]; }
 private:
  unsigned stores, warehouses;
  vector<unsigned> capacity;
  vector<unsigned> fixed_cost;
  vector<unsigned> amount_of_goods;
  vector<vector<unsigned>> supply_cost;
};

class WL_Output 
{
  friend ostream& operator<<(ostream& os, const WL_Output& out);
  friend istream& operator>>(istream& is, WL_Output& out);
  friend bool operator==(const WL_Output& out1, const WL_Output& out2);
public:
  WL_Output(const WL_Input& i);
  WL_Output& operator=(const WL_Output& out);
  int Assignment(unsigned s) const { return assignment[s]; }
  void Assign(unsigned s, int w);
  unsigned Load(unsigned w) const { return load[w]; }
  unsigned ResidualCapacity(unsigned w) const { return in.Capacity(w) - load[w]; }
  void Reset();
  void Dump(ostream& os) const;
private:
  const WL_Input& in;
  vector<int> assignment;   // warehouse assigned to the store (-1 for unassigned)
  vector<unsigned> load; // load assigned to the warehouse (redundant)
};
#endif
