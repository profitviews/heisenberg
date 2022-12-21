from distutils.core import setup, Extension


helloWorld_module = Extension('_helloWorld',
                           sources=['helloWorld_wrap.c', 'helloWorld.c'],
                           )

setup (name = 'helloWorld',
       version = '1.0.0',
       author      = "Rainer Grimm",
       description = "Python Module helloWorld",
       ext_modules = [helloWorld_module],
       )
