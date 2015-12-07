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
void dump (proto::entry& entry, const cat& c)
{
  proto::cat *cat = entry.mutable_e_cat();
  cat->set_id(c.get_id());
  cat->set_name(c.get_name());
}
 
template<typename t>
void load (const proto::entry& entry, set<cat>& s)
{
  proto::cat e = entry.e_cat();
  s.insert(cat(e.id(), e.name()));
}

