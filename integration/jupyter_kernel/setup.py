from distutils.core import setup

with open('README.rst') as f:
    readme = f.read()

setup(
        name='citronkernel',
        version='1.0',
        packages=['citronkernel'],
        description='Citron Kernel for IPython',
        long_description=readme,
        author='AnotherTest',
        author_email='devanothertest@gmail.com',
        url='anothertest.ir/citron',
        install_requires=[
            'jupyter_client', 'IPython', 'ipykernel'
        ],
        classifiers=[
            'Intended Audience :: Developers',
            'License :: BSD License',
            'Programming Language :: Python :: 3, Citron :: 0.0.7',
        ],
)
