from skbuild import setup

setup(
  name='opencamlib',
  packages=['opencamlib'],
  package_dir={'opencamlib': 'src/pythonlib/opencamlib'},
  include_package_data=True,
  zip_safe=False,
  python_requires='>=3.6',
)
