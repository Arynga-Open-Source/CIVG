#!/usr/bin/python

import sys
sys.path.append('../python')

from urlparse import urljoin
from hashlib import sha1
from uuid import uuid4
from optparse import OptionParser
import os.path

try:
    from releasepackage_pb2 import UpdateFileMeta
except ImportError as err:
    print '''
Error: failed to import message definitions
       perhaps you need to build python bindings??
'''
    print str(err)
    sys.exit(1)

def main():
    """main
    """
    parser = OptionParser('usage: %prog [options] <file>')
    parser.add_option('-u', '--url-prefix', dest='url_host',
                      help='URL address prefix',
                      default='http://localhost/')
    parser.add_option('-n', '--no-checksum', dest='no_checksum',
                      action='store_true', default=False,
                      help='skip generation of checksum')
    parser.add_option('-d', '--uuid', dest='uuid',
                      help='use given UUID')
    parser.add_option('-r', '--reboot', action='store_true',
                      dest='reboot', default=False,
                      help='set reboot flag')
    parser.add_option('-t', '--type', dest='type', default='raw',
                      help='package type')
    parser.add_option('-a', '--name', dest='name',
                      help='package file name in URL (default same as UUID)')
    parser.add_option('-w', '--write', dest='write_file',
                      help='write UF to file')

    (options, args) = parser.parse_args()

    if len(args) == 0:
        print 'ERROR: missing input file'
        parser.print_help()
        raise SystemExit(1)

    infile = None

    if len(args):
        infile = args[0]

    uf = UpdateFileMeta()

    if options.uuid:
        uf.uuid = options.uuid
    else:
        uf.uuid = str(uuid4())

    # setup package file URL
    url_pkg_name = uf.uuid
    if options.name:
        url_pkg_name = options.name

    # set package url
    uf.url = urljoin(options.url_host, url_pkg_name)

    # generate chekcsum as needed
    if not options.no_checksum:
        assert infile
        chsum = sha1()
        with open(infile, 'r') as inf:
            chsum.update(inf.read(4096))

        uf.checksum = chsum.hexdigest()

    # package file type
    uf.type = options.type
    # reboot flag
    uf.reboot = options.reboot
    # encrypted flags
    uf.encrypted = False

    sys.stderr.write('Update File:\n')
    sys.stderr.write(str(uf))

    if options.write_file:
        if options.write_file == '-':
            sys.stdout.write(uf.SerializeToString())
        else:
            if os.path.exists(options.write_file):
                print 'ERROR: file %s already exists, ' \
                    'will not overwrite' % (options.write_file)
            else:
                with open(options.write_file, 'wb') as outf:
                    outf.write(uf.SerializeToString())


if __name__ == '__main__':
    main()
