project('sdformat', 'cpp',
  version : '4.0.0',
  license : 'Apache2')
ign_math = dependency('ignition-math2')

sdf_dir = 'cmake/'

inc = include_directories(['include', 'build', 'sdf'])

conf_data = configuration_data()
conf_data.set('SDF_VERSION', '4.0.0')
configure_file(input : sdf_dir + 'sdf_config.h.in', 
               output : 'sdf_config.h', 
               install_dir : 'lala',
               configuration : conf_data)

subdir('src')
