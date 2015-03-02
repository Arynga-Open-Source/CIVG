#!/usr/bin/python

import sys
sys.path.append('../python')

from optparse import OptionParser
import google.protobuf.message

try:
    from releasepackage_pb2 import UpdateFileMeta
except ImportError as imperr:
    print '''
Error: failed to import message definitions
       perhaps you need to build python bindings??
'''
    print str(imperr)
    sys.exit(1)

def main():
    """main
    """
    parser = OptionParser('usage: %prog <file>')
    (_, args) = parser.parse_args()

    if len(args) == 0:
        print 'error: missing input file'
        parser.print_help()
        raise SystemExit(1)

    infile = sys.argv[1]

    try:
        with open(infile, 'r') as inf:
            data = inf.read()
            uf = UpdateFileMeta()

            try:
                uf.ParseFromString(data)
            except google.protobuf.message.DecodeError, derr:
                print 'error decoding UF:', str(derr)
                err = True
            else:
                print uf
                err = False

        if err:
            raise SystemExit(1)

    except IOError, ioerr:
        print 'error reading file:', str(ioerr)



if __name__ == '__main__':
    main()
