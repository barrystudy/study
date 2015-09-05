import sys, os, inspect

cmd_folder = os.path.realpath(os.path.abspath(os.path.split(inspect.getfile( inspect.currentframe() ))[0]))
pylib = os.path.join(cmd_folder,'gyp','pylib')
sys.path.insert(0, pylib)
import gyp

if __name__=='__main__':
    print sys.argv[1:]
    sys.exit(gyp.main(sys.argv[1:]))
