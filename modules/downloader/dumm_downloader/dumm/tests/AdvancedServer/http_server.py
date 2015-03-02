#!/usr/bin/env python
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from SocketServer import ThreadingMixIn

import threading
import sqlite3
import urlparse
import os

#import parse_folder

PORT_NUMBER = 8080
import os # os. path

CWD = os.path.abspath('./')
print CWD

def make_index( prefix, relpath ):
     abspath = os.path.abspath(relpath) # ;
     flist = os.listdir( abspath ) # ;

     rellist = []
     for fname in flist :
         relname = prefix + fname
         rellist.append(relname)

     inslist = []
     for r in rellist :
         inslist.append( '<a href="%s">%s</a><br>' % (r,r) )

     page_tpl = "<html><head></head><body>%s</body></html>"
     ret = page_tpl % ( '\n'.join(inslist) , )
     return ret

#This class will handles any incoming request from
#the browser
class HTTPRequestHandler(BaseHTTPRequestHandler):
    def do_HEAD(self):
        print self.headers
        print self.path
        o = urlparse.urlparse(self.path).path
        path_parts = o[1:].split('/')

        if len(path_parts) < 2:
            self.send_error(404,'Wrong request, expected: [files]/[filename]')
            return 

        if path_parts[0]  == 'files':
            fPath = os.path.join(CWD, 'files', path_parts[1])
            if not os.path.exists(fPath):
                self.send_error(404,'File not found')
                return
            self.send_response(200)
            self.send_header('Content-type', 'application/octet-stream')
            self.send_header('Content-length', str((os.path.getsize(fPath))))
            self.end_headers() 
            return
        else:
            self.send_error(404,'Unknown command')
            return

    def do_GET(self):
        try:
            print 'IN HTTP REQUEST HANDLER ' + self.path
            o = urlparse.urlparse(self.path).path
            path_parts = o[1:].split('/')
            print path_parts

            if len(path_parts) < 2:
                self.send_error(404,'Wrong request, expected: [cmd,files]/[cmd, filename]')
                return 
                
            if path_parts[0]  == 'cmd':
                if path_parts[1] == 'list':
                    r = make_index('/files/', './files/')
                    print r
                    self.send_response(200)
                    self.send_header("Content-type", "text/html")
                    self.end_headers()
                    self.wfile.write(r)
                    self.wfile.close()
                    return
            elif path_parts[0]  == 'files':
                fPath = os.path.join(CWD, 'files', path_parts[1])
                if not os.path.exists(fPath):
                    self.send_error(404,'File not found')
                    return
                with open(fPath, 'rb') as f:
                    self.send_response(200)
                    self.send_header('Content-type', 'application/octet-stream')
                    self.end_headers()
                    self.wfile.write(f.read())
                return
            else:
                self.send_error(404,'Unknown command')
                return

        except IOError as e :
         # debug
            print e
            self.send_error(404,'File Not Found: %s' % self.path)

    def do_PUT(self):
        print "PUT"
        print self.headers
        o = urlparse.urlparse(self.path).path
        path_parts = o[1:].split('/')
        print path_parts

        if len(path_parts) < 2:
            self.send_error(500,'Wrong request, expected: [cmd,files]/[cmd, filename]')
            return 

        if 'Content-Length' not in self.headers:
            self.send_error(411,'Content-Length required')
            return 

        if path_parts[0]  == 'files':
            fPath = os.path.join(CWD, 'files', path_parts[1])
            if os.path.exists(fPath):
                #self.send_error(405,'File exists')
                #return
                os.remove(fPath)
            length = int(self.headers['Content-Length'])
            with open(fPath, "wb") as f:
                f.write(self.rfile.read(length))

            #content = self.rfile.read(length)
            self.send_response(200)
            return
        else:
            self.send_error(415,'Unknown command')
            return

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    allow_reuse_address = True

    def shutdown(self):
        HTTPServer.shutdown(self)
        self.socket.close()

class SimpleHttpServer():
    def __init__(self, ip, port):
        self.server = ThreadedHTTPServer((ip,port), HTTPRequestHandler)

    def start(self):
        self.server_thread = threading.Thread(target=self.server.serve_forever)
        self.server_thread.daemon = True
        self.server_thread.start()

    def waitForThread(self):
        self.server_thread.join()

    def stop(self):
        print 'stop ', self.__class__.__name__
        self.server.shutdown()
        self.waitForThread()

if __name__ == "__main__":
    server = SimpleHttpServer('localhost', 8080)
    server.start()
    while(1):
        pass
    try:
        sys.stdin.read(1)
    except:
        print '^C received, shutting down server'
        pass
    print "Closing all..."

    server.stop()
