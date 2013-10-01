
#include <sdf/sdf.hh>
#include <sdf/dom/Sdf.hh>

int main(int _argc, char ** _argv)
{
  sdf::Sdf sdf = sdf::parse(_argv[1]);
  std::cout << sdf.DebugString() << std::endl;

  /*
  sdf::SDFPtr sdf(new sdf::SDF);
  if (!sdf::init(sdf))
  {
    std::cerr << "Unable to initialize SDF\n";
    return -1;
  }

  if (!sdf::readFile(_argv[1], sdf))
  {
    std::cerr << "Unable to read sdf file[" << _argv[1] << "]\n";
    return -1;
  }

  sdf->root->PrintValues("");

  std::cout << "--------------------------------------\n";

  sdf::Sdf sdfDom;
  sdfDom.SetFromSDF(sdf->root);
  std::cout << sdfDom.DebugString() << std::endl;
  */

  return 0;
}
