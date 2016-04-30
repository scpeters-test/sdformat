sdf_sources = [
  'Assert.cc',
  'Console.cc',
  'Converter.cc',
  'Element.cc',
  'Exception.cc',
  'parser.cc',
  'parser_urdf.cc',
  'Param.cc',
  'SDF.cc',
  'SDFExtension.cc',
  'Types.cc']

sdflib = shared_library('sdf',
                        sdf_sources,
                        include_directories : inc,
                        dependencies : ign_math,
                        install : true)
