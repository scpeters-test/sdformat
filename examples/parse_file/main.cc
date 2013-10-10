
#include <sdf/sdf.hh>
#include <sdf/dom/Sdf.hh>

int main(int _argc, char ** _argv)
{
  // Parse the SDF file, and get a Sdf object.
  sdf::Sdf sdf = sdf::parse(_argv[1]);

  // Output all the values.
  std::cout << sdf.DebugString() << std::endl;

  // Output the version of SDF used.
  std::cout << "SDF Version[" << sdf.version() << "]\n";

  // Output a little info about each world
  for (unsigned int i = 0; i < sdf.world_size(); ++i)
  {
    // Get the world
    sdf::World world = sdf.world(i);

    std::cout << "World Name[" << world.name() << "]\n";
    std::cout << "  Model Count[" << world.model_size() << "]\n";
    for (unsigned int j = 0; j < world.model_size(); ++j)
    {
      std::cout << "    Model[" << j << "] Name["
        << world.model(j).name() << "] "
        << "Pose[" << world.model(j).pose() << "]\n";
    }
  }

  return 0;
}
