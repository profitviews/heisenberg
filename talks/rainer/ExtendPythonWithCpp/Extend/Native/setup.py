from distutils.core import setup, Extension

def main():
    setup(name="helloWorld",
          version="1.0.0",
          description="Python interface to the hello world C-function.",
          author="Rainer Grimm",
          author_email="schulung@ModernesCpp.de",
          ext_modules=[Extension("helloWorld", ["helloWorldModule.c"])])

if __name__ == "__main__":
    main()
