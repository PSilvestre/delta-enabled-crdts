#include<string>

using namespace std;

class cat {
  private:
    int id;
    string name;

  public:
    cat (int id_, string name_) : id(id_), name(name_) {}

    int get_id () const
    {
      return id;
    }

    string get_name() const
    {
      return name;
    }

    bool operator < (const cat& c) const 
    {
      return id < c.id;  
    }

    friend ostream& operator << (ostream& output, const cat& c)
    {
      output << "cat{id: " << c.id <<", name: " << c.name << "}";
      return output;
    }
};

template<typename T>
void dump(delta::crdt& crdt, const cat& c)
{
  delta::cat *dcat = crdt.add_cat_payload();
  dcat->set_id(c.get_id());
  dcat->set_name(c.get_name());
}
 
template<typename t>
void load (const delta::crdt& crdt, set<cat>& s)
{
  for(const delta::cat& e : crdt.cat_payload())
    s.insert(cat(e.id(), e.name()));
}

